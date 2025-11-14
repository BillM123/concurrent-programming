CC = gcc
CFLAGS = -Wall -g

test: test.c mysem.c mysem.h
	$(CC) $(CFLAGS) -pthread test.c mysem.c -o test

mysem.o: mysem.c mysem.h
	$(CC) $(CFLAGS) -c -o mysem.o mysem.c

clean:
	rm -r mysem.o test
