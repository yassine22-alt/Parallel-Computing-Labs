#include <stdio.h>
#include <omp.h>

int main() {
    int num_threads;
    
    // Parallel region - each thread executes this block
    #pragma omp parallel
    {
        // Get the rank (ID) of the current thread
        int thread_id = omp_get_thread_num();
        
        // Get the total number of threads
        num_threads = omp_get_num_threads();
        
        // Each thread prints its rank
        printf("Hello from the rank %d thread\n", thread_id);
        
        // Only the master thread (rank 0) prints the summary
        #pragma omp master
        {
            printf("Parallel execution of hello_world with %d thread", num_threads);
            if (num_threads > 1) {
                printf("s");
            }
            printf("\n");
        }
    }
    
    return 0;
}
