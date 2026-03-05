# TP6: MPI Derived Types - Lab Report

## Overview

In this lab, I worked with MPI derived datatypes to handle complex data structures and non-contiguous memory patterns. The two exercises demonstrated different aspects of this powerful feature: matrix transposition through custom memory layouts and distributed machine learning with structured data communication.

## Exercise 1: Matrix Transposition Using Derived Types

### Objective
I needed to send a 4×5 matrix from process 0 and have process 1 receive it already transposed as a 5×4 matrix, using a single send/receive pair. The transposition had to happen implicitly through the datatype definition.

### Implementation Approach

The key insight was that transposing means extracting columns from the source and placing them as rows in the destination. I built this in two stages:

**First, I defined a column pattern** using `MPI_Type_vector`:
```c
MPI_Type_vector(COLS, 1, ROWS, MPI_INT, &column_type);
```
This creates a pattern that extracts 5 consecutive elements from the message buffer and places them with a stride of 4 positions in memory. Essentially, it takes one row from the source and spreads it out to form one column in the destination.

**Then, I applied this pattern multiple times** using `MPI_Type_create_hvector`:
```c
MPI_Type_create_hvector(ROWS, 1, sizeof(int), column_type, &transpose_type);
```
The stride of `sizeof(int)` means each column pattern starts one integer later in the source buffer. This extracts all 4 rows of the source as separate columns in the destination.

### Key Takeaway

The critical parameter was the hvector stride. I initially thought it should be `ROWS × sizeof(int)` (the row width of the destination), but the correct value is just `sizeof(int)` because we're describing where each column *starts* in the source buffer, not the destination layout. The column_type already encodes the destination spacing.

## Exercise 2: Distributed Gradient Descent

### Objective
I parallelized a batch gradient descent algorithm across multiple MPI processes. Each process handles a subset of training samples, computes local gradients, then all processes synchronize to update weights collectively.

### Implementation Approach

**Data Structure Definition:** The training samples contained both a feature vector and a label:
```c
typedef struct {
    double x[N_FEATURES];
    double y;
} Sample;
```

I used `MPI_Type_create_struct` to define this as an MPI datatype. The critical step was computing field offsets using `MPI_Get_address` rather than assuming the compiler's memory layout—this accounts for potential padding between fields.

**Data Distribution:** I used `MPI_Scatterv` to distribute samples across processes. This was necessary because the total sample count might not divide evenly, so some processes get one extra sample. Each process calculates its own `local_n` based on its rank.

**Parallel Training Loop:** Each iteration consists of:
1. Local computation: Each process computes MSE loss and gradient for its samples
2. Global reduction: `MPI_Allreduce` sums all local losses and gradients
3. Synchronous update: All processes apply the same weight update

The algorithm converged to approximately w[0]≈3.0 and w[1]≈-5.0, matching the true model parameters used to generate the synthetic data (y = 3x₀ - 5x₁ + noise).

### Key Decisions

I calculated `local_n` independently on each process rather than broadcasting it, which reduces communication overhead.

The gradient computation uses the standard MSE gradient formula: ∂L/∂w_j = 2/N × Σ(error × x_j). I accumulate these locally, then average after the global reduction.



## Lessons Learned

1. **Stride semantics matter:** Element stride vs. byte stride (`Type_vector` vs. `Type_create_hvector`) requires careful attention to whether you're describing source or destination layout.

2. **Structure padding is real:** Using `MPI_Get_address` instead of hardcoding offsets prevents bugs on architectures with different alignment requirements.

3. **Scope management in MPI:** Variables used in collective operations and post-loop need careful scope consideration, especially when timing or reporting final states.

4. **Scatterv over Scatter:** When dataset size doesn't divide evenly, `MPI_Scatterv` with calculated counts per rank is cleaner than trying to handle remainders manually.

## Compilation & Execution

Both exercises compiled successfully with MS-MPI:
```bash
gcc -I"..." -L"..." source.c -lmsmpi -o output.exe
mpiexec -n <processes> ./output.exe [args]
```

Exercise 1 requires exactly 2 processes; Exercise 2 scales to arbitrary process counts.
