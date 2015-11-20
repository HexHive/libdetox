include ../../../Makedefs

CC = gcc 
CFLAGS = -g -m32

.PHONY: all clean test main main.rtld

all: lib main main.rtld

test: lib main
	./main
	
lib: lib.c
	gcc -m32 -shared -fPIC -o lib.so lib.c

main: main.c
	gcc -m32 -o main main.c ./lib.so
	
main.rtld:
	gcc -m32 -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld main.c ./lib.so
	
clean:
	rm -f main main.rtld *.o *~ *.so
