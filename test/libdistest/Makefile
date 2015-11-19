libdistest: libdistest.o
	gcc -ggdb -ldisasm -o libdistest libdistest.o
	
libdistest.o: libdistest.c
	gcc -ggdb -c -o libdistest.o libdistest.c 

clean:
	rm libdistest.o
	rm libdistest
	
