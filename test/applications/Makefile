include ../../Makedefs

.PHONY: all clean test build

all: build

build:
	make -C hw
	make -C sharedlib
	make -C sharedlibPIE

test: build
	make -C hw test
	make -C sharedlib test
	make -C sharedlibPIE test
	
clean: 
	make -C hw clean
	make -C sharedlib clean
	make -C sharedlibPIE clean
