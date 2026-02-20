import matplotlib.pyplot as plt
import numpy as np

# Benchmark data from actual Pi calculation runs
# Will be updated with full results
data = {
    10000000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.987920, 1.893488, 1.997185, 3.343299],
        'efficiency': [0.987920, 0.946744, 0.499296, 0.417912]
    },
    100000000: {
        'processes': [1, 2, 4, 8],
        'speedup': [0.996649, 1.245491, 2.569465, 2.847605],
        'efficiency': [0.996649, 0.622745, 0.642366, 0.355951]
    },
    # Add 1 billion results after benchmark completes
}

# Create figure with 2 subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Colors and markers for different N values
colors = ['#1f77b4', '#ff7f0e', '#2ca02c']
markers = ['o', 's', '^']
N_values = [10000000, 100000000]

# Plot 1: Speedup
for i, N in enumerate(N_values):
    ax1.plot(data[N]['processes'], data[N]['speedup'], 
             marker=markers[i], color=colors[i], linewidth=2, 
             markersize=8, label=f'N={N:,}')

# Ideal speedup line
processes = [1, 2, 4, 8]
ideal_speedup = [p for p in processes]
ax1.plot(processes, ideal_speedup, '--', color='gray', 
         linewidth=2, label='Ideal Speedup', alpha=0.7)

ax1.set_xlabel('Number of Processes', fontsize=12, fontweight='bold')
ax1.set_ylabel('Speedup', fontsize=12, fontweight='bold')
ax1.set_title('Pi Calculation: Speedup vs Number of Processes', fontsize=14, fontweight='bold')
ax1.legend(loc='best', fontsize=10)
ax1.grid(True, alpha=0.3)
ax1.set_xticks(processes)
ax1.set_ylim(bottom=0)

# Plot 2: Efficiency
for i, N in enumerate(N_values):
    ax2.plot(data[N]['processes'], 
             [e * 100 for e in data[N]['efficiency']], 
             marker=markers[i], color=colors[i], linewidth=2, 
             markersize=8, label=f'N={N:,}')

# Ideal efficiency line (100%)
ax2.axhline(y=100, linestyle='--', color='gray', 
            linewidth=2, label='Ideal (100%)', alpha=0.7)

ax2.set_xlabel('Number of Processes', fontsize=12, fontweight='bold')
ax2.set_ylabel('Efficiency (%)', fontsize=12, fontweight='bold')
ax2.set_title('Pi Calculation: Efficiency vs Number of Processes', fontsize=14, fontweight='bold')
ax2.legend(loc='best', fontsize=10)
ax2.grid(True, alpha=0.3)
ax2.set_xticks(processes)
ax2.set_ylim(bottom=0, top=110)

plt.tight_layout()
plt.savefig('pi_speedup_efficiency.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'pi_speedup_efficiency.png'")

# Print summary statistics
print("\n=== Pi Calculation Performance Summary ===")
for N in N_values:
    print(f"\nN = {N:,}")
    best_speedup_idx = np.argmax(data[N]['speedup'])
    best_efficiency_idx = np.argmax(data[N]['efficiency'])
    print(f"  Best Speedup: {data[N]['speedup'][best_speedup_idx]:.4f}x with {data[N]['processes'][best_speedup_idx]} processes")
    print(f"  Best Efficiency: {data[N]['efficiency'][best_efficiency_idx]*100:.2f}% with {data[N]['processes'][best_efficiency_idx]} process(es)")

plt.show()
