include ../../Makedefs

.PHONY: all clean test build

all: build

build:
	make -C cfi_illegal_dso_call
	make -C cfi_illegal_dso_jmp
	make -C cfi_no_symbol_call
	make -C cfi_dst_not_in_exec
	make -C hw_stripped
	make -C sharedlib_stripped

test: build
	make -C cfi_illegal_dso_call test
	make -C cfi_illegal_dso_jmp test
	make -C cfi_no_symbol_call test
	make -C cfi_dst_not_in_exec test
	make -C hw_stripped test
	make -C sharedlib_stripped test

clean: 
	make -C cfi_illegal_dso_call clean
	make -C cfi_illegal_dso_jmp clean
	make -C cfi_no_symbol_call clean
	make -C cfi_dst_not_in_exec clean
	make -C hw_stripped clean
	make -C sharedlib_stripped clean

