import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results_exp1.csv")

# Extract NX and NY from the Grid string (e.g., "250x100")
df[['NX', 'NY']] = df['Grid'].str.split('x', expand=True).astype(int)

# Calculate Particles Per Cell (PPC)
df['PPC'] = df['Points'] / (df['NX'] * df['NY'])

# Calculate Per-Particle Execution Time (in nanoseconds for readability)
# Time is divided by 10 (Maxiter) and then by total points
df['Time_Per_Particle_ns'] = (df['Imm_Total'] / 10 / df['Points']) * 1e9

plt.figure(figsize=(8, 6))
plt.scatter(df['PPC'], df['Time_Per_Particle_ns'], color='purple', alpha=0.7, edgecolors='k')

plt.xscale('log')
plt.title('Per-Particle Execution Time vs. Particles Per Cell (PPC)', fontsize=14)
plt.xlabel('Particles Per Cell (PPC) [Log Scale]', fontsize=12)
plt.ylabel('Execution Time per Particle (nanoseconds)', fontsize=12)
plt.grid(True, linestyle='--', alpha=0.5)

plt.savefig('q4_ppc_plot.png', dpi=300)
print("Saved q4_ppc_plot.png")
