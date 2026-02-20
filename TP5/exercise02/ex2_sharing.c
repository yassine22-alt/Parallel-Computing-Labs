#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int rank;
    int value;
    
    MPI_Init(&argc, &argv);
    
    // Get process rank and total number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Loop until negative value is entered
    while (1) {
        if (rank == 0) {
            printf("Enter an integer (negative to quit): ");
            fflush(stdout);
            scanf("%d", &value);
        }
        
        // Broadcast the value from rank 0 to all processes
        MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (value < 0) {
            if (rank == 0) {
                printf("Negative value entered. Exiting...\n");
            }
            break;
        }
        
        // Each process prints its rank and received value
        printf("Process %d got %d\n", rank, value);
        fflush(stdout);
    }
    
    MPI_Finalize();
    
    return 0;
}