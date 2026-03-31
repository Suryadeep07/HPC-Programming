#!/bin/bash
echo "Compiling Experiment 2 & Verification..."
g++ -O3 -fopenmp init.cpp utils.cpp main_exp2.cpp -o exp2
g++ -O3 -fopenmp init.cpp utils.cpp verify_dist.cpp -o verify
echo "Running Verification..."
./verify
python plot_verification.py
echo "Running Experiment 2..."
./exp2
echo "Generating Exp 2 Plots..."
python plot_exp2.py
echo "Experiment 2 Pipeline Complete!"
