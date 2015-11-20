include ../../../Makedefs

CC = gcc 
CFLAGS = -g -m32

.PHONY: all clean test main main.rtld

all: main main.rtld

test: main
	./main

main:
	$(CC) $(CFLAGS) -o main hw.c
	strip main
	
main.rtld:
	$(CC) $(CFLAGS) -Wl,-dynamic-linker,$(RTLDPATH) -o main.rtld hw.c
	strip main.rtld
	
clean:
	rm -f main main.rtld *.o *~
