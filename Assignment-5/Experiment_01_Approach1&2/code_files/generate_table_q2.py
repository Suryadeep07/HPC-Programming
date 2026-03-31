grids = [(250, 100), (500, 200), (1000, 400)]
particles = [100, 10000, 1000000, 100000000, 1000000000] 

# 12th Gen i5-12500 L3 Cache (Source 152)
L3_CACHE_MB = 18.0 
# Machine Balance: Peak GFLOPS / Peak BW (Approx for i5-12500)
MACHINE_BALANCE = 8.62 

with open("q2_memory_table.tex", "w") as f:
    for nx, ny in grids:
        for p in particles:
            mem_particles = p * 16
            mem_grid = (nx + 1) * (ny + 1) * 8
            total_mem_mb = (mem_particles + mem_grid) / (1024 * 1024)
            
            flops = p * 30 * 10
            arithmetic_intensity = 0.62
            
            if total_mem_mb <= L3_CACHE_MB:
                bound = "Compute-Bound (In Cache)"
            else:
                bound = "Memory-Bound (DRAM Spill)"
            
            f.write(f"${nx}\\times{ny}$ & $10^{len(str(p))-1}$ & {total_mem_mb:.2f} & {flops:.2e} & {arithmetic_intensity:.2f} & {MACHINE_BALANCE:.2f} & {bound} \\\\\n")