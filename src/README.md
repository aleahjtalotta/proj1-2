# CSCE 311 Project 1

## src/main.cc

This file contains all of the code for the project.

The program starts with the command-line arguments to determine which mode is being used --all , --rate , or --thread and whether a timeout was provided. next it reads all input rows from standard input using file redirection.

After reading the input, the program prompts the user for the maximum number of threads k using /dev/tty. This makes sure the prompt still works even when input is redirected from a file.

The program gets the number of available processors using get_nprocs() and creates that many pthreads. All threads start in a paused state and wait in a loop using Timings_SleepMs(1) until they are released.

The release modes work like this:

--all: releases the first k threads at the same time.
--rate: releases one thread every ms.
--thread: the main thread releases thread 1, and then each thread releases the next thread.

Each thread processes rows using the formula:

    row = thread num + i * k

Threads log when they start, when they complete a row, and when they return. If a thread number is greater than k, it will exit quick. Also threads will stop running if the timeout expires of course.

The hashing is completed using the ComputeIterativeSha256Hex function in the code. After all threads finish, the program 
prints the final report labeled which is "Thread Start Encryption" and displays the thread #, the input text, and a smaller version of the hash.

# didnt use the main.h file just main.cc