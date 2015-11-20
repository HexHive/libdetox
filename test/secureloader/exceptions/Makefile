include ../../../Makedefs

CC = g++ 
CFLAGS = -m32
FILES = exceptions.cpp
LIBCFLAGS = -Wall -m32
LD = ld
LIBLDFLAGS = -m elf_i386 -shared -z relro

.PHONY: all clean test

all: main main.rtld

test: main
	./main

main: $(FILES)
	$(CC) $(CFLAGS) -o main $(FILES)
	
main.rtld:
	$(CC) $(CFLAGS) -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld $(FILES)
	
clean:
	rm -f main main.rtld *.o *~ *.so
