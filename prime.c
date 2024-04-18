#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

//this function checks if a given number is considered a prime
int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }
        //if divisible return false, else return true
        for (int i = 2; i <= sqrt(n); i++) {
            if (n % i == 0){
                return 0;
            }    
    }
    return 1;
    
}

int main(int argc, char *argv[]) {
    //basic variable initializations
    int rank, size, N;

    MPI_Init(&argc, &argv);  // initillizing our MPI environment
    //rank-> aka process identifier 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //handling potential errors
    if (argc != 2) {
    {
        if (rank == 0) {
            printf("Please check again, program must run with 'mpiexec -n x ./primes' N\n");
        }
        MPI_Finalize();
    }
    return -1;
}

    // start timing
    double start = MPI_Wtime();

    N = atoi(argv[1]);
    int local_start;
    int local_end;
    //we know that the range is the "floor" of dividing the total numbers by the number of processes
    int range = (N - 1) / size;
    //we can start from 2 and move upwards based on the current processes' rank
    local_start = rank * range + 2;
    //we're ensuring that the last process will cover all the possible numbers
    if (rank == size - 1) {
        local_end = N;
    } else {
        local_end = local_start + range - 1;
    }

    //static/dynamic initilizations for storing primes
    int local_range = local_end - local_start;
    int max_primes = local_range + 1;
    int* local_primes_arr = (int*)malloc(max_primes * sizeof(int));
    bool* local_primes = (bool*)malloc((local_range + 1) * sizeof(bool));
    int local_prime_count = 0;

    //initlize each as "true" as the set default
    for (int i = 0; i <= local_range; i++) {
        local_primes[i] = true;
    }

    //only check up to sqrt(N) because x must be divisible by a smaller number otherwise
    for (int x = 2; x <= sqrt(N); x++) {
        if (is_prime(x)) {
            //calculate our first multiple
            int first_multiple = (local_start/x) * x;

            //two cases to consider: if the first multiple is less than our start, we need to adjust it
            if (first_multiple < local_start) {
                first_multiple += x;
            }
            // we also need to skip in this case as well 
            if (first_multiple == x) {
                first_multiple += x;
            }

            //loop over all the multiples in range (like the directions example)
            for (int num = first_multiple; num <= local_end; num += x) {
                if (num >= local_start) {
                    //remove these numbers by marking them as not prime
                    local_primes[num - local_start] = false;
                }
            }
        }
    }

    int i = 0;
    while (i <= local_end - local_start) {
        //first check if the curr_num has been marked as a prime
        if (local_primes[i]) { 
            //store our prime into the array we initliazed earlier
            local_primes_arr[local_prime_count] = local_start + i;
            //we should also increase total count of all the primes
            local_prime_count++;
        }
        i++;
    }   

    //we're dynamicalling allocating memory to store the count of primes from each process
    int* all_prime_counts = (int*)malloc(size * sizeof(int));
    //we need to gather the counts of primes from all of our processes to our root process
    MPI_Gather(&local_prime_count, 1, MPI_INT, all_prime_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* displacements = NULL;
    int* all_primes = NULL;

    //this is for our root process, our process 0
    if (rank == 0) {
        displacements = (int*)malloc(size * sizeof(int));
        displacements[0] = 0;

        //calculate displacements based on how many primes each process has found
        for (int i = 1; i < size; i++) {
            displacements[i] = displacements[i - 1] + all_prime_counts[i - 1];
        }

        //calculate our total number of primces that we have found
        int total_primes = 0;
        for (int i = 0; i < size; i++) {
            total_primes += all_prime_counts[i];
        }

        all_primes = (int*)malloc(total_primes * sizeof(int));
    }

    //now we take all the primes from the individual array and put them into our root array 
    MPI_Gatherv(local_primes_arr, local_prime_count, MPI_INT,
                all_primes, all_prime_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    // we stop right before writing to our file
    double finish = MPI_Wtime();
    double elapsed_time = finish - start;

    printf("Proc %d > Elapsed time = %e seconds\n", rank, finish-start);
    // printf("hi");

    //instructions say to use process 0
    if (rank == 0) {
        printf("Total elapsed time: %f seconds\n", elapsed_time);
    }

    if (rank == 0) {
        char filename[256];
        sprintf(filename, "%d.txt", N);
        FILE* fp = fopen(filename, "w");
        int currentIndex = 0;  // helps us track the current index in the all_primes array
        for (int i = 0; i < size; i++) {
            currentIndex = displacements[i];  // lets set currentIndex to the start of the current set of primes
            for (int j = 0; j < all_prime_counts[i]; j++) {
                fprintf(fp, "%d ", all_primes[currentIndex]);  
                currentIndex++;  
            }
        }
    fclose(fp);  // Close the file after writing all primes

    }

    //we need to free all the memory allocated previously

    free(local_primes);
    free(local_primes_arr);
    free(all_prime_counts);
    
    if (rank == 0) {
        free(all_primes);
        free(displacements);
    }

    //clean up the mpi environment
    MPI_Finalize();
    return 0;
}

