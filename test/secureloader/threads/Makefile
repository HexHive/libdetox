include ../../../Makedefs

CC = gcc 
CFLAGS = -Wall -g -pthread -m32 -L. -Wl,--hash-style=both -Wl,-rpath=./

.PHONY: all clean test

all: main main.rtld

test: main
	./main

main:
	$(CC) $(CFLAGS) -o main main.c
	
main.rtld:
	$(CC) $(CFLAGS) -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld main.c
	
clean: 
	rm -f main main.o main.rtld


