CC = gcc
CFLAGS = -Wall -g

main: main.c mycoroutines.c mycoroutines.h
	$(CC) $(FLAGS) -pthread main.c mycoroutines.c -o main -fsanitize=address

mycoroutines.o: mycoroutines.c mycoroutines.h
	$(CC) $(FLAGS) -c -o mycoroutines.o mycoroutines.c -fsanitize=address

clean:
	rm -r main
