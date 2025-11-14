CC=gcc
CFLAGS=-Wall -g

.PHONY: data.bin

main: main.c
	$(CC) $(CFLAGS) main.c -lpthread -o main  -fsanitize=address

data.bin: bin_generator
	@./bin_generator

bin_generator: bin_generator.c
	$(CC) $(CFLAGS) bin_generator.c -o bin_generator

clean:
	rm data.bin
	rm bin_generator
	rm main