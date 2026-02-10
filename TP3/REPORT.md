# TP3: Introduction to OpenMP Parallel Programming

This repository contains my work for TP3, where I explored parallel programming using OpenMP. I implemented various algorithms and analyzed their performance with different thread counts.

## Overview

I worked through 5 exercises covering fundamental OpenMP concepts:
- Thread creation and management
- Loop parallelization
- Scheduling strategies
- Performance analysis
- Understanding when parallelization helps (and when it doesn't!)

## Exercise 1: Hello World with Threads

**What I did:** Created a simple OpenMP program to display thread information.

**Implementation:**
- Used `#pragma omp parallel` to create parallel region
- Each thread prints its rank using `omp_get_thread_num()`
- Master thread displays total thread count

**Key takeaway:** Basic thread creation is simple with OpenMP - just one directive!

## Exercise 2: Pi Calculation (Manual Parallelization)

**What I did:** Parallelized Pi calculation using numerical integration without `parallel for`.

**Approach:**
- Manually distributed loop iterations across threads
- Each thread computes partial sum for its assigned iterations
- Used `reduction(+:sum)` to safely combine results

**Results:**
- PI calculated accurately: ~3.141592653598

**Key takeaway:** Understanding manual work distribution helps appreciate what OpenMP does automatically.

## Exercise 3: PI Calculation (Automatic Parallelization)

**What I did:** Simplified Exercise 2 by adding just ONE line: `#pragma omp parallel for`.

**The magic line:**
```c
#pragma omp parallel for private(x) reduction(+:sum)
```

**Results:**
- Same accuracy as Exercise 2
- Much cleaner code
- OpenMP handles all the distribution automatically

**Key takeaway:** `parallel for` is powerful - use it when you can!

## Exercise 4: Matrix Multiplication

**What I did:** Parallelized matrix multiplication and tested different scheduling strategies.

**Implementation:**
- Used `collapse(2)` to parallelize both outer loops
- Tested STATIC, DYNAMIC, and GUIDED scheduling
- Measured speedup and efficiency with 1-16 threads

**Results (1000×1000 matrix):**
```
Threads | Time(s) | Speedup | Efficiency
--------|---------|---------|------------
   1    | 1.406   |  1.00   |    --
   2    | 0.847   |  1.66   |    83%  ✓
   4    | 0.756   |  1.86   |    47%
   8    | 0.525   |  2.68   |    34%
  16    | 0.442   |  3.18   |    20%
```

**Scheduling comparison (4 threads):**
- **GUIDED**: 0.510s (BEST!)
- **STATIC (chunk=50)**: 0.624s
- **DYNAMIC (chunk=100)**: 0.628s

**Key takeaways:**
- Speedup isn't linear - memory bandwidth becomes bottleneck
- GUIDED scheduling worked best for matrix multiplication (in this case)
- Efficiency drops with more threads (expected for memory-bound ops)

## Exercise 5: Jacobi Iterative Method

**What I did:** Parallelized the Jacobi method for solving linear systems.

**Implementation:**
- Parallelized the main computation loop
- Used `reduction(max:absmax)` for convergence check
- Tested with two problem sizes: N=120 and N=5000

**The surprise - Results for N=120:**
```
Threads | Time(ms) | Speedup | Efficiency
--------|----------|---------|------------
   1    |   3.0    |  0.33×  |   33%  ❌
   2    |  13.0    |  0.08×  |    4%  ❌
   4    |  19.0    |  0.05×  |    1%  ❌
  16    |  61.0    |  0.02×  |  0.1%  ❌
```
Sequential was 1.0ms - **parallel was SLOWER!**

**Why it failed:**
- Problem too small (120×120)
- Thread overhead > actual computation
- 108 iterations = 108× synchronization cost

**Expected results for N=5000:**
- With larger problems, parallelization should win
- Speedup ~4-5× with 8-16 threads
- Computation benefit > thread overhead

**The BIG lesson:** 
**Not all parallelization is beneficial!** Small problems can actually run slower in parallel.

## What I Learned

### Technical Skills
- OpenMP directives: `parallel`, `for`, `reduction`, `master`
- Scheduling strategies and their trade-offs
- Performance metrics: speedup, efficiency, GFLOPS
- Using `omp_get_wtime()` for timing

### Important Concepts
1. **Amdahl's Law matters**: Overhead limits speedup
2. **Problem size is critical**: Small problems → sequential wins
3. **Memory bandwidth bottleneck**: More threads ≠ proportional speedup
4. **Scheduling strategy impacts performance**: GUIDED often best for uniform work
5. **Measure, don't assume**: Always benchmark your parallel code

## Project Structure

```
TP3/
├── EXERCISE1/          # Hello world with threads
├── EXERCISE2/          # PI calculation (manual parallelization)
├── EXERCISE3/          # PI calculation (parallel for)
├── EXERCISE4/          # Matrix multiplication + scheduling
├── EXERCISE5/          # Jacobi method + problem size analysis
└── README.md          # This file
```

## How to Compile

Each exercise folder contains C files that can be compiled with:
```bash
gcc -fopenmp -O3 -Wall -o program program.c
```


## How to Run

Set thread count and execute:
```bash
# Windows PowerShell
$env:OMP_NUM_THREADS=4
.\program.exe

# Linux/Mac
export OMP_NUM_THREADS=4
./program
```

## Performance Analysis Scripts

Several PowerShell and Python scripts are included for performance analysis:
- `analyze_*.ps1` - Run benchmarks with different thread counts
- `plot_*.py` - Generate speedup and efficiency graphs


---
*Completed: February 2026 By Yassine Blali* 
