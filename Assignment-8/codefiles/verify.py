import sys

def compare_meshes(file1, file2, tolerance=1e-5):
    try:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            vals1 = [float(x) for line in f1 for x in line.split()]
            vals2 = [float(x) for line in f2 for x in line.split()]
    except FileNotFoundError as e:
        print(f"Error: {e}")
        return

    if len(vals1) != len(vals2):
        print(f"FAILED: Mesh sizes differ ({len(vals1)} vs {len(vals2)})")
        return

    max_diff = 0.0
    for i, (v1, v2) in enumerate(zip(vals1, vals2)):
        diff = abs(v1 - v2)
        if diff > max_diff:
            max_diff = diff
        if diff > tolerance:
            print(f"FAILED at index {i}: Values exceed tolerance! Diff: {diff:.8f} (Output: {v1}, Expected: {v2})")
            return

    print(f"SUCCESS: Meshes match within tolerance! Maximum difference was: {max_diff:.8f}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python verify.py <Generated_Mesh.out> <Test_Mesh.out>")
        sys.exit(1)
    compare_meshes(sys.argv[1], sys.argv[2])
