# Exercise 1: Loop Unrolling Analysis
**Foundations of Parallel Computing - TP2**

---

## Experimental Setup

- **Array Size:** N = 10,000,000 elements
- **Operation:** Sum of array elements (simple addition)
- **Data Types Tested:** `double` (8B), `float` (4B), `int` (4B), `short` (2B)
- **Unrolling Factors:** U = 1, 2, 4, 8, 16, 32
- **Optimization Levels:** -O0 (no optimization), -O2 (full optimization)
- **Compiler:** GCC with inline assembly barrier to prevent dead code elimination

---

## Key Results Summary

### Best Performance by Data Type

| Type | Size | Data Volume | -O0 Best | -O2 Best | Speedup (-O2/-O0) |
|------|------|-------------|----------|----------|-------------------|
| **short** | 2 bytes | 20 MB | 7ms (U=32) | 2ms (U=1) | 3.5× |
| **int** | 4 bytes | 40 MB | 6ms (U=32) | 3ms (U=1,8,16) | 2× |
| **float** | 4 bytes | 40 MB | 5ms (U=32) | 4ms (U=4,16,32) | 1.25× |
| **double** | 8 bytes | 80 MB | 7ms (U=32) | 6ms (U=16,32) | 1.17× |

---

## What We Discovered

### 1. Manual Unrolling vs Compiler Optimization

**At -O0:** Loop unrolling shows dramatic improvements
- `double` U=1: 38ms → U=32: 7ms (**5.4× speedup**)
- Reducing loop overhead (branch instructions, counter increments) is the main benefit
- More ILP (Instruction-Level Parallelism) allows CPU to pipeline additions

**At -O2:** Compiler already does significant optimization
- `double` U=1: 9ms (compiler auto-optimizes) vs -O0 U=1: 38ms (**4.2× improvement**)
- Manual unrolling still helps: -O2 U=1: 9ms → U=32: 6ms (**1.5× additional speedup**)
- **Conclusion:** Compiler optimization gives most of the benefit, but manual unrolling can still add 30-50% improvement

---

### 2. The Bandwidth Wall

**Why didn't performance scale linearly with unrolling?**

At some point, you hit the **memory bandwidth limit**. Your CPU can compute faster than memory can deliver data.




**We're bandwidth-limited!** This explains why:
- Smaller data types are faster (less data to transfer)
- Beyond U=4-8, more unrolling doesn't help much
- You can't go faster than memory can feed the CPU

---

### 3. Instruction Cache Thrashing

**Look at this anomaly:**
- `short` -O2 U=16: 5ms
- `short` -O2 U=32: **14ms** (worse!)

**Why?** The unrolled loop body at U=32 is HUGE. It doesn't fit in the L1 instruction cache (~32KB). The CPU must reload instructions from slower L2/L3 cache every iteration.

**Rule:** Aggressive unrolling can backfire if it bloats code size beyond i-cache capacity.

This hits fast operations (like short addition) hardest because they spend more time waiting for instructions than computing.

---

### 4. Integer vs Floating-Point Performance

**Surprising finding:**
- `int` (4 bytes): Best time 3ms
- `float` (4 bytes): Best time 4ms

Same data size, but `int` is faster! **Why?**
- Integer addition uses integer ALU (faster)
- Floating-point uses FPU (can be slower on some CPUs)
- Integer operations have simpler logic (no rounding, no special cases)

---

## How to Reason About Optimal Unrolling

### Step 1: Identify the Bottleneck

**If** measured_time ≈ data_size / bandwidth:
→ **Bandwidth-limited** (like our case)
→ Aggressive unrolling won't help much
→ Watch out for i-cache thrashing

**If** measured_time >> data_size / bandwidth:
→ **Compute-limited**
→ Unrolling helps significantly (more ILP)
→ Keep going until you saturate execution units

---

### Step 2: Apply Heuristics

**For FAST operations** (simple addition on small types):
- Operations complete quickly
- Hit bandwidth limit early
- Large unrolling → i-cache issues dominate
- **Optimal: Low to medium unrolling (U=2-8)**

**For SLOW operations** (complex math, large types):
- Operations take more time
- More room for ILP benefits
- I-cache impact relatively smaller
- **Optimal: Medium to high unrolling (U=8-32)**

**Our data confirms this:**
- `short` -O2: Best at U=1-2 (low)
- `int` -O2: Best at U=1-8 (low-medium)
- `double` -O2: Best at U=16-32 (high)

---

## Practical Takeaways

### When Manual Unrolling Matters:
1. **Performance-critical hot loops** identified by profiling
2. **Simple operations** where compiler can't auto-vectorize effectively
3. **Embedded systems** where you know exact hardware characteristics
4. When you need that **last 30-50%** after compiler optimization

### When It Doesn't Matter:
1. Already using **-O2/-O3** optimization (gets you 80% of the way)
2. **Complex algorithms** where algorithmic improvements yield more
3. **Bandwidth-limited** code (like our array sum)
4. Code **maintainability** is more important than micro-optimization

---

## The Complete Mental Model

```
Performance = min(Compute_Speed, Memory_Bandwidth)

Manual Unrolling Benefits:
├─ Reduces loop overhead (fewer branches)
├─ Increases ILP (more parallel additions)
└─ Diminishing returns when:
    ├─ Bandwidth-limited (can't fetch data fast enough)
    ├─ I-cache thrashing (code too big)
    └─ Execution units saturated (CPU maxed out)

Compiler -O2 Already Does:
├─ Automatic loop unrolling
├─ Vectorization (SIMD instructions)
├─ Register allocation
└─ Instruction scheduling

Manual Unrolling Adds:
└─ Architecture-specific tuning (if you know your CPU)
└─ Sometimes better than compiler (30-50% improvement)
```

---

## Final Wisdom

**The compiler is smart, but not magic.** At -O2, it gives you most of the benefit automatically. Manual unrolling is a micro-optimization that:
- Can add 30-50% on top of compiler optimization
- Is only worth it for proven bottlenecks
- Requires understanding your hardware (cache sizes, bandwidth, execution units)
- Can backfire if you over-optimize (i-cache thrashing)

**Best practice:** Profile first, optimize algorithmic bottlenecks, enable -O2, then—and only then—consider manual unrolling for the hottest 1% of your code.

---

**Date:** January 29, 2026  
**System Specs:** Windows, GCC compiler, ~13.3 GB/s memory bandwidth
