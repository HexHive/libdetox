include ../Makedefs
include ../Makedefs.security
include ../Makedefs.extensions
include ../Makedefs.performance

# activate optimizations
CFLAGS+= $(OPTFLAGS) -fno-stack-protector

# relro with global offset table protection
LDFLAGS += -Wl,-z,relro,-z,now,-z,initfirst $(I386)

# use gcc
CC=gcc
# flags for position independent code in object files
LIBFLAGS=-fpic -c

# object files
FILES=libfastbt.c fbt_mem_mgmt.c fbt_translate.c fbt_code_cache.c fbt_actions.c \
	fbt_llio.c fbt_libc.c fbt_debug.c fbt_trampoline.c fbt_syscall.c \
	fbt_dso.c

# name of the library
LIBNAME=libfastbt
# major version
LIBVERS=0
# minor version
LIBMIN=3.0

LIBDIR=../lib

# we include some kernel stuff as we directly call system calls.
INCLUDEDIR=/lib/modules/$(shell uname -r)/build/arch/x86/include

.PHONY: clean includes

all: $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(LIBNAME).a

$(LIBNAME).so.$(LIBVERS).$(LIBMIN): *.h $(FILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -I$(INCLUDEDIR) -combine -o libfastbt.o $(FILES)
	$(CC) -shared $(LDFLAGS) -Wl,-soname,$(LIBNAME).so.$(LIBVERS) -o \
	$(LIBNAME).so.$(LIBVERS).$(LIBMIN) libfastbt.o
	cp $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(LIBDIR)

$(LIBNAME).a:
	ar cru libfastbt.a libfastbt.o

includes: libfastbt.h fbt_datatypes.h fbt_actions.h
	cp -f libfastbt.h fbt_datatypes.h fbt_actions.h ../include/

clean:
	rm -f *.o *.lo *.la *~
	rm -f $(LIBNAME).so.$(LIBVERS).$(LIBMIN)
	rm -f $(LIBNAME).a
	rm -rf ../lib/*.a
	rm -rf ../include/*.h
	rm -f fbt_asm_offsets.h
	rm -f asmdefs
