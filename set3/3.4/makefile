CC = gcc
CFLAGS = -Wall -g -fsanitize=address# -fsanitize=bounds

main: main.c
	$(CC) $(CFLAGS) -pthread main.c -o main

clean:
	rm main
