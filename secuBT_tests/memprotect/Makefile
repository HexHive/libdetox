CFLAGS = -ggdb -Wall

rodata: memprotect_test.c
	gcc ${CFLAGS} -o memprotect_test memprotect_test.c

.PHONY: clean

clean:
	rm -f memprotect_test

