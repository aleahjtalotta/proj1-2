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
};

void* ThreadFunc(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    return nullptr;
}

int main(int argc, char** argv) {
    CliMode mode;
    uint32_t timeout_ms;
    CliParse(argc, argv, &mode, &timeout_ms);
    
    int k;
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        int var;
        tty_in >> var;
        k = var;
        std::cout << "You entered Var:" << var << std::endl;
    }
    
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
    
    int n = get_nprocs();
    
    return 0;
}