#!/bin/bash
echo "Compiling Experiment 3..."
g++ main_exp3.cpp init.cpp utils.cpp -lm -fopenmp -O3 -o exp3_run
echo "Running Experiment 3... Saving to exp3_results.csv"
./exp3_run > exp3_results.csv
echo "Done!"
