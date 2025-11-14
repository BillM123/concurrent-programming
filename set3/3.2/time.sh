#!/bin/bash

# 1st implementation
program1="./previous/main"

# 2nd implementation
program2="./previous2/main"

# 3rd implementation
program3="./main"

# Output file
output_file="testing/timings.txt"

#Cores of your cpu
num_cores=32

# Clear the output file
> $output_file

#make
# cd ./../set2/2.2/previous
make
cd ./previous
make
cd ./../previous2
make
cd ./../

# Run the program 6 times with increasing cpu cores
for i in {1..10} 
do

    ((arg = i * (num_cores / 8)))

    echo "NOW RUNNING USING $arg CORES" >> $output_file

    echo "Running with small primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-small.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-small.txt"; } 2>> $output_file
    
    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/primes-small.txt"; } 2>> $output_file

    printf "."


    echo "Running with medium primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-medium.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-medium.txt"; } 2>> $output_file

    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/primes-medium.txt"; } 2>> $output_file

    printf "."


    echo "Running with large primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-large.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-large.txt"; } 2>> $output_file

    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/primes-large.txt"; } 2>> $output_file

    printf "."


    echo "Running with mix-sized primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/primes-mix.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/primes-mix.txt"; } 2>> $output_file

    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/primes-mix.txt"; } 2>> $output_file

    printf "."


    echo "Running with medium-sized only primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/only-primes-medium.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/only-primes-medium.txt"; } 2>> $output_file

    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/only-primes-medium.txt"; } 2>> $output_file

    printf "."


    echo "Running with large-sized only primes" >> $output_file

    echo "  P1:" >> $output_file
    { time $program1 $arg < "testing/only-primes-large.txt"; } 2>> $output_file

    echo "  P2:" >> $output_file
    { time $program2 $arg < "testing/only-primes-large.txt"; } 2>> $output_file

    echo "  P3:" >> $output_file
    { time $program3 $arg < "testing/only-primes-large.txt"; } 2>> $output_file

    echo "||" >> $output_file
    echo "||" >> $output_file

    printf ".\n"
done