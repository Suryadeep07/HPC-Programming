import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("particles_verification.csv")
x = df['X']
y = df['Y']

# Create a large figure with 3 subplots
fig = plt.figure(figsize=(15, 5))

# Plot 1: 2D Scatter Plot (Shows empty regions or boundaries)
ax1 = fig.add_subplot(131)
ax1.scatter(x, y, s=0.1, alpha=0.5, color='blue')
ax1.set_title("Particle Position Scatter Plot")
ax1.set_xlabel("X Coordinate")
ax1.set_ylabel("Y Coordinate")
ax1.set_xlim(0, 1)
ax1.set_ylim(0, 1)

# Plot 2: 2D Histogram / Heatmap (Shows clustering/bias)
ax2 = fig.add_subplot(132)
h = ax2.hist2d(x, y, bins=(50, 50), cmap='viridis')
fig.colorbar(h[3], ax=ax2, label='Particle Count per Cell')
ax2.set_title("Cell-wise Particle Count (Heatmap)")
ax2.set_xlabel("X Coordinate")
ax2.set_ylabel("Y Coordinate")

# Plot 3: 1D Histograms for X and Y (Shows uniform distribution)
ax3 = fig.add_subplot(133)
ax3.hist(x, bins=50, alpha=0.5, label='X coordinates', color='red', density=True)
ax3.hist(y, bins=50, alpha=0.5, label='Y coordinates', color='green', density=True)
ax3.set_title("1D Coordinate Histograms")
ax3.set_xlabel("Coordinate Value")
ax3.set_ylabel("Density")
ax3.legend()

plt.tight_layout()
plt.savefig("verification_plots.png", dpi=300)
print("Saved verification_plots.png")
