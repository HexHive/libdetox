all: libtestme.so.0.1.0 symbols

symbols: symbols.c
	gcc -L . -ldl -ltestme -o symbols symbols.c

testlib.o: testlib.c
	gcc -c -fPIC -o testlib.o testlib.c

libtestme.so.0.1.0: testlib.o
	gcc -shared -Wl,-soname,libtestme.so.0 -o libtestme.so.0.1.0 testlib.o
	ln -s libtestme.so.0.1.0 libtestme.so.0
	ln -s libtestme.so.0 libtestme.so

.PHONY: clean

clean:
	rm -f symbols
	rm -f libtestme.so*
	rm -f *.o

