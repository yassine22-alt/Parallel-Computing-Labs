import matplotlib.pyplot as plt
import numpy as np

# Parse the results file
block_sizes = []
times = []
bandwidths = []
speedups = []

# Try different encodings
try:
    with open('mxm_bloc_results.txt', 'r', encoding='utf-8') as f:
        lines = f.readlines()
except UnicodeDecodeError:
    try:
        with open('mxm_bloc_results.txt', 'r', encoding='utf-16') as f:
            lines = f.readlines()
    except:
        with open('mxm_bloc_results.txt', 'r') as f:
            lines = f.readlines()

if True:  # Keep the indentation for the rest of the code
    pass
else:
    lines = []
    
if lines:
    
    # Skip header lines (first 4 lines)
    for line in lines[4:]:
        line = line.strip()
        if not line:
            continue
        
        parts = line.split(',')
        if len(parts) >= 4:
            try:
                # Handle both block sizes and "Standard (no blocking)"
                if 'Standard' in parts[0]:
                    block_sizes.append(512)  # Use 512 to represent no blocking
                else:
                    block_sizes.append(int(parts[0].strip()))
                
                times.append(float(parts[1].strip()))
                bandwidths.append(float(parts[2].strip()))
                speedup_str = parts[3].strip().replace('x', '')
                speedups.append(float(speedup_str))
            except ValueError:
                continue

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Plot 1: Execution Time vs Block Size
ax1.plot(block_sizes, times, 'o-', linewidth=2, markersize=8, color='#e74c3c', label='Execution Time')
ax1.set_xlabel('Block Size (512 = No Blocking)', fontsize=13, fontweight='bold')
ax1.set_ylabel('Execution Time (msec)', fontsize=13, fontweight='bold')
ax1.set_title('Execution Time vs Block Size', fontsize=15, fontweight='bold')
ax1.grid(True, alpha=0.3, linestyle='--')
ax1.set_xscale('log', base=2)  # Log scale since block sizes are powers of 2
ax1.set_xticks(block_sizes)
ax1.set_xticklabels([str(bs) if bs != 512 else 'No Block' for bs in block_sizes], rotation=45)

# Mark the optimal (minimum time)
min_idx = times.index(min(times))
label_text = f'Optimal: {block_sizes[min_idx]}'
if block_sizes[min_idx] == 512:
    label_text = 'Optimal: No Blocking'
ax1.plot(block_sizes[min_idx], times[min_idx], 'g*', markersize=20, 
         label=f'{label_text} ({times[min_idx]:.2f} ms)')
ax1.legend(fontsize=11)

# Plot 2: Bandwidth vs Block Size
ax2.plot(block_sizes, bandwidths, 's-', linewidth=2, markersize=8, color='#3498db', label='Bandwidth')
ax2.set_xlabel('Block Size (512 = No Blocking)', fontsize=13, fontweight='bold')
ax2.set_ylabel('Bandwidth (MB/s)', fontsize=13, fontweight='bold')
ax2.set_title('Memory Bandwidth vs Block Size', fontsize=15, fontweight='bold')
ax2.grid(True, alpha=0.3, linestyle='--')
ax2.set_xscale('log', base=2)
ax2.set_xticks(block_sizes)
ax2.set_xticklabels([str(bs) if bs != 512 else 'No Block' for bs in block_sizes], rotation=45)

# Mark the optimal (maximum bandwidth)
max_idx = bandwidths.index(max(bandwidths))
label_text = f'Optimal: {block_sizes[max_idx]}'
if block_sizes[max_idx] == 512:
    label_text = 'Optimal: No Blocking'
ax2.plot(block_sizes[max_idx], bandwidths[max_idx], 'g*', markersize=20,
         label=f'{label_text} ({bandwidths[max_idx]:.2f} MB/s)')
ax2.legend(fontsize=11)

plt.tight_layout()
plt.savefig('block_size_analysis.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'block_size_analysis.png'")

# Print analysis
print("\n" + "="*60)
print("BLOCK SIZE ANALYSIS")
print("="*60)

print(f"\n📊 Data Summary:")
print(f"   Block sizes tested: {block_sizes}")
print(f"\n⏱️  Execution Times (msec):")
for bs, t in zip(block_sizes, times):
    label = f"Block size {bs:3d}" if bs != 512 else "No Blocking   "
    print(f"   {label}: {t:7.2f} ms")

print(f"\n💾 Memory Bandwidth (MB/s):")
for bs, bw in zip(block_sizes, bandwidths):
    label = f"Block size {bs:3d}" if bs != 512 else "No Blocking   "
    print(f"   {label}: {bw:10.2f} MB/s")

print(f"\n⚡ Speedup Factors:")
for bs, sp in zip(block_sizes, speedups):
    label = f"Block size {bs:3d}" if bs != 512 else "No Blocking   "
    print(f"   {label}: {sp:5.2f}x")

print(f"\n🏆 OPTIMAL BLOCK SIZE:")
opt_label = f"Block size {block_sizes[min_idx]}" if block_sizes[min_idx] != 512 else "No Blocking"
print(f"   Fastest execution: {opt_label} ({times[min_idx]:.2f} ms)")
opt_label = f"Block size {block_sizes[max_idx]}" if block_sizes[max_idx] != 512 else "No Blocking"
print(f"   Highest bandwidth: {opt_label} ({bandwidths[max_idx]:.2f} MB/s)")

if min_idx == max_idx:
    opt_label = f"block size {block_sizes[min_idx]}" if block_sizes[min_idx] != 512 else "no blocking"
    print(f"\n✓ {opt_label.capitalize()} is optimal for both metrics!")
else:
    print(f"\n⚠ Different optimal sizes for time vs bandwidth")

print("\n" + "="*60)

# Cache analysis
print("\n📚 WHY IS THIS THE OPTIMAL SIZE?")
print("="*60)
optimal_bs = block_sizes[max_idx]
block_memory = 3 * optimal_bs * optimal_bs * 8  # 3 blocks (A, B, C), 8 bytes per double

print(f"\nBlock size {optimal_bs}:")
print(f"  • Working set = 3 blocks × {optimal_bs}×{optimal_bs} × 8 bytes")
print(f"  • Total memory = {block_memory:,} bytes = {block_memory/1024:.1f} KB")

if block_memory <= 32 * 1024:
    print(f"  ✓ Fits in L1 cache (~32 KB)")
elif block_memory <= 256 * 1024:
    print(f"  ✓ Fits in L2 cache (~256 KB)")
else:
    print(f"  ⚠ Larger than L2 cache, uses L3 or RAM")

print(f"\nComparison:")
for bs in block_sizes:
    mem = 3 * bs * bs * 8
    cache_level = "L1" if mem <= 32*1024 else ("L2" if mem <= 256*1024 else "L3/RAM")
    print(f"  Block {bs:3d}: {mem:8,} bytes ({mem/1024:6.1f} KB) → {cache_level}")

plt.show()
