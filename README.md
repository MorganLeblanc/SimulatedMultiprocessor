# SimulatedMultiprocessor

Simulates the running of processes on a system that has two available processors.

Each process has a process id, a process state (create, waiting, executing, ready, terminating), a priority, and a CPU burst time.

Each processor receives a number of processes that it must run, these numbers are provided by the user using standard input. At simulation start, each process randomly generates its needed burst time, its priority, and its process id. Each processor operates on its own thread, and must synchronise with the other processor as needed.

Each process exists in different process states, and may only run if it is in the ready states. Waiting programs randomly switch to ready to simulate real compute operation.

Processor 1 uses a FCFS process scheduling system.

Processor 2 uses a priority based scheduling system.

Processes are aged every 2 seconds.

When one processor finishes with all of it's processes, it simulates load balancing by taking over processes from the othe rprocessor.

Each step of the simulation is printed to the console. When all processes are finished, the program exits.

Created for CS 440 2018
