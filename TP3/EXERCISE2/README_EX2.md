# Exercise 2: PI Calculation - Parallelization Guide

## Overview
This exercise parallelizes a PI calculation using numerical integration (midpoint rule).

## Mathematical Background
- PI is calculated using: ∫₀¹ 4/(1+x²) dx = π
- We approximate the integral using Riemann sum with rectangles
- More steps = better accuracy

## Key Concepts

### 1. Variable Scoping
- **Shared variables**: Same memory location for all threads
  - `num_steps`, `step` - read-only, safe
  - `sum` - needs synchronization!
  
- **Private variables**: Each thread has own copy
  - `i`, `x` - must be private (loop variables)
  - `partial_sum` - private accumulator

### 2. Work Distribution
Since we can't use `#pragma omp parallel for`, we manually distribute:
```c
for (i = thread_id; i < num_steps; i += num_threads)
```
- Thread 0: iterations 0, 4, 8, 12, ...
- Thread 1: iterations 1, 5, 9, 13, ...
- Thread 2: iterations 2, 6, 10, 14, ...
- Thread 3: iterations 3, 7, 11, 15, ...

### 3. Two Synchronization Approaches

#### Approach 1: REDUCTION (pi_parallel.c) - RECOMMENDED
```c
#pragma omp parallel reduction(+:sum)
{
    for (i = thread_id; i < num_steps; i += num_threads) {
        sum += 4.0 / (1.0 + x * x);
    }
}
```
- Each thread has private copy of `sum`
- OpenMP automatically combines at end
- Better performance (no synchronization overhead during computation)

#### Approach 2: CRITICAL SECTION (pi_critical.c)
```c
#pragma omp parallel
{
    double partial_sum = 0.0;
    
    for (...) {
        partial_sum += ...;
    }
    
    #pragma omp critical
    {
        sum += partial_sum;
    }
}
```
- Each thread uses local `partial_sum`
- Critical section protects final update
- Only one thread can enter critical section at a time

## Files

1. **pi_sequential.c** - Sequential baseline for comparison
2. **pi_parallel.c** - Parallel version using REDUCTION (recommended)
3. **pi_critical.c** - Parallel version using CRITICAL SECTION

## Compilation
```bash
gcc -fopenmp -Wall -o pi_sequential pi_sequential.c
gcc -fopenmp -Wall -o pi_parallel pi_parallel.c
gcc -fopenmp -Wall -o pi_critical pi_critical.c
```

## Running
```bash
# Sequential
./pi_sequential

# Parallel with N threads
$env:OMP_NUM_THREADS=N
./pi_parallel
./pi_critical
```

## Performance Testing
Run the test script:
```bash
./test_pi.ps1
```

## Expected Results
- PI value: ~3.141592653589793
- Parallel version should show speedup with more threads
- Both parallel approaches give same result

## Key OpenMP Functions
- `omp_get_wtime()` - Get wall-clock time
- `omp_get_thread_num()` - Get thread ID (0 to N-1)
- `omp_get_num_threads()` - Get total threads

## Important Notes
- The calculation is correct (matches true π value closely)
- Thread output may vary slightly due to floating-point rounding
- Reduction is generally faster than critical section
- For very small workloads, overhead may exceed benefits
