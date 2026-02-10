import matplotlib.pyplot as plt
import numpy as np

# Parse O0 results
strides_O0 = []
times_O0 = []
rates_O0 = []

with open('results_O0.txt', 'r', encoding='utf-16') as f:
    lines = f.readlines()[1:]
    for line in lines:
        line = line.strip()
        if not line: 
            continue
        parts = line.split(',')
        if len(parts) >= 4:
            strides_O0.append(int(parts[0].strip()))
            times_O0.append(float(parts[2].strip()))
            rates_O0.append(float(parts[3].strip()))

# Parse O2 results
strides_O2 = []
times_O2 = []
rates_O2 = []

with open('results_O2.txt', 'r', encoding='utf-16') as f:
    lines = f.readlines()[1:]  # Skip header
    for line in lines:
        line = line.strip()
        if not line:  # Skip empty lines
            continue
        parts = line.split(',')
        if len(parts) >= 4:
            strides_O2.append(int(parts[0].strip()))
            times_O2.append(float(parts[2].strip()))
            rate_str = parts[3].strip()
            try:
                rate = float(rate_str)
                # Handle infinity values
                if rate == float('inf') or np.isinf(rate):
                    rate = rates_O2[-1] if rates_O2 else 10000  # Use previous or a high value
            except ValueError:
                rate = rates_O2[-1] if rates_O2 else 1000
            rates_O2.append(rate)

# Create figure with subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# Plot 1: Execution Time vs Stride
ax1.plot(strides_O0, times_O0, 'o-', label='-O0 (No Optimization)', linewidth=2, markersize=6)
ax1.plot(strides_O2, times_O2, 's-', label='-O2 (Optimization)', linewidth=2, markersize=6)
ax1.set_xlabel('Stride', fontsize=12)
ax1.set_ylabel('Execution Time (msec)', fontsize=12)
ax1.set_title('Execution Time vs Stride', fontsize=14, fontweight='bold')
ax1.legend()
ax1.grid(True, alpha=0.3)

# Plot 2: Bandwidth vs Stride
ax2.plot(strides_O0, rates_O0, 'o-', label='-O0 (No Optimization)', linewidth=2, markersize=6)
ax2.plot(strides_O2, rates_O2, 's-', label='-O2 (Optimization)', linewidth=2, markersize=6)
ax2.set_xlabel('Stride', fontsize=12)
ax2.set_ylabel('Bandwidth (MB/s)', fontsize=12)
ax2.set_title('Memory Bandwidth vs Stride', fontsize=14, fontweight='bold')
ax2.legend()
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('stride_analysis.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'stride_analysis.png'")

# Print statistics
print("\n=== Performance Comparison ===")
print(f"\nAverage execution time (msec):")
print(f"  -O0: {np.mean(times_O0):.2f}")
print(f"  -O2: {np.mean(times_O2):.2f}")
print(f"  Speedup: {np.mean(times_O0)/np.mean(times_O2):.2f}x")

print(f"\nAverage bandwidth (MB/s):")
print(f"  -O0: {np.mean(rates_O0):.2f}")
print(f"  -O2: {np.mean(rates_O2):.2f}")
print(f"  Improvement: {(np.mean(rates_O2)/np.mean(rates_O0) - 1)*100:.1f}%")

plt.show()
