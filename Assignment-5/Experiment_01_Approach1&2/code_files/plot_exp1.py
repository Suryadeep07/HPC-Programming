import pandas as pd
import matplotlib.pyplot as plt

# Load the data
df = pd.read_csv("results_exp1.csv")

# Create a plot for each grid size
grids = df['Grid'].unique()

for grid in grids:
    subset = df[df['Grid'] == grid]
    
    plt.figure(figsize=(8, 6))
    
    # Plot Total Time (Interpolation + Mover)
    plt.plot(subset['Points'], subset['Imm_Total'], marker='o', label='Immediate Replacement', color='blue')
    plt.plot(subset['Points'], subset['Def_Total'], marker='s', label='Deferred Insertion', color='red')
    
    plt.xscale('log')
    plt.yscale('log')
    
    plt.title(f'Execution Time vs Particles (Grid: {grid})', fontsize=14)
    plt.xlabel('Number of Particles (Log Scale)', fontsize=12)
    plt.ylabel('Total Execution Time in Seconds (Log Scale)', fontsize=12)
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.legend()
    
    filename = f'exp1_scaling_{grid}.png'
    plt.savefig(filename, dpi=300)
    print(f"Saved {filename}")
