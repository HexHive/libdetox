# have a look at CONFIGURATION and the different Makedefs to configure the BT
include ./Makedefs

.PHONY: all clean build test documentation

all: build

build:
	make -C src all

loader: build
	make -C trustedloader main

test:
	make -C src clean all
	make -C test clean all
#	make -C test clean

documentation:
	doxygen doxygen.config

clean:
	make -C src clean
	make -C test clean
	make -C microbenchmarks clean
	make -C trustedloader clean
	rm -rf documentation
	rm -f lib/loader lib/$(LIBNAME).so.$(LIBVERS).$(LIBMIN)
