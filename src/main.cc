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
  int iterations;
};

struct ThreadData {
  int tid;
  int k;
  CliMode mode;
  uint32_t timeout_ms;
  Timings_t start;
  volatile bool* should_exit;
  volatile bool* released;
  const std::vector<InputRow>* in;
  std::vector<std::string>* out;
};

void* ThreadFunc(void* arg) {
  ThreadData* d = static_cast<ThreadData*>(arg);

  if (d->tid > d->k) {
    ThreadLog("[thread %d] returned", d->tid);
    return nullptr;
  }

  while (!d->released[d->tid] && !*(d->should_exit)) {
    Timings_SleepMs(1);
    if (Timings_TimeoutExpired(d->start, d->timeout_ms)) {
      *(d->should_exit) = true;
      break;
    }
  }

  if (*(d->should_exit)) {
    ThreadLog("[thread %d] returned", d->tid);
    return nullptr;
  }

  ThreadLog("[thread %d] started", d->tid);

  if (d->mode == CLI_MODE_THREAD && d->tid < d->k) {
    d->released[d->tid + 1] = true;
  }

  int total = d->in->size();

  for (int i = 0; !*(d->should_exit); i++) {
    if (Timings_TimeoutExpired(d->start, d->timeout_ms)) {
      *(d->should_exit) = true;
      break;
    }

    int row = d->tid + i * d->k;
    int idx = row - 1;
    if (idx >= total) break;

    const InputRow& r = (*d->in)[idx];
    char hex[65];

    ComputeIterativeSha256Hex(
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

int main(int argc, char** argv) {
  CliMode mode;
  uint32_t timeout_ms;
  CliParse(argc, argv, &mode, &timeout_ms);

  int row_count;
  std::string line;

  if (!std::getline(std::cin, line)) Dief("no input");
  {
    std::istringstream iss(line);
    iss >> row_count;
  }

  std::vector<InputRow> input;
  input.reserve(row_count);

  for (int i = 0; i < row_count; i++) {
    if (!std::getline(std::cin, line)) break;
    std::istringstream iss(line);
    std::string id;
    InputRow r;
    if (!(iss >> id >> r.text >> r.iterations)) Dief("bad input");
    input.push_back(r);
  }

  std::cout << "Enter max threads (1 - 8): ";
  std::cout.flush();

  std::ifstream tty_in("/dev/tty");
  if (!tty_in) Die("tty error");

  int k;
  tty_in >> k;
  if (k < 1 || k > 8) Dief("bad k");

  int n = get_nprocs();

  std::vector<std::string> output(input.size());
  volatile bool should_exit = false;

  volatile bool* released = new bool[n + 1];
  for (int i = 0; i <= n; i++) released[i] = false;

  Timings_t start = Timings_NowMs();

  std::vector<pthread_t> threads(n);
  std::vector<ThreadData> td(n);

  for (int i = 0; i < n; i++) {
    td[i] = {i + 1, k, mode, timeout_ms, start,
             &should_exit, released, &input, &output};
    pthread_create(&threads[i], nullptr, ThreadFunc, &td[i]);
  }

  if (mode == CLI_MODE_ALL) {
    for (int i = 1; i <= k; i++) released[i] = true;
  } else if (mode == CLI_MODE_RATE) {
    for (int i = 1; i <= k; i++) {
      released[i] = true;
      Timings_SleepMs(1);
    }
  } else if (mode == CLI_MODE_THREAD) {
    released[1] = true;
  }

  for (int i = 0; i < n; i++) pthread_join(threads[i], nullptr);

  ThreadLog("Thread Start Encryption");
  for (size_t i = 0; i < input.size(); i++) {
    if (output[i].empty()) continue;
    std::string h = output[i];
    std::string t = h.substr(0, 16) + "..." + h.substr(h.size() - 16);
    ThreadLog("%zu %s %s", i + 1, input[i].text.c_str(), t.c_str());
  }

  delete[] released;
  return 0;
}
