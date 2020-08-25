CC = gcc
CFLAGS = -Wall -Werror -pedantic -g -lpthread

.PHONY: clean

main: main.o utilities.o
	$(CC) $(CFLAGS) -o main main.o utilities.o

main.o: main.c utilities.h
	$(CC) $(CFLAGS) -c main.c 

utilities.o: utilities.c utilities.h
	$(CC) $(CFLAGS) -c utilities.c

clean:
	-rm main utilities.o main.o