include ../../../Makedefs

CC = gcc 
CFLAGS =  -g -m32 -L. -Wl,--hash-style=both

.PHONY: all clean test

all: main main.rtld

test: main
	./main

main: $(FILES)
	$(CC) $(CFLAGS) -o main main.c 
	
main.rtld:
	$(CC) $(CFLAGS) -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld main.c
	
clean:
	rm -f main main.rtld *.o *~
