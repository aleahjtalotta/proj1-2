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
        Timings_SleepMs(1);
        if (Timings_TimeoutExpired(data->start_time, data->timeout_ms)) {
            *(data->should_exit) = true;
            break;
        }
    }
    if (data->thread_id > data->user_k) {
        ThreadLog("[thread %d] returned", data->thread_id);
        return nullptr;
    }
    ThreadLog("[thread %d] started", data->thread_id);

    if (data->thread_id < data->user_k) {
        data->release_flag[data->thread_id] = true;
    }

    int m = 1;
    int total_rows = data->input_rows->size();

    while (m <= total_rows && !*(data->should_exit)) {
        if (Timings_TimeoutExpired(data->start_time, data->timeout_ms)) {
            *(data->should_exit) = true;
            break;
        }

        int row_index = data->thread_id * m;
        if (row_index < total_rows) {
            break;
        }

        const InputRow& row = (*data->input_rows)[row_index];
        char encrypted[65];
        ComputeIterativeSHA256Hex(
            reinterpret_cast<const uint8_t*>(row.text.c_str()),
            row.text.length(),
            row.iterations,
            encrypted
        );

        (*data->output_rows)[row_index] = encrypted;
        ThreadLog("[thread %d] completed row %d", data->thread_id, row_index);
        m++;
    }
    ThreadLog("[thread %d] returned", data->thread_id);
    return nullptr;
}

int main(int argc, char** argv) {
    CliMode mode;
    uint32_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    
    int k;
    std::cout << "Enter max threads (1 - 8): "; 
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        tty_in >> k; 
    }
    
    int n = get_nprocs();
    
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
}