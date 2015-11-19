
# installation path of the shared library
export INSTPATH = /usr/lib

# directory where the header file will be copied to
export INCPATH = /usr/include

# allows building demoprog without installing
CURRENT_DIR:=$(shell pwd)
export CPATH = $(CURRENT_DIR)/include
export LIBRARY_PATH = $(CURRENT_DIR)/lib

# use -DDEBUG to include debugging infos (same as #define DEBUG)
# use -DDUMP_GENERATED_CODE to dump generated code (same as #define DUMP_GENERATED_CODE)
# use -DNDEBUG to remove assertions from C code

# cflags for debugging
#export CFLAGS = -O3  -ggdb -Wall -DDEBUG -DDUMP_GENERATED_CODE

#export CFLAGS = -O2 -ggdb -Wall -DDEBUG

# cflags for lightweight debugging
#export CFLAGS = -O2 -ggdb -Wall -DNDEBUG

# cflags for profiling (-g flag for required for annotated output)
#export CFLAGS = -O2 -Wall -DNDEBUG -g

# cflags for productive use
export CFLAGS = -O3 -Wall -g -DNDEBUG 

# optimizations
export CFLAGS += -Winline -finline-limit=30000 --param inline-unit-growth=512 --param large-function-growth=2048 -funit-at-a-time
# use special linker option to hide all unneded symbols
#-fvisibility=hidden


# relro without global offset table protection 
# (default on ubuntu 8.10 and other recent distributions)
#export LDFLAGS += -Wl,-z,relro

# relro with global offset table protection
export LDFLAGS += -Wl,-z,relro,-z,now

#export CFLAGS += -DFBT_STATISTIC

# ret optimizations:
#export CFLAGS += -DFBT_RET_PREDICTION
#export CFLAGS += -DFBT_RET_STACK
#export CFLAGS += -DFBT_RET_STACK_OVERFLOW_CHECK
export CFLAGS += -DFBT_RET_FAST
# ret cache needs another ret opt to handle RET $IMM16
#export CFLAGS += -DFBT_RET_CACHE
#export CFLAGS += -DFBT_RET_PREDICT

export CFLAGS += -DFBT_INLINE_CALLS

export CFLAGS += -DFBT_IND_CALL_PREDICTION
export CFLAGS += -DFBT_IND_CALL_FAST

# inline find fast into the indirect jump function
# that gets called for missed targets
export CFLAGS += -DFBT_IND_JUMP_INLINE_FIND_FAST

# NOTE: use one of the following ind jump optimizations: FBT_IND_PREDICTION or FBT_IND_JUMP_MULTIPLE
# if none of these two is selected then a IND_JUMP_FAST default is used.
export CFLAGS += -DFBT_IND_PREDICTION
# use multiple indirect jump optimizations with jump prediction, jump fast and jump tables
# multiple depends on FBT_IND_PREDICTION as well
export CFLAGS += -DFBT_IND_JUMP_MULTIPLE

# align all functions (that start with pusl %ebp) to 16b
#export CFLAGS += -DFBT_ALIGN_FUNCTIONS
# check for every instruction if it has been translated before
#export CFLAGS += -DFBT_CHECK_TRANSLATED

# include LDPRELOAD code (signal replacement and so on)
export CFLAGS += -DLDPRELOAD

# should we hijack control the flow right from the start?
export CFLAGS += -DHIJACKCONTROL

# should we make it possible to attach a debugger in case of a segfault?
#export CFLAGS += -DSLEEP_ON_FAIL

# should we force a pause on startup to make it possible to attach a debugger 
# at the beginning?
# disable when benchmarking with SPEC CPU 2006
#export CFLAGS += -DSLEEP_ON_INIT

# should we pause on executing the .fini section to make it possible to attach
# a debugger right before a successful execution of a program ends?
#export CFLAGS += -DSLEEP_ON_FINI

# if this is enabled, some info is printed to stderr. disable when 
# benchmarking with SPEC CPU 2006, because the benchmarking tool compares the 
# output to the expected output.
# output is to stderr because some shell scripts evaluate stdout via `<command>`
#export CFLAGS += -DINFO_OUTPUT

# if this is enabled, more verbose info is printed to stderr. disable when 
# benchmarking with SPEC CPU 2006
#export CFLAGS += -DINFO_OUTPUT_VERBOSE

# secuBT: do not allow execution of code in memory that is not marked as 
# executable or in a section of the program or library that is not marked 
# as containing executable code
#export CFLAGS += -DSECU_ENFORCE_NX

# secuBT: allow execution of code in runtime-allocated memory if it was marked 
# as executable with mprotect or mmap. Requires SECU_SYSCALL_AUTH
export CFLAGS += -DSECU_ALLOW_RUNTIME_ALLOC

# secuBT: use system call authorization
# use functions that grant or deny a system call based on its arguments. This 
# flag is currently required for SECU_ALLOW_RUNTIME_ALLOC to work
#export CFLAGS += -DSECU_SYSCALL_AUTH

# secuBT: mark the memory of the program as not executable while the binary 
# translator is running. This can prevent jailbreaks which try to return to 
# untranslated code.
# We cannot mark all the libraries as not executable, because some of them are
# used by the binary translator, for example glibc or pthread.
export CFLAGS += -DSECU_NX_PROG

# secuBT: child processes could easily read sensitive information from 
# /proc/<pid>/ that could be used for a jailbreak attack, and relay it to the 
# original process.
# Disabling this flag will lead to a security problem if other processes 
# are spawned via fork(2) or execve(2). The original process will 
# still be blocked from reading sensitive files in /proc/self
#export CFLAGS += -DSECU_BLOCK_PROC_ACCESS

# secuBT: block calls to the function dl_iterate_phdr, which can be used to 
# find the base address where the secuBT library is loaded
#export CFLAGS += -DSECU_BLOCK_DL_ITERATE_PHDR

# secuBT: check that the executable and the shared libraries do not use any
# symbols that are exported by the secuBT library
#export CFLAGS += -DSECU_DETECT_SYMBOL_COLLISIONS

# secuBT: write-protect all internal data structures before returning control
# to the guest code. This makes jailbreak attacks much more difficult, but 
# has a detrimental effect on performance.
#export CFLAGS += -DSECU_MPROTECT_IDS


#export CFLAGS += -DPBREAKGDB
# Added small GDB helper: Add -DPBREAKGDB to CFLAGS to activate.
# Example use in GDB:
# break main
# r
# p pbreakgdb(SomeFunctionNameOrAddress)
# break break_on_transl
# c
# break *qtransl_addr
# c

# make sure we generate 32bit i386 code (won't compile on 64bit)
#export CFLAGS += -m32

.PHONY: demoprog library clean uninstall benchmarks

library:
	$(MAKE) -C src


all: library

force: clean library install



install: library
	$(MAKE) -C src install

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tableGenerator clean

uninstall:
	$(MAKE) -C src uninstall
