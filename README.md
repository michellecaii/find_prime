
# MPI Prime Number Calculator

## Overview
This MPI program leverages parallel computing to efficiently calculate all prime numbers up to a given number `N`. By distributing the workload across multiple processors, the program accelerates the prime-checking process through concurrent computations.

## Prerequisites
- **MPI Library**: The program requires an MPI (Message Passing Interface) library such as MPICH or OpenMPI to be installed on the system.
- **C Compiler**: A C compiler like GCC capable of compiling MPI programs is necessary.

## Compilation
Compile the program using the following command, substituting `mpicc` with your specific MPI compiler if different:

```bash
mpicc -o primes primes.c -lm
```

This command will compile the source code into an executable named `primes`. The `-lm` flag links against the math library for the `sqrt` function.

## Usage
Execute the program using the `mpiexec` or `mpirun` command followed by the `-n` option to specify the number of processes. For example:

```bash
mpiexec -n 4 ./primes 100
```

This command runs the program with 4 processes to find all prime numbers up to 100.

### Input Arguments
- `N`: The upper limit for prime number calculations. It should be passed as a command-line argument.

## Output
The program outputs the elapsed time for the calculations to the standard output and writes the list of prime numbers to a text file named `<N>.txt`, where `N` is the input limit. For instance, if run with `N` as 100, the output file will be `100.txt`.
