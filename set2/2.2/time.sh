#!/bin/bash

# 1st implementation
program1="./previous/main"

# 2nd implementation
program2="./main"

# Output file
output_file="testing/timings.txt"

#Cores of your cpu
num_cores=8

# Clear the output file
> $output_file

#make
make
cd ./previous
make
cd ..

# Run the program 6 times with increasing cpu cores
for i in {1..5} 
do

    ((arg = i * (num_cores / 4)))

    echo "NOW RUNNING USING $arg CORES" >> $output_file

    echo "Running with small primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-small.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-small.txt"; } 2>> $output_file


    echo "Running with medium primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-medium.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-medium.txt"; } 2>> $output_file


    echo "Running with large primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-large.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-large.txt"; } 2>> $output_file


    echo "Running with mix-sized primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-mix.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-mix.txt"; } 2>> $output_file

done
