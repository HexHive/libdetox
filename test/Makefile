include ../Makedefs

.PHONY: all clean build runtests

all: build

build:
	@echo "\n>>>>>>>>>> BUILD TESTS <<<<<<<<<<\n"
	make -C secureloader
	make -C applications
	# make -C fastbt
	make -C security

runtests:
	./runtests.rb

clean:
	make -C secureloader clean
	make -C applications clean
	# make -C fastbt clean
	make -C security clean
