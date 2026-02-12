// Copyright Aleah Talotta 2026
#include <pthread.h>
#include <sys/sysinfo.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

struct InputRow {
  std::string text;
  int iterations; };

struct ThreadData {
  int tid;
  int k;
  CliMode mode;
  uint32_t timeout_ms;
  Timings_t start;
  volatile bool* released;
  const std::vector<InputRow>* in;
  std::vector<std::string>* out; };

static void* ThreadFunc(void* arg) {  // thread entry funct.
  ThreadData* d = static_cast<ThreadData*>(arg); // gets thread data

  if (d->tid > d->k) {  // if thread bigger than k, exits immediately 
    ThreadLog("[thread %d] returned", d->tid);
    return nullptr;
  }
  while (!d->released[d->tid]) {
    Timings_SleepMs(1);  // this is so we dont busy wait 
    if (Timings_TimeoutExpired(d->start, d->timeout_ms)) {
      ThreadLog("[thread %d] returned", d->tid);
      return nullptr;
    }
  }

  ThreadLog("[thread %d] started", d->tid); // log that I started work

  if (d->mode == CLI_MODE_THREAD && d->tid < d->k) {
  d->released[d->tid + 1] = true; // realeases next thread
  }

  const int total = static_cast<int>(d->in->size()); // number of input rows

  for (int i = 0;; i++) {
    if (Timings_TimeoutExpired(d->start, d->timeout_ms)) { // stops if timeout hits 
      ThreadLog("[thread %d] returned", d->tid);
      return nullptr;
    }

    const int row = d->tid + i * d->k;
    const int idx = row - 1;
    if (idx >= total) break;

    const InputRow& r = (*d->in)[idx];

    char hex[65];  
    ComputeIterativeSha256Hex(  // do the hashing work for this row
        reinterpret_cast<const uint8_t*>(r.text.c_str()),
        r.text.size(),
        r.iterations,
        hex);
    (*d->out)[idx] = hex;
    ThreadLog("[thread %d] completed row %d", d->tid, row);
  }

  ThreadLog("[thread %d] returned", d->tid);
  return nullptr;
}
int main(int argc, char** argv) {  // program starts 
  CliMode mode;
  uint32_t timeout_ms;
  CliParse(argc, argv, &mode, &timeout_ms);

  std::string line;
  if (!std::getline(std::cin, line)) Dief("no input");

  int row_count = 0;
  {
    std::istringstream iss(line);
    if (!(iss >> row_count)) Dief("bad row count");
  }

  std::vector<InputRow> input;
  input.reserve(row_count);  // reserve space for rows 

  for (int i = 0; i < row_count; i++) {
    if (!std::getline(std::cin, line)) Dief("missing rows");
    std::istringstream iss(line);
    std::string id;
    InputRow r;
    if (!(iss >> id >> r.text >> r.iterations)) Dief("bad row");
    input.push_back(r);
  }

  const int n = get_nprocs(); // how many CPU contexts (how many threads to create)


  std::ofstream tty_out("/dev/tty");
  std::ifstream tty_in("/dev/tty");
  if (!tty_in || !tty_out) Die("Cannot open /dev/tty");

  int k = 0;
  tty_out << "Enter max threads (1 - 8): " << std::flush;
  tty_in >> k;
  if (k < 1 || k > 8) Dief("Thread count must be between 1 and 8");

  std::vector<std::string> output(input.size());

  volatile bool* released = new bool[n + 2];   // releases flags for each thread id
  for (int i = 0; i < n + 2; i++) released[i] = false;

  const Timings_t start = Timings_NowMs();

  std::vector<pthread_t> threads(n);
  std::vector<ThreadData> td(n);

  for (int i = 0; i < n; i++) {
    td[i].tid = i + 1;
    td[i].k = k;
    td[i].mode = mode;
    td[i].timeout_ms = timeout_ms;
    td[i].start = start;
    td[i].released = released;
    td[i].in = &input;
    td[i].out = &output;

    pthread_create(&threads[i], nullptr, ThreadFunc, &td[i]);
  }

  if (mode == CLI_MODE_ALL) {   // releases all at once
    for (int i = 1; i <= k; i++) released[i] = true;
  } else if (mode == CLI_MODE_RATE) {   // realease 1 per ms
    for (int i = 1; i <= k; i++) {
      released[i] = true;
      Timings_SleepMs(1);
    }
  } else if (mode == CLI_MODE_THREAD) {
    released[1] = true;
  }

  for (int i = 0; i < n; i++) {   // wait for all threads to finish
    pthread_join(threads[i], nullptr);
  }
  
  ThreadLog("Thread Start Encryption");
  for (size_t i = 0; i < input.size(); i++) {
    if (output[i].empty()) continue;
    size_t row = i + 1;
    int tid = static_cast<int>(((row - 1) % static_cast<size_t>(k)) + 1);

    const std::string& h = output[i];
    std::string t = h.substr(0, 16) + "..." + h.substr(h.size() - 16);  // shorten hash for printing
    ThreadLog("%d %s %s", tid, input[i].text.c_str(), t.c_str());
  }
  delete[] released;
  return 0;
}
