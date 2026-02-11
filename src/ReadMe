# CSCE 311 Project 1 

# Description
This project is a simple demonstration of multi-threading using POSIX threads in a Linux environment.
The program creates a number of threads equal to the number of available CPU cores and then allows up to
`k` of those threads to run using different release strategies. The goal is to observe how threads behave
and how race conditions appear when no synchronization is used.

# src/main.cc
This file contains the main program logic.

It does the following:
- Parses command-line arguments (`--all`, `--rate`, `--thread`, and optional `--timeout`)
- Reads all input rows from standard input
- Prompts the user for the maximum number of threads (`k`) using `/dev/tty`
- Gets the number of available processors using `get_nprocs()`
- Creates `n` paused threads
- Releases the first `k` threads according to the selected mode
- Assigns work to threads using the rule  
  `row = thread_id + i × k`
- Makes threads with an index greater than `k` exit immediately
- Stops threads when they finish their work or when the timeout expires
- Waits for all threads to finish and prints a final report

No synchronization primitives are used, and thread execution order is intentionally nondeterministic.
