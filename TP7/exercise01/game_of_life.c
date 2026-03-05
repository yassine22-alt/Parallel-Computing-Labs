#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define GLOBAL_NX 16
#define GLOBAL_NY 16
#define NUM_GENS  10

#define IDX(i, j, ny_with_halo) ((i) * (ny_with_halo) + (j))

/* Count 8 neighbors of cell (i,j) — halo guarantees valid access */
static int count_neighbors(const int *grid, int i, int j, int ny_halo) {
    return grid[IDX(i-1, j-1, ny_halo)] + grid[IDX(i-1, j, ny_halo)] + grid[IDX(i-1, j+1, ny_halo)]
         + grid[IDX(i,   j-1, ny_halo)]                                + grid[IDX(i,   j+1, ny_halo)]
         + grid[IDX(i+1, j-1, ny_halo)] + grid[IDX(i+1, j, ny_halo)] + grid[IDX(i+1, j+1, ny_halo)];
}

/* Gather all subgrids to rank 0 and print the full global grid */
static void gather_and_print(const int *local_grid, int local_nx, int local_ny,
                             int ny_halo, int rank, int size,
                             MPI_Comm cart_comm, int gen) {
    int *local_buf = malloc(local_nx * local_ny * sizeof(int));
    for (int i = 0; i < local_nx; i++)
        for (int j = 0; j < local_ny; j++)
            local_buf[i * local_ny + j] = local_grid[IDX(i + 1, j + 1, ny_halo)];

    int sub_size = local_nx * local_ny;
    int *all_bufs = NULL;
    int *recv_buf = NULL;
    if (rank == 0) {
        recv_buf = malloc(GLOBAL_NX * GLOBAL_NY * sizeof(int));
        all_bufs = malloc(size * sub_size * sizeof(int));
    }

    MPI_Gather(local_buf, sub_size, MPI_INT,
               all_bufs, sub_size, MPI_INT, 0, cart_comm);

    if (rank == 0) {
        /* Reconstruct global grid from gathered subgrids */
        for (int p = 0; p < size; p++) {
            int coords[2];
            MPI_Cart_coords(cart_comm, p, 2, coords);
            int start_row = coords[0] * local_nx;
            int start_col = coords[1] * local_ny;
            for (int i = 0; i < local_nx; i++)
                for (int j = 0; j < local_ny; j++)
                    recv_buf[(start_row + i) * GLOBAL_NY + (start_col + j)] =
                        all_bufs[p * sub_size + i * local_ny + j];
        }

        printf("=== Global Grid - Generation %d ===\n", gen);
        for (int i = 0; i < GLOBAL_NX; i++) {
            for (int j = 0; j < GLOBAL_NY; j++)
                printf("%d ", recv_buf[i * GLOBAL_NY + j]);
            printf("\n");
        }
        printf("\n");
        fflush(stdout);
        free(recv_buf);
        free(all_bufs);
    }

    free(local_buf);
}

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Compute 2D process grid */
    int dims[2] = {0, 0};
    MPI_Dims_create(size, 2, dims);

    if (rank == 0)
        printf("Process grid: %d x %d  |  Global grid: %d x %d  |  Generations: %d\n\n",
               dims[0], dims[1], GLOBAL_NX, GLOBAL_NY, NUM_GENS);

    if (GLOBAL_NX % dims[0] != 0 || GLOBAL_NY % dims[1] != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: grid %dx%d not evenly divisible by process grid %dx%d\n",
                    GLOBAL_NX, GLOBAL_NY, dims[0], dims[1]);
        MPI_Finalize();
        return 1;
    }

    /* Create 2D Cartesian communicator with periodic boundaries */
    int periods[2] = {1, 1};
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    MPI_Comm_rank(cart_comm, &rank);

    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    /* Allocate local subgrid with 1-cell halo border */
    int local_nx = GLOBAL_NX / dims[0];
    int local_ny = GLOBAL_NY / dims[1];
    int ny_halo  = local_ny + 2;
    int grid_size = (local_nx + 2) * ny_halo;

    int *grid     = calloc(grid_size, sizeof(int));
    int *grid_new = calloc(grid_size, sizeof(int));

    /* Seed a glider pattern at global position (1,1) */
    int glider[][2] = {{1,2}, {2,3}, {3,1}, {3,2}, {3,3}};
    int row_start = coords[0] * local_nx;
    int col_start = coords[1] * local_ny;
    for (int g = 0; g < 5; g++) {
        int gr = glider[g][0], gc = glider[g][1];
        if (gr >= row_start && gr < row_start + local_nx &&
            gc >= col_start && gc < col_start + local_ny)
            grid[IDX(gr - row_start + 1, gc - col_start + 1, ny_halo)] = 1;
    }

    /* Find 4 direct neighbors: direction 0 = row, direction 1 = col */
    int north, south, east, west;
    MPI_Cart_shift(cart_comm, 0, 1, &north, &south);
    MPI_Cart_shift(cart_comm, 1, 1, &west, &east);

    /* Column datatype for non-contiguous column exchange */
    MPI_Datatype column_type;
    MPI_Type_vector(local_nx, 1, ny_halo, MPI_INT, &column_type);
    MPI_Type_commit(&column_type);

    for (int gen = 0; gen < NUM_GENS; gen++) {

        /* Exchange halo rows (north/south) */
        MPI_Sendrecv(&grid[IDX(1, 1, ny_halo)], local_ny, MPI_INT, north, 0,
                     &grid[IDX(local_nx + 1, 1, ny_halo)], local_ny, MPI_INT, south, 0,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&grid[IDX(local_nx, 1, ny_halo)], local_ny, MPI_INT, south, 1,
                     &grid[IDX(0, 1, ny_halo)], local_ny, MPI_INT, north, 1,
                     cart_comm, MPI_STATUS_IGNORE);

        /* Exchange halo columns (east/west) */
        MPI_Sendrecv(&grid[IDX(1, 1, ny_halo)], 1, column_type, west, 2,
                     &grid[IDX(1, local_ny + 1, ny_halo)], 1, column_type, east, 2,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&grid[IDX(1, local_ny, ny_halo)], 1, column_type, east, 3,
                     &grid[IDX(1, 0, ny_halo)], 1, column_type, west, 3,
                     cart_comm, MPI_STATUS_IGNORE);

        /* Exchange 4 diagonal corner cells (needed for 8-neighbor stencil) */
        int nw, ne, sw, se;
        {
            int c0[2] = {(coords[0]-1+dims[0])%dims[0], (coords[1]-1+dims[1])%dims[1]};
            int c1[2] = {(coords[0]-1+dims[0])%dims[0], (coords[1]+1)%dims[1]};
            int c2[2] = {(coords[0]+1)%dims[0],         (coords[1]-1+dims[1])%dims[1]};
            int c3[2] = {(coords[0]+1)%dims[0],         (coords[1]+1)%dims[1]};
            MPI_Cart_rank(cart_comm, c0, &nw);
            MPI_Cart_rank(cart_comm, c1, &ne);
            MPI_Cart_rank(cart_comm, c2, &sw);
            MPI_Cart_rank(cart_comm, c3, &se);
        }
        MPI_Sendrecv(&grid[IDX(1, 1, ny_halo)], 1, MPI_INT, nw, 4,
                     &grid[IDX(local_nx+1, local_ny+1, ny_halo)], 1, MPI_INT, se, 4,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&grid[IDX(1, local_ny, ny_halo)], 1, MPI_INT, ne, 5,
                     &grid[IDX(local_nx+1, 0, ny_halo)], 1, MPI_INT, sw, 5,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&grid[IDX(local_nx, 1, ny_halo)], 1, MPI_INT, sw, 6,
                     &grid[IDX(0, local_ny+1, ny_halo)], 1, MPI_INT, ne, 6,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&grid[IDX(local_nx, local_ny, ny_halo)], 1, MPI_INT, se, 7,
                     &grid[IDX(0, 0, ny_halo)], 1, MPI_INT, nw, 7,
                     cart_comm, MPI_STATUS_IGNORE);

        /* Apply Conway's rules */
        for (int i = 1; i <= local_nx; i++) {
            for (int j = 1; j <= local_ny; j++) {
                int n = count_neighbors(grid, i, j, ny_halo);
                int alive = grid[IDX(i, j, ny_halo)];
                grid_new[IDX(i, j, ny_halo)] = (alive) ? (n == 2 || n == 3) : (n == 3);
            }
        }

        int *tmp = grid;
        grid = grid_new;
        grid_new = tmp;
    }

    gather_and_print(grid, local_nx, local_ny, ny_halo, rank, size, cart_comm, NUM_GENS);

    MPI_Type_free(&column_type);
    free(grid);
    free(grid_new);
    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}
