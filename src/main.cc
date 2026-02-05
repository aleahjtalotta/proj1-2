// Copyright Aleah Talotta 2026

#include <iostream>

#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

#include <vector>
#include <fstream>

int main() {
int k;
std::ifstream tty_in("/dev/tty");
if (tty_in) {
    int var;
    tty_in >> var;
    k = var;
    std::cout << "You entered Var:" << var << std::endl;
}
 return 0;
}
