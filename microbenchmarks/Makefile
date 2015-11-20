include ../Makedefs

# use g++ compiler
CC=gcc

#FASTBT=../src/libfastbt.so.0.3.0
FASTBT=../src/libfastbt.o

INCLUDEDIR=../src/
LIBPATH=../lib

.PHONY: runtest build clean

all: build

build:
	$(CC) $(CFLAGS) -O -I$(INCLUDEDIR) -lpthread microbenchmark.c $(FASTBT) -o test
	$(CC) $(CFLAGS) -O -I$(INCLUDEDIR) -lpthread  iotest.c $(FASTBT) -o iotest
	$(CC) $(CFLAGS) -O -I$(INCLUDEDIR) -lpthread  racetrap.c $(FASTBT) -o racetrap

runtest: build
	LD_LIBRARY_PATH=$(LIBPATH) ./test

clean:
	rm -f *.o test file iotest racetrap debug.txt code_dump.txt jmpTable_dump.txt test.dat *~
