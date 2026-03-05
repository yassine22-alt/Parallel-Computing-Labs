#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global domain size and local index bounds (shared with compute.c) */
int ntx, nty;
int sx, sy, ex, ey;

/* Functions from compute.c */
extern void initialization(double **pu, double **pu_new, double **pu_exact);
extern void compute(double *u, double *u_new);
extern void output_results(double *u, double *u_exact);

#define IDX(i, j) ( ((i)-(sx-1))*(ey-sy+3) + (j)-(sy-1) )
#define MAX_ITER 10000
#define TOL      1.0e-6

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    ntx = 12;
    nty = 10;

    if (rank == 0)
        printf("Poisson execution with %d MPI processes\nDomain size: ntx=%d nty=%d\n",
               size, ntx, nty);

    /* Create 2D Cartesian topology (non-periodic) */
    int dims[2] = {0, 0};
    MPI_Dims_create(size, 2, dims);

    if (rank == 0)
        printf("Topology dimensions: %d along x, %d along y\n", dims[0], dims[1]);

    if (ntx % dims[0] != 0 || nty % dims[1] != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: domain %dx%d not divisible by process grid %dx%d\n",
                    ntx, nty, dims[0], dims[1]);
        MPI_Finalize();
        return 1;
    }

    int periods[2] = {0, 0};
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    MPI_Comm_rank(cart_comm, &rank);

    int coords[2];
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    /* Compute local index range (1-based global indices) */
    int local_nx = ntx / dims[0];
    int local_ny = nty / dims[1];
    sx = coords[0] * local_nx + 1;
    ex = sx + local_nx - 1;
    sy = coords[1] * local_ny + 1;
    ey = sy + local_ny - 1;

    if (rank == 0) printf("----------------------------------------\n");
    MPI_Barrier(cart_comm);

    /* Find neighbors: direction 0 = x (north/south), direction 1 = y (east/west) */
    int north, south, east, west;
    MPI_Cart_shift(cart_comm, 0, 1, &north, &south);
    MPI_Cart_shift(cart_comm, 1, 1, &west, &east);

    /* Print process info in order */
    for (int p = 0; p < size; p++) {
        if (rank == p) {
            printf("Rank in the topology: %d  Array indices: x from %d to %d, y from %d to %d\n",
                   rank, sx, ex, sy, ey);
            printf("Process %d has neighbors: N %d  E %d  S %d  W %d\n",
                   rank, north, east, south, west);
            fflush(stdout);
        }
        MPI_Barrier(cart_comm);
    }

    /* Initialize arrays via compute.c */
    double *u, *u_new, *u_exact;
    initialization(&u, &u_new, &u_exact);

    /* Column datatype for east/west halo exchange */
    int nrows = ex - sx + 1;
    int ncols = ey - sy + 1;
    int ny_local = ey - sy + 3;  /* local array width including halo */
    MPI_Datatype column_type;
    MPI_Type_vector(nrows, 1, ny_local, MPI_DOUBLE, &column_type);
    MPI_Type_commit(&column_type);

    double start_time = MPI_Wtime();
    double global_error = 0.0;
    int iter;

    for (iter = 1; iter <= MAX_ITER; iter++) {

        /* Exchange halo rows (north/south — contiguous) */
        MPI_Sendrecv(&u[IDX(sx, sy)], ncols, MPI_DOUBLE, north, 0,
                     &u[IDX(ex + 1, sy)], ncols, MPI_DOUBLE, south, 0,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&u[IDX(ex, sy)], ncols, MPI_DOUBLE, south, 1,
                     &u[IDX(sx - 1, sy)], ncols, MPI_DOUBLE, north, 1,
                     cart_comm, MPI_STATUS_IGNORE);

        /* Exchange halo columns (east/west — non-contiguous, use column_type) */
        MPI_Sendrecv(&u[IDX(sx, sy)], 1, column_type, west, 2,
                     &u[IDX(sx, ey + 1)], 1, column_type, east, 2,
                     cart_comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&u[IDX(sx, ey)], 1, column_type, east, 3,
                     &u[IDX(sx, sy - 1)], 1, column_type, west, 3,
                     cart_comm, MPI_STATUS_IGNORE);

        /* Jacobi update (in compute.c) */
        compute(u, u_new);

        /* Compute local squared error */
        double local_error = 0.0;
        for (int i = sx; i <= ex; i++)
            for (int j = sy; j <= ey; j++) {
                double diff = u_new[IDX(i, j)] - u[IDX(i, j)];
                local_error += diff * diff;
            }

        /* Global reduction so all processes know when to stop */
        MPI_Allreduce(&local_error, &global_error, 1, MPI_DOUBLE, MPI_SUM, cart_comm);
        global_error = sqrt(global_error);

        /* Swap u and u_new */
        double *tmp = u;
        u = u_new;
        u_new = tmp;

        if (iter % 100 == 0 && rank == 0)
            printf("Iteration %d  global_error = %g\n", iter, global_error);

        if (global_error < TOL)
            break;
    }

    double elapsed = MPI_Wtime() - start_time;

    if (rank == 0)
        printf("Converged after %d iterations in %f seconds\n", iter, elapsed);

    /* Print comparison on rank 0 (first column of domain) */
    if (rank == 0)
        output_results(u, u_exact);

    free(u);
    free(u_new);
    free(u_exact);
    MPI_Type_free(&column_type);
    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}
