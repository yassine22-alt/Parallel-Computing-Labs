#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double compute_pi_partial(long long start, long long end, long long N) {
    double sum = 0.0;
    double x;
    
    for (long long i = start; i < end; i++) {
        x = (i + 0.5) / N;
        sum += 1.0 / (1.0 + x * x);
    }
    
    return sum;
}

int main(int argc, char* argv[]) {
    int rank, size;
    long long N;
    double start_time, end_time;
    double local_sum, global_sum, pi;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <N>\n", argv[0]);
            printf("Example: %s 1000000000\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    N = atoll(argv[1]);
    if (N <= 0) {
        if (rank == 0) {
            printf("N must be positive.\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Calculate work distribution (handle N not divisible by size)
    long long base_iterations = N / size;
    long long remainder = N % size;
    
    // First 'remainder' processes get one extra iteration
    long long local_start, local_end, local_iterations;
    if (rank < remainder) {
        local_iterations = base_iterations + 1;
        local_start = rank * local_iterations;
    } else {
        local_iterations = base_iterations;
        local_start = rank * base_iterations + remainder;
    }
    local_end = local_start + local_iterations;
    
    // Barrier to synchronize before timing
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Each process computes its portion of the sum
    local_sum = compute_pi_partial(local_start, local_end, N);
    
    // Reduce all partial sums to process 0
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    // Process 0 computes final result
    if (rank == 0) {
        pi = 4.0 * global_sum / N;
        double parallel_time = end_time - start_time;
        
        double serial_start = MPI_Wtime();
        double serial_sum = compute_pi_partial(0, N, N);
        double pi_serial = 4.0 * serial_sum / N;
        double serial_time = MPI_Wtime() - serial_start;
        
        printf("N = %lld\n", N);
        printf("Number of processes: %d\n", size);
        printf("Computed Pi (parallel): %.15f\n", pi);
        printf("Computed Pi (serial):   %.15f\n", pi_serial);
        printf("Actual Pi:              %.15f\n", M_PI);
        printf("Error (parallel): %.15e\n", fabs(pi - M_PI));
        printf("Error (serial):   %.15e\n", fabs(pi_serial - M_PI));
        printf("Serial time:   %f seconds\n", serial_time);
        printf("Parallel time: %f seconds\n", parallel_time);
        printf("Speedup:       %f\n", serial_time / parallel_time);
        printf("Efficiency:    %f (%.2f%%)\n", 
               (serial_time / parallel_time) / size,
               ((serial_time / parallel_time) / size) * 100);
    }
    
    MPI_Finalize();
    return 0;
}
