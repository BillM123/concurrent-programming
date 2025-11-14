CC=gcc
CFLAGS=-Wall -g

main: main.c mypipe.o mypipe.h
	$(CC) $(CFLAGS) -pthread main.c mypipe.o -o main

mypipe.o: mypipe.c mypipe.h
	$(CC) $(CFLAGS) -c -o mypipe.o mypipe.c

test: main
	./main test.txt
	@sed -e '$$a\' -i  test.txt
	@sed -e '$$a\' -i first_output.txt
	@sed -e '$$a\' -i second_output.txt
	@echo ""
	@echo "Now printing first diff"
	@echo
	diff --strip-trailing-cr test.txt first_output.txt
	@echo
	@echo "Now printing second diff"
	@echo
	diff --strip-trailing-cr first_output.txt second_output.txt
	@echo
	@echo "Now printing third diff"
	@echo
	diff --strip-trailing-cr test.txt second_output.txt

clean:
	rm -r mypipe.o
	rm -r main