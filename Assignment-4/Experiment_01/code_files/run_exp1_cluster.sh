#!/bin/bash
echo "Compiling Experiment 1..."
g++ main_exp1.cpp init.cpp utils.cpp -lm -fopenmp -O3 -o exp1_run

echo "Running Experiment 1 in the background..."
echo "Results will be saved to exp1_results.csv"
echo "Progress updates will be saved to exp1_progress.log"

# Run with nohup in the background, redirecting stdout to CSV and stderr to a log file
nohup ./exp1_run > exp1_results.csv 2> exp1_progress.log &

echo "Job started successfully! You can now close your laptop."
