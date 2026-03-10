#!/bin/bash
echo "Compiling Experiment 2..."
g++ main_exp2.cpp init.cpp utils.cpp -lm -fopenmp -O3 -o exp2_run
echo "Running Experiment 2... Saving to exp2_results.csv"
./exp2_run > exp2_results.csv
echo "Done!"
