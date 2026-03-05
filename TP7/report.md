# LAB 7: MPI Communicators: Lab Report

## Introduction

In this lab, I worked with MPI Cartesian topologies to solve two classic parallel computing problems: Conway's Game of Life and the Poisson equation via Jacobi iteration. Both exercises revolve around the same core pattern, which was distributing a 2D grid across processes, exchanging halo (ghost) layers between neighbors, and updating local cells using a stencil. The key MPI features I used were `MPI_Cart_create` for topology setup, `MPI_Cart_shift` for neighbor discovery, `MPI_Sendrecv` for deadlock-free halo exchange, and `MPI_Type_vector` for non-contiguous column communication.

---

## Exercise 1: Game of Life

### Approach

I parallelized Conway's Game of Life on a 16×16 grid. I used `MPI_Dims_create` to split the available processes into a 2D process grid, then `MPI_Cart_create` with periodic boundaries (`periods = {1, 1}`) so the grid wraps around on all edges.

Each process owns a subgrid of size `local_nx × local_ny`, allocated as `(local_nx+2) × (local_ny+2)` to include a 1-cell halo border. I identified neighbors with `MPI_Cart_shift` (direction 0 for north/south, direction 1 for east/west).

The main challenge was the halo exchange. Rows are contiguous in memory, so I sent them directly with `MPI_Sendrecv`. Columns are strided, so I created a derived datatype with `MPI_Type_vector(local_nx, 1, ny_halo, MPI_INT, &column_type)` to handle them without manual packing. Since the Game of Life uses an 8-neighbor stencil, I also needed the 4 diagonal corner cells. I computed the diagonal neighbor ranks manually with `MPI_Cart_rank` on shifted coordinates, then exchanged those single cells via additional `MPI_Sendrecv` calls.

After exchanging halos, I applied Conway's rules on the interior cells only (rows 1 to `local_nx`, cols 1 to `local_ny`), then swapped the current and next-generation grids. After all generations, I gathered subgrids to rank 0 with `MPI_Gather` and reconstructed the global grid for display.

### Results

Running with 4 processes on a 2×2 process grid for 10 generations, I seeded a glider at position (1,1). The output showed the glider correctly moved diagonally down-right, confirming that both the periodic boundaries and the halo exchange (including corners) work properly.

```
Process grid: 2 x 2  |  Global grid: 16 x 16  |  Generations: 10

=== Global Grid - Generation 10 ===
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
...
0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0
0 0 0 1 0 1 0 0 0 0 0 0 0 0 0 0
0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0
...
```

---

## Exercise 2: Poisson Equation (Jacobi Solver)

### Approach

I solved the Poisson equation Δu = f on [0,1]² with homogeneous Dirichlet boundary conditions using Jacobi iteration. The provided `compute.c` file already contained the initialization (grid spacing, RHS, exact solution) and the Jacobi update formula, I wrote the MPI driver around it.

The topology setup was similar to Exercise 1, except I used non-periodic boundaries (`periods = {0, 0}`) since the Poisson problem has fixed boundary conditions at the domain edges. When `MPI_Cart_shift` returns `MPI_PROC_NULL` for a boundary process, `MPI_Sendrecv` automatically skips the communication, keeping the halo at zero, which is exactly the Dirichlet condition.

The `compute.c` file uses global indices `(sx, sy)` to `(ex, ey)` to identify each process's portion of the domain, with an `IDX` macro that maps global coordinates to local array offsets. I set these globals based on each process's Cartesian coordinates: `sx = coords[0] * local_nx + 1`, `ex = sx + local_nx - 1`, and similarly for y.

At each iteration, I performed 4 `MPI_Sendrecv` calls for the halo exchange (same row/column pattern as Exercise 1, but with `MPI_DOUBLE` and no diagonal corners since the Poisson stencil is 4-point). Then I called `compute(u, u_new)` for the Jacobi update, computed a local squared residual, and used `MPI_Allreduce` with `MPI_SUM` to get the global error norm. Using `MPI_Allreduce` instead of `MPI_Reduce` means all processes know the global error and can independently decide whether to stop, without an extra broadcast.

### Results

With `ntx=12, nty=10` and 4 processes:

```
Topology dimensions: 2 along x, 2 along y
Rank 0: x from 1 to 6, y from 1 to 5   | Neighbors: N -1  E 1  S 2  W -1
Rank 1: x from 1 to 6, y from 6 to 10  | Neighbors: N -1  E -1 S 3  W 0
Rank 2: x from 7 to 12, y from 1 to 5  | Neighbors: N 0   E 3  S -1 W -1
Rank 3: x from 7 to 12, y from 6 to 10 | Neighbors: N 1   E -1 S -1 W 2

Iteration 100  global_error = 0.000446007
Iteration 200  global_error = 1.42714e-05
Converged after 278 iterations in 0.003013 seconds

Exact solution u_exact - Computed solution u
 5.86826e-03 -  5.86794e-03
 1.05629e-02 -  1.05623e-02
 1.40838e-02 -  1.40830e-02
 1.64311e-02 -  1.64301e-02
 1.76048e-02 -  1.76037e-02
```

I also ran with 1 process and got identical convergence (278 iterations, same error values), confirming that the parallel decomposition does not alter the numerical result.

---

## Key Takeaways

- **Cartesian topologies** (`MPI_Cart_create`) simplify neighbor identification in structured grid problems (no manual rank arithmetic needed).
- **Halo exchange** is the core communication pattern for stencil computations. Rows are straightforward; columns require `MPI_Type_vector` to avoid manual packing.
- The **8-point stencil** (Game of Life) requires diagonal corner exchanges on top of the 4 standard row/column exchanges. The **4-point stencil** (Poisson) does not.
- **Periodic vs non-periodic** boundaries are handled entirely by the topology configuration, the communication code stays the same thanks to `MPI_PROC_NULL`.
- `MPI_Allreduce` is the natural choice for convergence checks because it avoids an extra broadcast step — every process gets the result simultaneously.
