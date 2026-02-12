import matplotlib.pyplot as plt
import numpy as np

# Exercise 4 Results
threads = np.array([1, 2, 4, 8, 16])

# Version 1: Implicit Barrier
v1_time = np.array([0.0620, 0.0620, 0.0600, 0.0520, 0.0460])
v1_speedup = np.array([1.11, 1.11, 1.15, 1.33, 1.50])
v1_efficiency = np.array([111.3, 55.6, 28.7, 16.6, 9.4])
v1_mflops = np.array([774.19, 774.19, 800.00, 923.08, 1043.48])

# Version 3: Static + nowait
v3_time = np.array([0.0650, 0.0670, 0.0480, 0.0430, 0.0460])
v3_speedup = np.array([1.06, 1.03, 1.44, 1.60, 1.50])
v3_efficiency = np.array([106.2, 51.5, 35.9, 20.1, 9.4])
v3_mflops = np.array([738.46, 716.42, 1000.00, 1116.28, 1043.48])

# Create figure with 4 subplots
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
fig.suptitle('Exercise 4: Barrier Overhead Analysis (Matrix-Vector Multiplication)', 
             fontsize=16, fontweight='bold')

# Plot 1: CPU Time
ax1 = axes[0, 0]
ax1.plot(threads, v1_time, 'o-', linewidth=2, markersize=8, label='V1: Implicit Barrier')
ax1.plot(threads, v3_time, 's-', linewidth=2, markersize=8, label='V3: Static + nowait')
ax1.set_xlabel('Number of Threads', fontsize=12)
ax1.set_ylabel('Execution Time (seconds)', fontsize=12)
ax1.set_title('CPU Time vs Threads', fontsize=13, fontweight='bold')
ax1.grid(True, alpha=0.3)
ax1.legend(fontsize=10)
ax1.set_xticks(threads)

# Plot 2: Speedup
ax2 = axes[0, 1]
ax2.plot(threads, v1_speedup, 'o-', linewidth=2, markersize=8, label='V1: Implicit Barrier')
ax2.plot(threads, v3_speedup, 's-', linewidth=2, markersize=8, label='V3: Static + nowait')
ax2.plot(threads, threads, '--', linewidth=1.5, color='gray', alpha=0.7, label='Ideal (Linear)')
ax2.set_xlabel('Number of Threads', fontsize=12)
ax2.set_ylabel('Speedup', fontsize=12)
ax2.set_title('Speedup vs Threads', fontsize=13, fontweight='bold')
ax2.grid(True, alpha=0.3)
ax2.legend(fontsize=10)
ax2.set_xticks(threads)

# Plot 3: Efficiency
ax3 = axes[1, 0]
ax3.plot(threads, v1_efficiency, 'o-', linewidth=2, markersize=8, label='V1: Implicit Barrier')
ax3.plot(threads, v3_efficiency, 's-', linewidth=2, markersize=8, label='V3: Static + nowait')
ax3.axhline(y=100, linestyle='--', color='gray', linewidth=1.5, alpha=0.7, label='Ideal (100%)')
ax3.set_xlabel('Number of Threads', fontsize=12)
ax3.set_ylabel('Efficiency (%)', fontsize=12)
ax3.set_title('Efficiency vs Threads', fontsize=13, fontweight='bold')
ax3.grid(True, alpha=0.3)
ax3.legend(fontsize=10)
ax3.set_xticks(threads)

# Plot 4: MFLOPS
ax4 = axes[1, 1]
ax4.plot(threads, v1_mflops, 'o-', linewidth=2, markersize=8, label='V1: Implicit Barrier')
ax4.plot(threads, v3_mflops, 's-', linewidth=2, markersize=8, label='V3: Static + nowait')
ax4.set_xlabel('Number of Threads', fontsize=12)
ax4.set_ylabel('MFLOP/s', fontsize=12)
ax4.set_title('Performance (MFLOP/s) vs Threads', fontsize=13, fontweight='bold')
ax4.grid(True, alpha=0.3)
ax4.legend(fontsize=10)
ax4.set_xticks(threads)

plt.tight_layout()
plt.savefig('exercise4_results.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'exercise4_results.png'")
plt.show()

# Print summary statistics
print("\n=== SUMMARY STATISTICS ===")
print(f"\nBest speedup achieved:")
print(f"  V1 (barrier):  {v1_speedup.max():.2f}x at {threads[np.argmax(v1_speedup)]} threads")
print(f"  V3 (nowait):   {v3_speedup.max():.2f}x at {threads[np.argmax(v3_speedup)]} threads")

print(f"\nV3 improvement over V1:")
improvement = ((v1_time - v3_time) / v1_time * 100)
for i, t in enumerate(threads):
    print(f"  {t} threads: {improvement[i]:+.1f}% {'(faster)' if improvement[i] > 0 else '(slower)'}")

print(f"\nKey insight:")
print(f"  - nowait helped most at 8 threads: {improvement[3]:.1f}% faster")
print(f"  - Both versions plateau at 16 threads (same performance)")
print(f"  - Limited scalability due to memory bandwidth bottleneck")
