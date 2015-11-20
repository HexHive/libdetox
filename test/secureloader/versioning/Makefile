include ../../../Makedefs

CC = gcc 
CFLAGS = -g -m32 -Wl,--hash-style=both -Wl,-rpath=$(CURDIR)
LIBCFLAGS = -Wall -m32 -fPIC -shared 
LD = ld
LIBLDFLAGS = -m elf_i386 -shared

.PHONY: all clean test

all: main main.rtld

test: main
	./main

main: lib1.so main.o
	$(CC) $(CFLAGS) -o main main.c -ldl lib1.so
	
main.rtld:
	$(CC) $(CFLAGS) -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld main.c -ldl lib1.so
	
lib1.so: lib1.o
	$(LD) $(LIBLDFLAGS) -version-script vers.vs -soname lib1.so -o lib1.so lib1.o

%.o: %.c 
	$(CC) $(LIBCFLAGS) -c $<

clean: 
	rm -f main main.rtld *.so *.o

