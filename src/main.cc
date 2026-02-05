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
        data->release_flag[data->thread_id + 1] = true;
    }
    
    int m = 1;
    int row_index = data->thread_id * m;
    int total_rows = data->input_rows->size();
    
    if (row_index < total_rows && !*(data->should_exit) && 
        !Timings_TimeoutExpired(data->start_time, data->timeout_ms)) {
        
        const InputRow& row = (*data->input_rows)[row_index];
        char encrypted[65];
        ComputeIterativeSha256Hex(
            reinterpret_cast<const uint8_t*>(row.text.c_str()),
            row.text.length(),
            row.iterations,
            encrypted
        );
        
        (*data->output_rows)[row_index] = encrypted;
        ThreadLog("[thread %d] completed row %d", data->thread_id, row_index);
    }
    
    ThreadLog("[thread %d] returned", data->thread_id);
    return nullptr;
}

int main(int argc, char** argv) {
    CliMode mode;
    uint32_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    
    std::vector<InputRow> input_rows;
    std::string line;
    
    int row_count = 0;
    if (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        iss >> row_count;
    }
    
    for (int i = 0; i < row_count && std::getline(std::cin, line); i++) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        InputRow row;
        std::string id;
        
        if (iss >> id >> row.text >> row.iterations) {
            input_rows.push_back(row);
        }
    }
    
    std::cin.clear();
    
    int k;
    std::cout << "Enter max threads (1 - 8): ";
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        tty_in.clear();  
        tty_in >> k;
        
        if (k < 1 || k > 8) {
            Dief("Thread count must be between 1 and 8");
        }
    } else {
        Die("Cannot open /dev/tty for input");
    }
    
    int n = get_nprocs();
    std::vector<std::string> output_rows(input_rows.size());
    
    volatile bool should_exit = false;
    volatile bool* release_flags = new bool[n + 1];
    for (int i = 0; i <= n; i++) {
        release_flags[i] = false;
    }
    
    Timings_t start_time = Timings_NowMs();
    
    std::vector<ThreadData> thread_data(n);
    std::vector<pthread_t> threads(n);
    
    for (int i = 0; i < n; i++) {
        thread_data[i].thread_id = i + 1;
        thread_data[i].should_exit = &should_exit;
        thread_data[i].release_flag = release_flags;
        thread_data[i].input_rows = &input_rows;
        thread_data[i].output_rows = &output_rows;
        thread_data[i].user_k = k;
        thread_data[i].timeout_ms = timeout_ms;
        thread_data[i].start_time = start_time;
        
        pthread_create(&threads[i], nullptr, ThreadFunc, &thread_data[i]);
    }
    
    if (mode == CLI_MODE_ALL) {
        for (int i = 1; i <= k; i++) {
            release_flags[i] = true;
        }
    } 
    else if (mode == CLI_MODE_RATE) {
        for (int i = 1; i <= k; i++) {
            release_flags[i] = true;
            Timings_SleepMs(1);
        }
    }
    else if (mode == CLI_MODE_THREAD) {
        release_flags[1] = true;
    }
    
    while (!should_exit) {
        if (Timings_TimeoutExpired(start_time, timeout_ms)) {
            should_exit = true;
            break;
        }
        Timings_SleepMs(100);
    }
    
    should_exit = true;
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    ThreadLog("Thread Start Encryption");
    for (size_t i = 0; i < input_rows.size(); i++) {
        if (!output_rows[i].empty()) {
            std::string hash = output_rows[i];
            std::string truncated = hash.substr(0, 16) + "..." + hash.substr(hash.length() - 16);
            ThreadLog("%zu %s %s", i, input_rows[i].text.c_str(), truncated.c_str());
        }
    }
    
    delete[] release_flags;
    return 0;
}