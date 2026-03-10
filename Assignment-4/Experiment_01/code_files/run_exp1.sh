#!/bin/bash
echo "Compiling Experiment 1..."
g++ main_exp1.cpp init.cpp utils.cpp -lm -fopenmp -O3 -o exp1_run
echo "Running Experiment 1... Saving to exp1_results.csv"
./exp1_run > exp1_results.csv
echo "Done!"
