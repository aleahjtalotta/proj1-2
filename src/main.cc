// Copyright Aleah Talotta 2026

#include <iostream>
//#include "cli_parser.h" 
#include "error.h"
#include "thread_log.h"
#include "timings.h"
#include "sha256.h"

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
