include ../../../Makedefs

CC = gcc 
CFLAGS = -g -m32

.PHONY: all clean test main

all: main main.rtld

test: main
	./main

main: main.c
	gcc -m32 -o main main.c
	strip main
	
main.rtld:
	gcc -m32 -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld main.c
	strip main.rtld
	
clean:
	rm -f main main.rtld *.o *~ *.so
