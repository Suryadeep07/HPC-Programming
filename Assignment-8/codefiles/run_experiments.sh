#!/bin/bash

MPICXX="/usr/mpi/gcc/openmpi-1.8.8/bin/mpic++"
MPIRUN="/usr/mpi/gcc/openmpi-1.8.8/bin/mpirun"

echo "Compiling the suite..."
g++ input_file_maker.cpp -o input_maker
g++ main_serial.cpp init.cpp utils_serial.cpp -o hpc_interp_serial -O3
$MPICXX main_mpi.cpp init.cpp utils_mpi.cpp -o hpc_interp_mpi -O3 -fopenmp

if [ $? -ne 0 ]; then
    echo "Compilation failed! Please check for errors."
    exit 1
fi
echo "Compilation successful."

CSV_FILE="interpolation_results.csv"
echo "Configuration,Points_Millions,Total_Cores,Interp_Time,Mover_Time,Overhead_Time,Total_Time" > $CSV_FILE

# Added '1' for the Serial Baseline!
CORES_LIST=(1 2 4 8 16 32 64)

run_exp() {
    local config=$1
    local nx=$2
    local ny=$3
    local pts=$4
    local pts_label=$5

    echo "=================================================="
    echo "Generating Input for Config $config (Points=$pts_label million)..."
    echo "$nx $ny $pts 10" | ./input_maker > /dev/null

    for total_cores in "${CORES_LIST[@]}"; do
        echo "Running Config $config with $total_cores total cores..."
        
        if [ "$total_cores" -eq 1 ]; then
            # Run the pure serial baseline
            output=$(./hpc_interp_serial input.bin)
        else
            # Run the hybrid MPI+OpenMP version
            NUM_NODES=4
            if [ "$total_cores" -lt 4 ]; then
                NUM_NODES=$total_cores
                THREADS_PER_RANK=1
            else
                THREADS_PER_RANK=$((total_cores / NUM_NODES))
            fi
            output=$($MPIRUN -np $NUM_NODES --hostfile sources.txt --map-by node -x OMP_NUM_THREADS=$THREADS_PER_RANK ./hpc_interp_mpi input.bin)
        fi
        
        interp_val=$(echo "$output" | grep "Interpolation_Time =" | awk '{print $3}')
        overhead_val=$(echo "$output" | grep "Overhead_Time =" | awk '{print $3}')
        mover_val=$(echo "$output" | grep "Mover_Time =" | awk '{print $3}')
        total_val=$(echo "$output" | grep "Total_Time =" | awk '{print $3}')

        interp_val=${interp_val:-0.0}
        overhead_val=${overhead_val:-0.0}
        mover_val=${mover_val:-0.0}
        total_val=${total_val:-0.0}

        echo "Total: $total_val sec | Interp: $interp_val sec | Mover: $mover_val sec"
        echo "$config,$pts_label,$total_cores,$interp_val,$mover_val,$overhead_val,$total_val" >> $CSV_FILE
    done
    
    rm input.bin
}

run_exp "A" 250 100 900000 "0.9"
run_exp "B" 250 100 5000000 "5.0"
run_exp "C" 500 200 3600000 "3.6"
run_exp "D" 500 200 20000000 "20.0"
run_exp "E" 1000 400 14000000 "14.0"

echo "=================================================="
echo "All experiments completed! Data saved to $CSV_FILE"
