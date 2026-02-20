#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void matrixVectorMult(double* A, double* b, double* x, int size) {
    for (int i = 0; i < size; ++i) {
        x[i] = 0.0;
        for (int j = 0; j < size; ++j) {
            x[i] += A[i * size + j] * b[j];
        }
    }
}

int main(int argc, char* argv[]) {
    int rank, size, N;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <matrix_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    N = atoi(argv[1]);
    if (N <= 0) {
        if (rank == 0) {
            printf("Matrix size must be positive.\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Variables for all processes
    double* A = NULL;
    double* b = (double*)malloc(N * sizeof(double));
    double* x_parallel = NULL;
    double* x_serial = NULL;
    
    // Calculate rows per process (handle uneven division)
    int* rows_per_proc = (int*)malloc(size * sizeof(int));
    int* displacements = (int*)malloc(size * sizeof(int));
    
    int base_rows = N / size;
    int remainder = N % size;
    
    int displacement = 0;
    for (int i = 0; i < size; i++) {
        rows_per_proc[i] = base_rows + (i < remainder ? 1 : 0);
        displacements[i] = displacement;
        displacement += rows_per_proc[i];
    }
    
    int local_rows = rows_per_proc[rank];
    double* local_A = (double*)malloc(local_rows * N * sizeof(double));
    double* local_x = (double*)malloc(local_rows * sizeof(double));
    
    // Process 0 initializes data
    if (rank == 0) {
        A = (double*)malloc(N * N * sizeof(double));
        x_parallel = (double*)malloc(N * sizeof(double));
        x_serial = (double*)malloc(N * sizeof(double));
        
        if (!A || !b || !x_parallel || !x_serial) {
            printf("Memory allocation failed.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        srand(42);
        
        // Fill A[0][:100] with random values
        int limit = (N < 100) ? N : 100;
        for (int j = 0; j < limit; ++j)
            A[j] = (double)rand() / RAND_MAX;
        
        // Copy A[0][:100] into A[1][100:200] if possible
        if (N > 1 && N > 100) {
            int copy_len = (N - 100 < 100) ? (N - 100) : 100;
            for (int j = 0; j < copy_len; ++j)
                A[1 * N + (100 + j)] = A[0 * N + j];
        }
        
        // Set diagonal
        for (int i = 0; i < N; ++i)
            A[i * N + i] = (double)rand() / RAND_MAX;
        
        // Fill vector b
        for (int i = 0; i < N; ++i)
            b[i] = (double)rand() / RAND_MAX;
        
        // Compute serial version for verification
        matrixVectorMult(A, b, x_serial, N);
    }
    
    // Broadcast vector b to all processes
    MPI_Bcast(b, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Start timing for parallel version
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Scatter rows of A to all processes (use Scatterv for uneven distribution)
    int* sendcounts = (int*)malloc(size * sizeof(int));
    int* displs = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        sendcounts[i] = rows_per_proc[i] * N;
        displs[i] = displacements[i] * N;
    }
    
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, local_rows * N, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);
    
    // Each process computes its portion of the result
    for (int i = 0; i < local_rows; i++) {
        local_x[i] = 0.0;
        for (int j = 0; j < N; j++) {
            local_x[i] += local_A[i * N + j] * b[j];
        }
    }
    
    // Gather results back to process 0
    MPI_Gatherv(local_x, local_rows, MPI_DOUBLE,
                x_parallel, rows_per_proc, displacements, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    
    // Process 0 verifies and reports results
    if (rank == 0) {
        double parallel_time = end_time - start_time;
        
        // Compute serial time
        double serial_start = MPI_Wtime();
        matrixVectorMult(A, b, x_serial, N);
        double serial_time = MPI_Wtime() - serial_start;
        
        // Compare results
        double max_error = 0.0;
        for (int i = 0; i < N; i++) {
            double diff = fabs(x_parallel[i] - x_serial[i]);
            if (diff > max_error)
                max_error = diff;
        }
        
        printf("Matrix size: %d x %d\n", N, N);
        printf("Number of processes: %d\n", size);
        printf("Serial time: %f seconds\n", serial_time);
        printf("Parallel time: %f seconds\n", parallel_time);
        printf("Speedup: %f\n", serial_time / parallel_time);
        printf("Efficiency: %f\n", (serial_time / parallel_time) / size);
        printf("Maximum difference: %e\n", max_error);
        
        free(A);
        free(x_parallel);
        free(x_serial);
    }
    
    // Cleanup
    free(b);
    free(local_A);
    free(local_x);
    free(rows_per_proc);
    free(displacements);
    free(sendcounts);
    free(displs);
    
    MPI_Finalize();
    return 0;
}
