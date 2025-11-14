#!/bin/bash

# Repeatedly runs the main program to test the pipe implementation.
# If incorrect output is detected (or it run n times), this will stop.
for i in {1..10000}
do
  # Τρέξιμο της εντολής ./test test.txt
  ./main test.txt > test_output.txt

  # Διαφορές μεταξύ test.txt και first_output.txt
  diff test.txt first_output.txt > diff1.txt

  # Διαφορές μεταξύ first_output.txt και second_output.txt
  diff first_output.txt second_output.txt > diff2.txt

  if [ -s diff1.txt ] || [ -s diff2.txt ]; then
    echo "Differences found, stopping the script."
    echo "Iter is $i"
    break
  fi
done
