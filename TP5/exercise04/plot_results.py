import matplotlib.pyplot as plt
import numpy as np

# Benchmark data from actual runs (larger matrices)
data = {
    1000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.464350, 0.704433, 0.726822, 0.646392],
        'efficiency': [0.464350, 0.352217, 0.181706, 0.080799]
    },
    2000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.525832, 0.640268, 0.699118, 0.803072],
        'efficiency': [0.525832, 0.320134, 0.174780, 0.100384]
    },
    4000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.545238, 0.692677, 0.812640, 0.827604],
        'efficiency': [0.545238, 0.346338, 0.203160, 0.103451]
    },
    8000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.449719, 0.563626, 0.781299, 0.945397],
        'efficiency': [0.449719, 0.281813, 0.195325, 0.118175]
    }
}

# Create figure with 2 subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Colors and markers for different matrix sizes
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
markers = ['o', 's', '^', 'D']
sizes = [1000, 2000, 4000, 8000]

# Plot 1: Speedup
for i, N in enumerate(sizes):
    ax1.plot(data[N]['processes'], data[N]['speedup'], 
             marker=markers[i], color=colors[i], linewidth=2, 
             markersize=8, label=f'N={N}')

# Ideal speedup line (for reference)
processes = [1, 2, 4, 8]
ideal_speedup = [p for p in processes]
ax1.plot(processes, ideal_speedup, '--', color='gray', 
         linewidth=1.5, label='Ideal Speedup', alpha=0.7)

ax1.set_xlabel('Number of Processes', fontsize=12, fontweight='bold')
ax1.set_ylabel('Speedup', fontsize=12, fontweight='bold')
ax1.set_title('Speedup vs Number of Processes', fontsize=14, fontweight='bold')
ax1.legend(loc='best', fontsize=10)
ax1.grid(True, alpha=0.3)
ax1.set_xticks(processes)

# Plot 2: Efficiency
for i, N in enumerate(sizes):
    ax2.plot(data[N]['processes'], data[N]['efficiency'], 
             marker=markers[i], color=colors[i], linewidth=2, 
             markersize=8, label=f'N={N}')

# Ideal efficiency line (100%)
ax2.axhline(y=1.0, linestyle='--', color='gray', 
            linewidth=1.5, label='Ideal Efficiency', alpha=0.7)

ax2.set_xlabel('Number of Processes', fontsize=12, fontweight='bold')
ax2.set_ylabel('Efficiency', fontsize=12, fontweight='bold')
ax2.set_title('Efficiency vs Number of Processes', fontsize=14, fontweight='bold')
ax2.legend(loc='best', fontsize=10)
ax2.grid(True, alpha=0.3)
ax2.set_xticks(processes)

plt.tight_layout()
plt.savefig('speedup_efficiency_plot.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'speedup_efficiency_plot.png'")

# Print summary statistics
print("\n=== Summary Statistics ===")
for N in sizes:
    print(f"\nMatrix Size: {N}x{N}")
    best_idx = np.argmax(data[N]['speedup'])
    print(f"  Best Speedup: {data[N]['speedup'][best_idx]:.4f} with {data[N]['processes'][best_idx]} processes")
    print(f"  Best Efficiency: {max(data[N]['efficiency']):.4f}")
    
plt.show()
