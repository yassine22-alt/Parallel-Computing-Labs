#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank, size;
    int value;
    int received_value;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    
    // Get process rank and total number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        printf("Enter an integer: ");
        fflush(stdout);
        scanf("%d", &value);
        
        printf("Process %d got %d\n", rank, value);
        
        MPI_Send(&value, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } 
    else {
        MPI_Recv(&received_value, 1, MPI_INT, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        received_value += rank;
        printf("Process %d got %d\n", rank, received_value);
        
        // Send to next process (if not the last one)
        if (rank < size - 1) {
            MPI_Send(&received_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    return 0;
}