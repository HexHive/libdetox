include ../../Makedefs

.PHONY: all clean test build

all: build

build:
	make -C relocations
	make -C threads
	make -C exceptions
	make -C tls_relocations
	make -C dynamic_loading
	make -C libc_basic
	make -C versioning
	make -C lib_resolve
	make -C irelative_reloc 

test: build
	make -C relocations test
	make -C threads test
	make -C tls_relocations test
	make -C dynamic_loading test
	make -C libc_basic test
	make -C versioning test
	make -C lib_resolve test
	make -C exceptions test
	make -C irelative_reloc test 
	
clean: 
	make -C relocations clean
	make -C threads clean
	make -C tls_relocations clean
	make -C dynamic_loading clean
	make -C libc_basic clean
	make -C versioning clean
	make -C lib_resolve clean
	make -C exceptions clean
	make -C irelative_reloc clean 
