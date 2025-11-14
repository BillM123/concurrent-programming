#!/bin/bash

# Repeatedly runs the main program to test if sorting is correct.
# If incorrect output is detected (or it run n times), this will stop.
for i in {1..10000}
do
    make data.bin
    ./main data.bin > debug-output.txt
    python sort-full.py
    diff filePostSorted.txt fully_sorted_output.txt > diff.txt

    if [ -s diff.txt ]; then
        echo "Differences found, stopping the script."
        echo "Iter is $i"
        echo "Program output < > Correct output" 
        break
    fi
done
