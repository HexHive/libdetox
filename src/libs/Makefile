include ../../Makedefs

FILES=uilib.c

.PHONY: exec lib rtld
	
exec:
	$(CC) $(CFLAGS) $(CFLAGSEXEC) -c -I$(INCLUDEDIR) *.c
	
lib:
	$(CC) $(CFLAGS) $(CFLAGSLIB) -c -I$(INCLUDEDIR) *.c
	
rtld:
	$(CC) $(CFLAGS) $(CFLAGSRTLD) -c -I$(INCLUDEDIR) *.c

clean:
	rm -f *.o *~

