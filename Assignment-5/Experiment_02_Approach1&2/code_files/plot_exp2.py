import pandas as pd
import matplotlib.pyplot as plt

# Load the data
df = pd.read_csv("results_exp2.csv")
grids = df['Grid'].unique()

for grid in grids:
    subset = df[df['Grid'] == grid]
    
    base_imm = subset['Imm_Mover'].iloc[0]
    base_def = subset['Def_Mover'].iloc[0]
    base_no_insert = subset['Base_Mover'].iloc[0]
    
    threads = subset['Threads']
    
    # Speedup = T(base) / T(threads)
    speedup_imm = base_imm / subset['Imm_Mover']
    speedup_def = base_def / subset['Def_Mover']
    speedup_no_insert = base_no_insert / subset['Base_Mover']
    
    plt.figure(figsize=(8, 6))
    plt.plot(threads, speedup_imm, marker='o', label='Parallel Mover (Immediate)')
    plt.plot(threads, speedup_def, marker='s', label='Parallel Mover (Deferred)')
    plt.plot(threads, speedup_no_insert, marker='^', label='Parallel Mover (Baseline No Insert)')
    
    # Ideal Speedup Line (Normalized to the starting thread count)
    plt.plot(threads, threads / threads.iloc[0], 'k--', label='Ideal Speedup')
    
    plt.title(f'Mover Speedup vs Threads (Grid: {grid}, Particles: 14M)', fontsize=14)
    plt.xlabel('Number of Threads', fontsize=12)
    plt.ylabel('Speedup Factor', fontsize=12)
    
    plt.xticks(threads) 
    
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend()
    
    filename = f'exp2_speedup_{grid}.png'
    plt.savefig(filename, dpi=300)
    print(f"Saved {filename}")
