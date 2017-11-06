# cpu-processes-simulation

A CPU is used for running the (user) processes. The times required to do the swapping and scheduling are ignored in the simulation.

For bringing a process from disk into memory, we use one of three different algorithms: first fit, best fit and worst fit. Assume that memory is partitioned into contiguous segments, where each segment is either occupied by a process or is a hole (a contiguous area of free memory).

The free list is a list of all the holes. Holes in the free list are kept in descending order of memory address. Adjacent holes in the free list should be merged into a single hole.

Instructions to run it:
$ make
$ ./swap -f input.txt -a first -m 1024 -q 5

-f 'filename'
-a 'algorithm_name' {first, best, worst}
-m 'memory size' in Mb as integer
-q 'quantum' integer in seconds
