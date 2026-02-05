// Copyright Aleah Talotta 2026


#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
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
    volatile bool* should_exit;
    volatile bool* release_flag;
    const std::vector<InputRow>* input_rows;
    std::vector<std::string>* output_rows;
    int user_k;
    uint32_t timeout_ms;
    Timings_t start_time;

};

void* ThreadFunc(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);

    while (!data->release_flag[data->thread_id] && !*(data->should_exit)) {
        Timeing_SleepMs(1);
        if (Timings_TimeoutExpired(data->start_time, data->timeout_ms)) {
            *(data->should_exit) = true;
            break;
        }
    }
}

int main(int argc, char** argv) {
    CliMode mode;
    uint32_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    
}


int k;
std::cout << "Enter max threads (1 - 8): "; 
std::ifstream tty_in("/dev/tty");
if (tty_in) {
    tty_in >> k; 
}

        int n = get_nprocs();

        return 0;
    std::vector<InputRow> input_rows;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            InputRow row;
            std::istringstream iss(line);
            std::string id;
            if (iss >> id >> row.text >> row.iterations) {
                input_rows.push_back(row);
            }
        }
    }
    
    
    return 0;