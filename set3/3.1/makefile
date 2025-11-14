CC = gcc
CFLAGS = -Wall -g -fsanitize=address# -fsanitize=bounds

main: 2.4main.c mysem.o mysem.h
	$(CC) $(CFLAGS) -pthread 2.4main.c mysem.o -o main

mysem.o: mysem.c mysem.h
	$(CC) $(CFLAGS) -c -o mysem.o mysem.c -pthread

clean:
	rm -r mysem.o main
