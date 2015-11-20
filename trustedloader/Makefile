.PHONY: all main test cmain clean libc-src

all: main

main:
	make -C src

test:
	make -C test

cmain:
	make -C src clean

libc-src:
	rm -rf libc-src
	mkdir libc-src
	cd libc-src; apt-get source libc6

clean:
	make -C src clean
	make -C test clean
