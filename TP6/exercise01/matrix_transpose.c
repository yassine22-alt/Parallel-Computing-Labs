#include <stdio.h>
#include <mpi.h>

#define ROWS 4
#define COLS 5

int main(int argc, char* argv[]) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (size != 2) {
        if (rank == 0) {
            printf("This program requires exactly 2 processes.\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Process 0: Source matrix
    if (rank == 0) {
        int a[ROWS][COLS];
        
        for (int i=0; i< ROWS; i++) {
            for (int j=0; j< COLS; j++){
                a[i][j] = i*COLS + j + 1;
            }
        }
        
        
        // Display the original matrix
        printf("Process 0 - Matrix a:\n");
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                printf("%2d ", a[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        
        MPI_Send(&a[0][0], ROWS * COLS, MPI_INT, 1, 0, MPI_COMM_WORLD);
        
    }
    
    // Process 1: Receive transposed matrix
    if (rank == 1) {
        int at[COLS][ROWS]; 
        
        // Place COLS consecutive message elements at positions with stride ROWS
        MPI_Datatype column_type;
    
        MPI_Type_vector(COLS, 1, ROWS, MPI_INT, &column_type);
        
        MPI_Type_commit(&column_type);
        
        
        // Apply column_type ROWS times, each starting sizeof(int) bytes later
        MPI_Datatype transpose_type;

        MPI_Type_create_hvector(ROWS, 1, sizeof(int), column_type, &transpose_type);

        MPI_Type_commit(&transpose_type);

        MPI_Recv(&at[0][0], 1, transpose_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Display the transposed matrix
        printf("Process 1 - Matrix transposee at:\n");
        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < ROWS; j++) {
                printf("%2d ", at[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        
        MPI_Type_free(&transpose_type);
        MPI_Type_free(&column_type);
    }
    
    MPI_Finalize();
    return 0;
}
