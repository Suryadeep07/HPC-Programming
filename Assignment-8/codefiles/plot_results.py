import pandas as pd
import matplotlib.pyplot as plt

try:
    df = pd.read_csv("interpolation_results.csv")
except FileNotFoundError:
    print("Error: interpolation_results.csv not found.")
    exit()

plot_cores = [1, 2, 4, 8, 16, 32, 64]

# 1. Execution Time vs Cores (Total Time)
plt.figure(figsize=(10, 6))
for config in df['Configuration'].unique():
    subset = df[df['Configuration'] == config]
    plt.plot(subset['Total_Cores'], subset['Total_Time'], marker='o', label=f'Config {config}')

plt.title('Total Execution Time vs Cores')
plt.xlabel('Number of Cores')
plt.ylabel('Execution Time (Seconds)')
plt.xscale('log', basex=2) # Reverted back to basex
plt.xticks(plot_cores, labels=plot_cores)
plt.grid(True, which="both", ls="--")
plt.legend()
plt.savefig('exec_time_vs_cores.png')
print("Generated exec_time_vs_cores.png")
plt.close()


# 2A. Speedup vs Cores (NO IDEAL LINE)
plt.figure(figsize=(10, 6))
for config in df['Configuration'].unique():
    subset = df[df['Configuration'] == config]
    base_time = subset[subset['Total_Cores'] == 1]['Total_Time'].values[0]
    
    speedup = base_time / subset['Total_Time']
    
    plt.plot(subset['Total_Cores'], speedup, marker='o', label=f'Config {config}')

plt.title('Parallel Speedup vs Cores (Actual Experiments)')
plt.xlabel('Number of Cores')
plt.ylabel('Speedup Factor')
plt.xscale('log', basex=2)
plt.xticks(plot_cores, labels=plot_cores)
plt.grid(True, which="both", ls="--")
plt.legend()
plt.savefig('speedup_no_ideal.png')
print("Generated speedup_no_ideal.png")
plt.close()

# 2B. Speedup vs Cores (WITH IDEAL LINE, Log-Log Scale)
plt.figure(figsize=(10, 6))
for config in df['Configuration'].unique():
    subset = df[df['Configuration'] == config]
    base_time = subset[subset['Total_Cores'] == 1]['Total_Time'].values[0]
    
    speedup = base_time / subset['Total_Time']
    
    plt.plot(subset['Total_Cores'], speedup, marker='o', label=f'Config {config}')

# Plot Ideal Speedup line starting from 1
plt.plot(plot_cores, plot_cores, 'k--', label='Ideal Speedup')

plt.title('Parallel Speedup vs Cores (Log-Log Scale)')
plt.xlabel('Number of Cores')
plt.ylabel('Speedup Factor')
plt.xscale('log', basex=2)
plt.yscale('log', basey=2) # Reverted back to basey
plt.xticks(plot_cores, labels=plot_cores)
plt.grid(True, which="both", ls="--")
plt.legend()
plt.savefig('speedup_with_ideal.png')
print("Generated speedup_with_ideal.png")
plt.close()

# 3. Phase Analysis (Bottleneck Identification)
config_to_analyze = 'E' 
if config_to_analyze in df['Configuration'].unique():
    plt.figure(figsize=(10, 6))
    subset = df[df['Configuration'] == config_to_analyze]
    
    plt.plot(subset['Total_Cores'], subset['Interp_Time'], marker='o', linewidth=2, label='Interpolation Phase')
    plt.plot(subset['Total_Cores'], subset['Mover_Time'], marker='s', linewidth=2, label='Mover Phase')
    plt.plot(subset['Total_Cores'], subset['Overhead_Time'], marker='^', linewidth=2, linestyle=':', label='Overhead (MPI+Norm)')

    plt.title(f'Phase Bottleneck Analysis (Config {config_to_analyze})')
    plt.xlabel('Number of Cores')
    plt.ylabel('Execution Time (Seconds)')
    plt.xscale('log', basex=2)
    plt.yscale('log', basey=10) # Reverted back to basey
    plt.xticks(plot_cores, labels=plot_cores)
    plt.grid(True, which="both", ls="--")
    plt.legend()
    plt.savefig('phase_bottleneck_analysis.png')
    print("Generated phase_bottleneck_analysis.png")
    plt.close()

print("All plots generated successfully!")