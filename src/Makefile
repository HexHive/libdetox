include ../Makedefs

# activate optimizations
CFLAGS += -fno-stack-protector -fno-omit-frame-pointer

# relro with global offset table protection
LDFLAGS += -Wl,-z,relro,-z,now,-z,initfirst $(I386)

# use gcc
CC=gcc
# flags for position independent code in object files
LIBFLAGS=-fpic -c

# Make sure we fail if anything fails in a loop
SHELL := /bin/bash -e

# object files
FILES+=libfastbt.c fbt_mem_mgmt.c fbt_translate.c fbt_code_cache.c fbt_actions.c \
	fbt_llio.c fbt_libc.c fbt_debug.c fbt_trampoline.c fbt_syscall.c \
	fbt_dso.c fbt_mutex.c fbt_restart_transaction.c \
	fbt_algorithms.c fbt_mem_pool.c fbt_lmem.c fbt_lmem_instruction.c \
	fbt_lmem_util.c fbt_lmem_spilling.c fbt_lmem_malloc.c \
	patching/fbt_patching.c patching/fbt_patching_util.c \
	patching/fbt_patching_translate.c patching/fbt_patching_socket.c \
	patching/fbt_patching_filesystem.c

LIBDIR=../lib

# we include some kernel stuff as we directly call system calls.
INCLUDEDIR=/lib/modules/$(shell uname -r)/build/arch/x86/include

GEN_DIR=generated/

.PHONY: clean includes

all: $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(LIBNAME).a

$(LIBNAME).so.$(LIBVERS).$(LIBMIN): *.h $(FILES)
	mkdir -p $(GEN_DIR)
	mkdir -p $(GEN_DIR)/patching
	
	for file in $(FILES); do \
		gcc ${CFLAGS} -E $$file > $(GEN_DIR)$$file; \
	done

	for file in $(FILES); do \
		$(DSL_PATH) $(GEN_DIR)$$file $(GEN_DIR)$$file ; \
	done
	
	cp *.h $(GEN_DIR)

	${CC} ${CFLAGS} $(LIBFLAGS) -I$(INCLUDEDIR) generated/patching/*.c generated/*.c
	$(CC) -shared -Wl,-soname,$(LIBNAME).so.$(LIBVERS) -o \
	$(LIBNAME).so.$(LIBVERS).$(LIBMIN) *.o $(LDFLAGS) 
	cp $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(LIBDIR)

	rm -R generated/

$(LIBNAME).a:
		ar cru libfastbt.a libfastbt.o

includes: libfastbt.h fbt_datatypes.h fbt_actions.h 
	cp -f libfastbt.h fbt_datatypes.h fbt_actions.h fbt_algorithms.h ../include/


clean:
	rm -f *.o *.lo *.la *~ *.as *.out
	rm -f $(LIBNAME).so.$(LIBVERS).$(LIBMIN)
	rm -f $(LIBNAME).a
	rm -rf ../lib/*.a
	rm -rf ../include/*.h
	rm -f fbt_asm_offsets.h
	rm -f asmdefs
	rm -rf generated
