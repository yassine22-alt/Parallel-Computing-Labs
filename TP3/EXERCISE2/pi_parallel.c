#include <stdio.h>
#include <omp.h>

static long num_steps = 100000;
double step;

int main() {
    int i;
    double x, pi, sum = 0.0;
    double start_time, end_time;
    
    step = 1.0 / (double) num_steps;
    
    // Start timing
    start_time = omp_get_wtime();
    
    // Parallel region with reduction
    // reduction(+:sum) means each thread has private sum, 
    // then OpenMP combines them automatically
    #pragma omp parallel private(i, x) shared(num_steps, step) reduction(+:sum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Each thread processes its portion of iterations
        // Thread 0: iterations 0, num_threads, 2*num_threads, ...
        // Thread 1: iterations 1, num_threads+1, 2*num_threads+1, ...
        // etc.
        for (i = thread_id; i < num_steps; i += num_threads) {
            x = (i + 0.5) * step;
            sum = sum + 4.0 / (1.0 + x * x);
        }
    }
    
    pi = step * sum;
    
    // End timing
    end_time = omp_get_wtime();
    
    printf("PI = %.15f\n", pi);
    printf("Execution time: %.6f seconds\n", end_time - start_time);
    printf("Number of steps: %ld\n", num_steps);
    
    return 0;
}
