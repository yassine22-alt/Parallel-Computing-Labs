# Exercise 4 Results Summary

## Speedup and Efficiency Analysis

### Results Table
```
Threads | Time(s)  | Speedup | Efficiency
--------|----------|---------|------------
      1 | 1.406    |   1.000 |      1.000
      2 | 0.847    |   1.660 |      0.830
      4 | 0.756    |   1.860 |      0.465
      8 | 0.525    |   2.678 |      0.335
     16 | 0.442    |   3.181 |      0.199
```

### Observations:
- **Speedup**: Increases with more threads but not linearly
  - 2 threads: 1.66x speedup (good)
  - 4 threads: 1.86x speedup (moderate)
  - 8 threads: 2.68x speedup (decreasing returns)
  - 16 threads: 3.18x speedup (diminishing returns)

- **Efficiency**: Decreases as thread count increases
  - Ideal efficiency = 1.0 (100%)
  - 2 threads: 83% efficiency (very good)
  - 4 threads: 46% efficiency (moderate)
  - 16 threads: 20% efficiency (poor)

### Why Not Linear Speedup?
1. **Overhead**: Thread creation, synchronization
2. **Memory bandwidth**: All threads compete for memory access
3. **Cache effects**: Cache misses increase with more threads
4. **False sharing**: Cache line contention

## Scheduling Strategy Comparison (4 threads)

### Best Performance by Strategy:
```
Strategy         | Best Time | GFLOPS 
-----------------|-----------|--------
STATIC (chunk=50)|  0.624s   | 3.205  
DYNAMIC (chunk=100)| 0.628s  | 3.185  
GUIDED (default) |  0.510s   | 3.922   
```

### Analysis:

**STATIC Scheduling:**
- Best with chunk_size=50: 0.624s
- Low overhead, predictable
- Good for uniform workload

**DYNAMIC Scheduling:**
- Best with chunk_size=100: 0.628s
- More overhead than static
- Not needed for matrix mult (uniform work)

**GUIDED Scheduling: WINNER!**
- Best overall: 0.510s (3.922 GFLOPS)
- Adaptive chunk sizing
- Balances load automatically
- Best for this workload

### Recommendations:
1. **Use GUIDED scheduling** for matrix multiplication
2. For uniform workloads, STATIC with appropriate chunk is good
3. DYNAMIC only needed for highly imbalanced workloads
4. Chunk size matters: too small = overhead, too large = imbalance

## To Generate Plots:
```bash
python plot_speedup.py
```

This will create `speedup_efficiency.png` with visualizations.
