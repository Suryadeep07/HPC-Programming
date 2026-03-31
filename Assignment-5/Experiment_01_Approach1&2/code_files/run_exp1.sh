#!/bin/bash
echo "Compiling Experiment 1..."
g++ -O3 -fopenmp init.cpp utils.cpp main_exp1.cpp -o exp1
echo "Running Experiment 1..."
./exp1
echo "Generating Exp 1 Plots..."
python plot_exp1.py
python plot_q4_ppc.py
python generate_table_q2.py
echo "Experiment 1 Pipeline Complete!"
