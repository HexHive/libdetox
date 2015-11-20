# have a look at CONFIGURATION and the different Makedefs to configure the BT
include ./Makedefs
include ./Makedefs.security
include ./Makedefs.extensions
include ./Makedefs.performance

.PHONY: all clean build test documentation

all: build

build:
	make -C src all

test:
	mv Makedefs Makedefs.tmp
	ln -s Makedefs.debug Makedefs
	make -C src clean all
	make -C test clean all
	make -C test clean
	rm Makedefs
	ln -s Makedefs.production Makedefs
	make -C src clean all
	make -C test clean all
	make -C test clean
	rm Makedefs
	mv Makedefs.tmp Makedefs

documentation:
	doxygen doxygen.config

clean:
	make -C src clean
	make -C test clean
	make -C microbenchmarks clean
	rm -rf documentation
