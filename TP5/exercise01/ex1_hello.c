#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    
    // Get process rank and total number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Start timing
    start_time = MPI_Wtime();
    
    printf("Hello World\n");
    
    printf("Hello from process %d of %d\n", rank, size);
    

    if (rank == 0) {
        printf("This message is from rank 0 only\n");
        printf("Total number of processes: %d\n", size);
    }
    
    // End timing
    end_time = MPI_Wtime();
    
    printf("Process %d execution time: %f seconds\n", rank, end_time - start_time);
    
    // MPI_Finalize();
    
    return 0;
}
