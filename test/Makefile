include ../Makedefs

# use g++ compiler
CC=g++

#FASTBT=../src/libfastbt.so.0.3.0
FASTBT=../src/libfastbt.o

INCLUDEDIR=../src/
LIBPATH=../lib:/usr/local/lib

# Enables / disables testing of fbt_algorithms.c
#export CFLAGS += -DTEST_FBT_ALGORITHMS

DSL_FILES = fbt_assembly_generation_test fbt_track_cftx_test fbt_shadow_stack_test fbt_online_patching_test fbt_lmem_test

.PHONY: runtest build clean

all: runtest

build_online_patching:
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -E -x c++ fbt_online_patching_test.dsl > fbt_online_patching_test.cpp.temp
	$(DSL_PATH) fbt_online_patching_test.cpp.temp fbt_online_patching_test.cpp	
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) gtest.cpp fbt_online_patching_test.cpp -lgtest -lpthread $(FASTBT) -o test

build:
	for file in $(DSL_FILES); do \
		cp $$file.dsl $$file.cpp; \
	done

	for file in $(DSL_FILES); do \
		gcc ${CFLAGS} -I$(INCLUDEDIR) -Igtest/include -E $$file.cpp > $$file.cpp.2; \
	done

	for file in $(DSL_FILES); do \
		$(DSL_PATH) $$file.cpp.2 $$file.cpp ; \
	done
	
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -Igtest/include *.cpp -Lgtest/lib -lgtest -ldl -lpthread $(FASTBT) -o test

runtest: build
	LD_LIBRARY_PATH=$(LIBPATH) ./test

clean:
	rm -f fbt_assembly_generation_test.cpp
	rm -f *.cpp.2
	rm -f *.o test file debug.txt code_dump.txt jmpTable_dump.txt fbt_assembly_generation_test.cpp fbt_track_cftx_test.cpp fbt_online_patching_test.cpp fbt_shadow_stack_test.cpp *.as *.out *~
