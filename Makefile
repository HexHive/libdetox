# have a look at the different Makedefs variables to configure Lockdown/TRuE
include ./Makedefs

.PHONY: all clean build test documentation

all: build

build:
	@echo "\n>>>>>>>>>> BUILD TRuE <<<<<<<<<<\n"
	mkdir -p bin/
	mkdir -p lib/
	make -C src all
	cp lib/$(RTLDNAME).so /tmp/

documentation:
	doxygen doxygen.config

test:
	make -C test

clean:
	make -C src clean
	rm -rf documentation
	rm -f bin/$(EXECNAME) lib/$(LIBNAME).so.$(LIBVERS).$(LIBMIN) lib/$(RTLDNAME).so
	rm -rf bin/
	rm -rf lib/
	make -C test clean
	rm -f cppcheck-result.xml
	rm -f debug.txt
	rm -f code_dump.txt
	rm -f jmpTable_dump.txt
	rm -f test/*/*/debug.txt
	rm -f test/*/*/code_dump.txt
	rm -f test/*/*/jmpTable_dump.txt
	rm -f test/*/*/secmetrics_and_stats.txt
	rm -f secmetrics_and_stats.txt

cppcheck:
	@echo "\n>>>>>>>>>> CPPCHECK <<<<<<<<<<\n"
	cppcheck -f --quiet --xml-version=2 ./ 2> cppcheck-result.xml

it: clean build test
