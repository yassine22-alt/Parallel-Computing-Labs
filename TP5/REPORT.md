# MPI Parallel Computing Lab Report

**Author:** TP5 Lab Session  
**Date:** February 20, 2026  
**Topic:** Message Passing Interface (MPI) - Point-to-Point and Collective Communications

---

## Introduction

In this lab, I explored parallel computing using MPI (Message Passing Interface), working through five progressively complex exercises. My goal was to understand how processes communicate, distribute work, and achieve performance gains through parallelization. I tested all implementations on a single machine with up to 8 processes.

---

## Exercise 1: Hello World - Getting Started with MPI

I started with the basics: initializing MPI, getting process ranks, and understanding the SPMD (Single Program Multiple Data) model. Every process runs the same code but behaves differently based on its rank.

**Key Takeaway:** I learned that `MPI_Finalize()` is mandatory. When I omitted it, MPI detected the abnormal termination and threw errors like "process exited without calling finalize." This taught me that MPI maintains state that must be properly cleaned up.

**What Worked:** Using `if (rank == 0)` to control which process prints specific messages. This pattern became crucial in later exercises.

---

## Exercise 2: Broadcasting Data

Here I implemented a program where process 0 reads user input and broadcasts it to all other processes using `MPI_Bcast`. This was my first encounter with collective operations.

**Results:** With 4 processes, when I entered values like 5 and 77, all processes received them correctly. The output order was non-deterministic (process 2 sometimes printed before process 1), which made sense since processes run independently.

**Important Insight:** `MPI_Bcast` is a collective operation - **all processes must call it**, even though only rank 0 provides the data. If one process skips the call, the program hangs waiting for synchronization. I learned this the hard way during debugging.

**Performance:** MPI_Bcast uses tree-based distribution internally, making it efficient for one-to-all communication compared to manually sending to each process.

---

## Exercise 3: Ring Communication Pattern

This exercise taught me about point-to-point communication using `MPI_Send` and `MPI_Recv`. I implemented a ring where each process receives a value, adds its rank, and forwards to the next process.

**Results:** With 7 processes and input value 22, I got a cumulative sum:
- Process 0: 22 (initial)
- Process 6: 43 (final = 22 + 0+1+2+3+4+5+6 = 43)

**Critical Lesson:** The order of operations matters! I initially tried to send before receiving (except for rank 0), which caused deadlocks. The correct pattern is:
- Process 0: Send only
- Middle processes: Receive → Process → Send
- Last process: Receive only

**Performance Limitation:** Ring communication has O(n) latency - each process waits for all previous ones. This sequential dependency makes it slower than tree-based patterns for large process counts.

---

## Exercise 4: Matrix-Vector Multiplication - Reality Check

This was my first real performance test. I parallelized matrix-vector multiplication by distributing matrix rows across processes using `MPI_Scatterv` and gathering results with `MPI_Gatherv`.

**Handling Uneven Division:** I used `MPI_Scatterv` instead of regular `MPI_Scatter` to handle cases where N isn't divisible by P. The first (N % P) processes get one extra row.

**Results - The Disappointment:**

| Matrix Size | Processes | Speedup | Efficiency |
|-------------|-----------|---------|------------|
| 1000×1000   | 4         | 0.73    | 18.2%      |
| 4000×4000   | 8         | 0.83    | 10.3%      |
| 8000×8000   | 8         | 0.95    | 11.8%      |

**The speedup never exceeded 1.0!** The parallel version was actually slower than serial for most cases.

**Why This Happened:**

I analyzed the communication overhead for the 8000×8000 case:
- Serial computation time: 0.128 seconds
- Expected parallel time: 0.128/8 = 0.016 seconds
- Actual parallel time: 0.135 seconds
- **Communication overhead: ~0.119 seconds**

The problem is memory-bandwidth limited on a single machine:
- `MPI_Scatterv` transfers ~64MB of matrix data per process
- `MPI_Bcast` broadcasts the vector (64KB)
- `MPI_Gatherv` collects results

All this happens on the same shared memory bus, so instead of 8x the bandwidth, I'm competing for the same resources.

**Correctness:** Despite poor performance, my implementation was correct - maximum difference between parallel and serial results: 0.000000e+00.

**Lessons I Learned:**
1. MPI isn't always faster, especially on single machines
2. Communication overhead can dominate for memory-bound operations
3. OpenMP would have been better here (shared memory, no message passing)
4. This would perform well on a distributed cluster with independent memory systems

---

## Exercise 5: Pi Calculation - Success!

Finally, I implemented Pi calculation using numerical integration. Each process computes part of the sum, then `MPI_Reduce` combines them with `MPI_SUM`.

**Formula:** π = (4/N) × Σ[1/(1+x²)] where x = (i+0.5)/N

**Results - Real Speedup:**

| N           | Processes | Speedup | Efficiency | Pi Accuracy    |
|-------------|-----------|---------|------------|----------------|
| 10M         | 8         | 3.34x   | 41.8%      | 10⁻¹⁴ error    |
| 100M        | 4         | 2.57x   | 64.2%      | 10⁻¹³ error    |
| 100M        | 8         | 2.85x   | 35.6%      | 10⁻¹⁴ error    |

**This actually worked!** I achieved real speedup (>1.0) for the first time.

**Why This Succeeded Where Matrix-Vector Failed:**

1. **Minimal Communication:** Only one `MPI_Reduce` at the end (8 doubles total)
2. **High Compute-to-Communication Ratio:** Billions of calculations vs. sending 8 numbers
3. **Embarrassingly Parallel:** No dependencies between iterations
4. **Perfect Load Balancing:** Work differs by at most 1 iteration

**Efficiency Trade-off:** I noticed efficiency drops as I add more processes (64% with 4 processes vs 36% with 8). This is normal - the fixed communication cost gets amortized less effectively, and there's simply less work per process.

---

## Overall Conclusions

### What I Learned About MPI

1. **Collective vs Point-to-Point:**
   - Collective operations (`MPI_Bcast`, `MPI_Reduce`) are optimized and easier to use
   - Point-to-point (`MPI_Send`/`MPI_Recv`) gives fine control but requires careful ordering

2. **Communication is Expensive:**
   - On a single machine, communication through MPI has significant overhead
   - The compute-to-communication ratio determines whether parallelization helps
   - Pi calculation (minimal communication) scaled well; matrix-vector (heavy communication) didn't

3. **When to Use MPI:**
   - ✓ Distributed systems (multiple machines)
   - ✓ Compute-intensive problems with little communication
   - ✓ Problems that need to scale beyond one machine
   - ✗ Single-machine shared-memory problems (use OpenMP instead)
   - ✗ Communication-heavy operations on shared resources

4. **Practical Skills:**
   - Handling uneven work distribution (N % P ≠ 0)
   - Measuring performance correctly (barriers before timing)
   - Verifying correctness (comparing parallel vs serial)
   - Debugging parallel programs (deadlocks from wrong send/receive order)



---

## Technical Environment

- **System:** Windows with MS-MPI
- **Compiler:** gcc with MS-MPI SDK
- **Maximum Processes:** 8
- **Timing:** `MPI_Wtime()` for wall-clock measurements

All experiments ran on a single machine, which explains the communication bottleneck in Exercise 4. MPI truly shines in distributed environments where each node has independent resources.
