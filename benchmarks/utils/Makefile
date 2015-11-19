CC=cc 
CFLAGS = -O2

CLOCKOBJ = gtod.o 


.c.o:
	${CC} ${CFLAGS}  -c $*.c 

default:libgetclock.a 

getclock.o: getclock.c 
	$(CC) $(CFLAGS) -c $*.c

libgetclock.a: getclock.o $(CLOCKOBJ) 
	ar -rv libgetclock.a getclock.o $(CLOCKOBJ)

clean: 
	rm -f *.o libgetclock.a




