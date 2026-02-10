import matplotlib.pyplot as plt
import numpy as np

# Data from benchmark
threads = np.array([1, 2, 4, 8, 16])
times = np.array([1.406, 0.847, 0.756, 0.525, 0.442])
speedup = times[0] / times
efficiency = speedup / threads

# Create figure with 2 subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# Plot 1: Speedup
ax1.plot(threads, speedup, 'b-o', linewidth=2, markersize=8, label='Actual Speedup')
ax1.plot(threads, threads, 'r--', linewidth=2, label='Ideal Speedup (Linear)')
ax1.set_xlabel('Number of Threads', fontsize=12)
ax1.set_ylabel('Speedup', fontsize=12)
ax1.set_title('Speedup vs Number of Threads\nMatrix Multiplication (1000x1000)', fontsize=14)
ax1.grid(True, alpha=0.3)
ax1.legend(fontsize=10)
ax1.set_xticks(threads)

# Add values on points
for i, (t, s) in enumerate(zip(threads, speedup)):
    ax1.annotate(f'{s:.2f}', (t, s), textcoords="offset points", 
                xytext=(0,10), ha='center', fontsize=9)

# Plot 2: Efficiency
ax2.plot(threads, efficiency, 'g-s', linewidth=2, markersize=8, label='Actual Efficiency')
ax2.axhline(y=1.0, color='r', linestyle='--', linewidth=2, label='Ideal Efficiency')
ax2.set_xlabel('Number of Threads', fontsize=12)
ax2.set_ylabel('Efficiency', fontsize=12)
ax2.set_title('Efficiency vs Number of Threads\nMatrix Multiplication (1000x1000)', fontsize=14)
ax2.grid(True, alpha=0.3)
ax2.legend(fontsize=10)
ax2.set_xticks(threads)
ax2.set_ylim([0, 1.2])

# Add values on points
for i, (t, e) in enumerate(zip(threads, efficiency)):
    ax2.annotate(f'{e:.3f}', (t, e), textcoords="offset points", 
                xytext=(0,10), ha='center', fontsize=9)

plt.tight_layout()
plt.savefig('speedup_efficiency.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'speedup_efficiency.png'")
plt.show()

# Print summary
print("\n========================================")
print("Performance Analysis Summary")
print("========================================")
print(f"{'Threads':<10} {'Time(s)':<12} {'Speedup':<12} {'Efficiency':<12}")
print("-" * 48)
for t, time, sp, eff in zip(threads, times, speedup, efficiency):
    print(f"{t:<10} {time:<12.3f} {sp:<12.3f} {eff:<12.3f}")
print("========================================")
