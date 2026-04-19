#!/bin/bash
if [ ! -f results.csv ]; then echo "Config,Mode,Threads,Time" > results.csv; fi

CONFIG="input_E.bin"
echo "========================================"
echo " Processing Configuration E             "
echo "========================================"

./maker $CONFIG 1000 400 14000000 10
THREADS=(2 4 6 8 10 12 14 16)

for t in "${THREADS[@]}"; do
    export OMP_NUM_THREADS=$t
    T1=$(./pic_parallel $CONFIG); T2=$(./pic_parallel $CONFIG); T3=$(./pic_parallel $CONFIG)
    AVG=$(echo "scale=6; ($T1 + $T2 + $T3) / 3" | bc)
    echo "Parallel | $CONFIG | Threads: $t | Time: $AVG s"
    echo "$CONFIG,Parallel,$t,$AVG" >> results.csv
done

T1=$(./pic_serial $CONFIG); T2=$(./pic_serial $CONFIG); T3=$(./pic_serial $CONFIG)
AVG=$(echo "scale=6; ($T1 + $T2 + $T3) / 3" | bc)
echo "Serial   | $CONFIG | Threads: 1 | Time: $AVG s"
echo "$CONFIG,Serial,1,$AVG" >> results.csv

rm $CONFIG
echo "Deleted $CONFIG to save disk space."
