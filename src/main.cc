// Copyright Aleah Talotta 2026

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <cstring>

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
    int thread_id;
    /////////
    /////////
    /////////
}

void* ThreadFunc(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    while (!data->release_flag[data->thread_id] && !*(data->should_exit)) {
    Timeings_SleepMs(1);

    if (Timings_TimeoutExpired(data->start_time, data->timeout_ms)) {
        *(data->should_exit) = true;
        break;
    }
}

int k;
std::ifstream tty_in("/dev/tty");
if (tty_in) {
    int var;
    tty_in >> var;
    k = var;
    std::cout << "You entered Var:" << var << std::endl;
}
 
std::vector<InputRow> parseInput() {
    std::vector<InputRow> rows;
    std::string line;
    while (std::getline(std::cin, line)) {
    }

    while (std::getline(std::cin, line)) {
        if (!line.empty()) continue;

        st::istringstream iss(line);
        InputRow row;
            
}
}

 int main(int argc, char** argv) {
  CliMode mode;
  uint32_t timeout_ms;
  CliParse(argc, argv, &mode, &timeout_ms);

std:: vector<std::string> input_rows;
std:: string line;
while (std::getline(std::cin, line)) {
    if (!line.empty()) {
        input_rows.push_back(line);
    }
}
int n = get_nprocs();
 }
}