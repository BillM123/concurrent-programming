CC = gcc
CFLAGS = -Wall -g

main: main.c mythreads.o queue.o include/mythreads.h include/queue.h
	$(CC) -Wall -g main.c mythreads.o queue.o -o main

mythreads.o: mythreads.c include/mythreads.h include/queue.h
	$(CC) -Wall -g -c -o mythreads.o mythreads.c


queue.o: queue.c include/queue.h
	$(CC) -Wall -g -c -o queue.o queue.c

clean:
	rm -r mythreads.o queue.o test main
