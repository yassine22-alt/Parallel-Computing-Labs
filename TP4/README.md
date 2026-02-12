# OpenMP Parallel Computing Lab - TP4
## Sections & Synchronization

This lab explored OpenMP's task parallelism using sections, synchronization mechanisms, and their impact on performance. I implemented four exercises that progressively revealed the complexities of parallel programming beyond simple loop parallelization.

---

## Exercise 1: Work Distribution with Parallel Sections

### What I Did
I parallelized statistical computations (sum, max, standard deviation) on a 1M-element array using `#pragma omp sections`. The challenge was handling data dependencies—specifically, standard deviation requires the mean, which depends on the sum.

### Implementation Strategy
I used two parallel sections blocks:
- **First block**: Computed sum and max in parallel (independent tasks)
- **Sequential step**: Calculated mean from sum
- **Second approach**: Parallelized the standard deviation loop using `#pragma omp for reduction` instead of wrapping it in a single section

### Key Insight
My initial implementation wrapped the stddev calculation in a sections block with only ONE section—completely pointless since there's no parallelism. I learned that sections are only beneficial when you have multiple independent tasks. For the stddev computation, using `#pragma omp parallel for reduction` was the correct approach, actually distributing work across threads.

### Takeaway
**Sections vs parallel for**: Sections divide different tasks among threads; parallel for divides iterations of the same task. Don't use sections when you need data parallelism—that's what `for` is for.

---

## Exercise 2: Master vs Single Directives

### What I Did
I implemented matrix initialization and sum computation comparing `#pragma omp master` and `#pragma omp single` directives to understand their synchronization behavior.

### Results (20 threads)
- **Sequential**: 0.850s
- **Parallel**: 0.331s
- **Speedup**: 2.57x

### Analysis
The 2.57x speedup with 20 threads is far from ideal (20x) because initialization and printing are sequential bottlenecks. This is Amdahl's Law in practice: even small sequential portions limit overall speedup.

The key difference I discovered:
- **`master`**: Only thread 0 executes, **no implicit barrier** (other threads continue)
- **`single`**: Any one thread executes, **implicit barrier** (others wait)

### When to Use Each
- `master`: I/O operations or initialization where other threads don't need to  (But still need to wait in order to use the matrix, so need to enforce a barrier)
- `single`: Operations where subsequent code depends on completion (synchronization needed)

---

## Exercise 3: Load Balancing with Task Scheduling

This exercise was eye-opening regarding load balancing. I simulated three tasks with different computational loads:
- Light: N iterations (1x)
- Moderate: 5N iterations (5x)
- Heavy: 20N iterations (20x)

### Results (20 threads, N=1M)
| Approach | Time (s) | Speedup |
|----------|----------|---------|
| Sequential | 0.565 | 1.00x |
| Naive sections (3 tasks) | 0.571 | 0.99x |
| Optimized (split heavy) | 0.209 | 2.70x |
| Task-based | 0.491 | 1.15x |

### What Went Wrong with Naive Sections
The naive approach was actually **slower** than sequential! Here's why:
```
Thread 1: [Light===] .................... (waits 11s)
Thread 2: [Moderate======] ............. (waits 10s)  
Thread 3: [Heavy========================]
```
The heavy task created a massive bottleneck. Threads 1 and 2 finished quickly then sat idle at the implicit barrier, wasting CPU cycles. The parallelization overhead actually made it slower.

### Optimization Success
I split the heavy task into 4 chunks, creating 6 total sections. This achieved 2.70x speedup—2.73x faster than the naive approach. The work distribution became much more balanced.

### Why Tasks Didn't Win
I expected the task-based approach to perform best due to dynamic scheduling, but it only achieved 1.15x speedup. Tasks have overhead from the task queue management, and for this regular workload, manual splitting was more effective.

**Lesson learned**: Dynamic scheduling (tasks) shines with unpredictable or recursive workloads. For known, regular workloads, manual optimization can be more efficient.

---

## Exercise 4: Synchronization Overhead and the nowait Clause

This exercise quantified the cost of implicit barriers in parallel loops and explored the `nowait` clause for optimization.

### Implementation
I implemented matrix-vector multiplication (40,000 × 600) in three versions:
1. **V1**: Static scheduling with implicit barrier
2. **V2**: Dynamic scheduling with `nowait`
3. **V3**: Static scheduling with `nowait`

### Results

| Threads | V1 Time | V1 Speedup | V3 Time | V3 Speedup | V3 Improvement |
|---------|---------|------------|---------|------------|----------------|
| 1 | 0.062s | 1.11x | 0.065s | 1.06x | -4.6% |
| 2 | 0.062s | 1.11x | 0.067s | 1.03x | -7.5% |
| 4 | 0.060s | 1.15x | 0.048s | 1.44x | +20.0% |
| 8 | 0.052s | 1.33x | 0.043s | 1.60x | +17.3% |
| 16 | 0.046s | 1.50x | 0.046s | 1.50x | 0.0% |

### Key Observations

**1. Barrier Overhead Emerges at Medium Thread Counts**
- At 1-2 threads: No benefit from `nowait` (actually slight overhead)
- At 4-8 threads: `nowait` provided 17-20% improvement
- At 16 threads: Both versions converged to same performance

**2. Why Limited Scalability?**
Despite 20 available threads, speedup plateaued at ~1.6x. This is a **memory bandwidth bottleneck**, not a synchronization issue. The matrix-vector multiplication is memory-bound—I'm limited by how fast I can read data from RAM, not by computation speed.

**3. Dynamic vs Static Scheduling**
V2 (dynamic + nowait) performed worse than V3 (static + nowait) because:
- This workload is perfectly balanced (all iterations do equal work)
- Dynamic scheduling adds overhead from the task queue
- Static scheduling is optimal for balanced workloads

### When is nowait Dangerous?

I learned that `nowait` removes the implicit barrier, which is **dangerous** when subsequent code depends on the parallel loop's completion:

```c
// DANGEROUS CODE
#pragma omp parallel
{
    #pragma omp for nowait
    for (int i = 0; i < N; i++)
        A[i] = compute(i);  // Some threads may not finish!
    
    #pragma omp for
    for (int i = 0; i < N; i++)
        B[i] = A[i] * 2;    // Race condition! May read incomplete A[i]
}
```

In this exercise, `nowait` was **safe** because:
- Each iteration is independent
- No code after the loop depends on the results within the same parallel region
- The parallel region ends immediately after, which has an implicit barrier


## Overall Conclusions

1. **Sections are for task parallelism**: Use them to divide different work among threads, not to wrap single operations pointlessly.

2. **Load balancing is critical**: Unbalanced sections create bottlenecks where fast threads wait for slow ones. Manual splitting can outperform dynamic scheduling for regular workloads.

3. **Barriers have real cost**: At medium thread counts (4-8), removing unnecessary barriers with `nowait` improved performance by 17-20%. But always verify safety first.

4. **Memory bandwidth limits scalability**: Even with perfect parallelization, I hit a ceiling due to hardware constraints. Not all problems scale linearly with threads.

5. **Know your bottleneck**: 
   - Compute-bound → add more threads
   - Memory-bound → optimize data access patterns
   - Synchronization-bound → use `nowait` or better work distribution

This lab demonstrated that effective parallel programming requires understanding not just the OpenMP syntax, but the performance characteristics and tradeoffs of different parallelization strategies.
