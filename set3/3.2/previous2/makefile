CC = gcc
CFLAGS = -Wall -g

main: main.c ../2.1/mysem.o
	$(CC) $(CFLAGS) main.c ../2.1/mysem.o -lpthread -o main

../2.1/mysem.o: ../2.1/mysem.c ../2.1/mysem.h
	$(CC) $(CFLAGS) ../2.1/mysem.c -c -o ../2.1/mysem.o

clean:
	rm -r main ../2.1/mysem.o