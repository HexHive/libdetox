
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
#export CFLAGS = -O2  -ggdb -Wall -DDEBUG -DDUMP_GENERATED_CODE

#export CFLAGS = -O2 -ggdb -Wall -DDEBUG

# cflags for lightweight debugging
#export CFLAGS = -O2 -ggdb -Wall -DNDEBUG -DDUMP_GENERATED_CODE

# cflags for profiling (-g flag for required for annotated output)
#export CFLAGS = -O2 -Wall -DNDEBUG -g

# cflags for productive use
export CFLAGS = -O3 -Wall -g -DNDEBUG

# optimizations
#export CFLAGS += -Winline -finline-limit=30000 --param inline-unit-growth=512 --param large-function-growth=2048 -funit-at-a-time 

#export CFLAGS += -DFBT_STATISTIC

# ret optimizations:
#export CFLAGS += -DFBT_RET_PREDICTION
#export CFLAGS += -DFBT_RET_STACK
#export CFLAGS += -DFBT_RET_STACK_OVERFLOW_CHECK
export CFLAGS += -DFBT_RET_FAST

export CFLAGS += -DFBT_INLINE_CALLS

export CFLAGS += -DFBT_IND_CALL_PREDICTION

# use fast implementation of tcache_find (assembly)
export CFLAGS += -DFBT_FIND_FAST

# inline find fast into indirect jump
export CFLAGS += -DFBT_IND_JUMP_INLINE_FIND_FAST

# jmp r/m32 and call r/32 optimization
#export CFLAGS += -DFBT_IND_PREDICTION

# include LDPRELOAD code (signal replacement and so on)
export CFLAGS += -DLDPRELOAD

# should we hijack control the flow right from the start?
#export CFLAGS += -DHIJACKCONTROL


.PHONY: demoprog library clean uninstall benchmarks

library:
	$(MAKE) -C src


all: library demoprog

force: clean library install demoprog



demoprog:
	$(MAKE) -C demoprog

install: library
	$(MAKE) -C src install

benchmarks:	export run_all = false
benchmarks:
	$(MAKE) -C benchmarks

all_benchmarks:	export run_all = true
all_benchmarks:
	$(MAKE) -C benchmarks

clean:
	$(MAKE) -C src clean
	$(MAKE) -C demoprog clean
	$(MAKE) -C benchmarks clean

uninstall:
	$(MAKE) -C src uninstall
