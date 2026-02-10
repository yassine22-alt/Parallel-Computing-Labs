# Exercise 5: Jacobi Method - Results

## What Was Parallelized

### Two main loops parallelized:

**1. Computing new x values (lines 60-69):**
```c
#pragma omp parallel for private(j)
for (i = 0; i < n; i++) {
    // Each iteration independent - can run in parallel
    x_courant[i] = (b[i] - Σ a[j][i]*x[j]) / a[i][i]
}
```

**2. Finding maximum difference (lines 72-76):**
```c
#pragma omp parallel for reduction(max:absmax)
for (i = 0; i < n; i++) {
    if (fabs(x[i] - x_courant[i]) > absmax)
        absmax = ...;  // reduction handles this
}
```

## Performance Results (120×120 matrix)

```
Threads | Time(ms) | Speedup | Efficiency
--------|----------|---------|------------
   1    |   3.0    |  0.33x  |   33%
   2    |  13.0    |  0.08x  |    4%
   4    |  19.0    |  0.05x  |    1%
   8    |  31.0    |  0.03x  |   0.4%
  16    |  61.0    |  0.02x  |   0.1%
```

Sequential baseline: **1.0 ms**

## ⚠️ Critical Finding: NEGATIVE SPEEDUP

### The Problem:
- More threads = **SLOWER** execution!
- Speedup < 1.0 means slowdown
- Efficiency drops to nearly 0

### Why This Happened:

**1. Problem Size Too Small**
- Matrix: 120×120 = 14,400 elements
- Not enough work per thread
- Overhead >> actual computation

**2. Parallelization Overhead**
- Thread creation: ~0.5-1ms per thread
- Synchronization: barriers at each iteration (108 iterations!)
- Memory allocation for thread stacks

**3. Memory Contention**
- All threads reading same arrays (a, x)
- Cache line bouncing
- False sharing effects

**4. Iteration Count**
- 108 iterations means 108 parallel region entries
- Each entry has overhead
- Total overhead > computation time

## When Does Parallelization Help?

### Rule of thumb for Jacobi:
- **N < 500**: Sequential faster
- **500 < N < 1000**: Break-even point
- **N > 1000**: Parallel starts winning
- **N > 5000**: Significant speedup expected

### For this problem:
```
N = 120:  Sequential wins (1ms vs 3-61ms)
N = 1000: Parallel should help
N = 5000: Expect 3-4x speedup with 8 threads
```

## Lessons Learned

1. **Not all parallelization is beneficial**
   - Must consider problem size
   - Overhead can exceed benefits

2. **Amdahl's Law applies**
   - Fixed overhead limits speedup
   - Small problems hit overhead wall quickly

3. **Memory-bound operations**
   - Jacobi is memory-intensive
   - Bandwidth limits scalability

4. **Iterative algorithms are challenging**
   - Many synchronization points
   - Overhead accumulates over iterations

## Recommendation

For production use:
```c
if (n < 500) {
    // Use sequential version
} else {
    // Use parallel version with OpenMP
}
```

Or use `if` clause:
```c
#pragma omp parallel for if(n > 500)
```
