INCLUDE = -I.
CFLAGS = $(INCLUDE)  -O3 -ggdb
LDFLAGS = -L. -ldisasm
RM = rm
CC = gcc

SRC		= 	libdis.c \
			i386_invariant.c \
			i386.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: libdisasm.a

libdisasm.a: libdis.o i386_invariant.o i386.o
	ar cru $@ $^

clean:
	$(RM) -f *.o *.a


quikdis: quikdis.o libdisasm.a
	$(CC) -o $@ $< $(LDFLAGS)

testdis: testdis.o libdisasm.a
	$(CC) -o $@ $< $(LDFLAGS)

