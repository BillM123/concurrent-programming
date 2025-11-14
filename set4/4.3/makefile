CC = gcc
CFLAGS = -Wall -g

test: test.c mythreads.o queue.o include/mythreads.h include/queue.h
	$(CC) -Wall -g test.c mythreads.o queue.o -o test -fsanitize=address

main: main.c mythreads.o queue.o include/mythreads.h include/queue.h
	$(CC) -Wall -g main.c mythreads.o queue.o -o main -fsanitize=address

mythreads.o: mythreads.c include/mythreads.h include/queue.h
	$(CC) -Wall -g -c -o mythreads.o mythreads.c -fsanitize=address


queue.o: queue.c include/queue.h
	$(CC) -Wall -g -c -o queue.o queue.c -fsanitize=address

clean:
	rm -r mythreads.o queue.o test main
