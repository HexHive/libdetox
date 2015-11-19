# don't invoke make in this directory! always invoke make in the libfastbt directory

CC=gcc

# use PositionIndependentCode in object files
LIBFLAGS=-fpic -c

# our objects
FILES=libfastbt.c fbt_translate.c fbt_tcache.c fbt_disassemble.c fbt_actions.c fbt_trampoline.c fbt_debug.c fbt_signals.c fbt_statistic.c fbt_rbtree.c fbt_mem_protection.c fbt_llio.c fbt_mem_alloc.c fbt_libc.c

ASMFILES=fbt_asm_functions.S

OBJS=libfastbt.o fbt_translate.o fbt_asm_functions.o fbt_tcache.o fbt_disassemble.o fbt_actions.o fbt_trampoline.o fbt_debug.o fbt_signals.o fbt_statistic.o fbt_rbtree.o fbt_mem_protection.o fbt_llio.o fbt_mem_alloc.o fbt_libc.o
# fbt_sec_syscalls.o fbt_syscall_table.o 

AUXOBJS=fbt_translate.o,fbt_asm_functions.o,fbt_tcache.o,fbt_disassemble.o,fbt_actions.o,fbt_trampoline.o,fbt_debug.o,fbt_signals.o,fbt_statistic.o,fbt_rbtree.o,fbt_mem_protection.o,fbt_llio.o,fbt_mem_alloc.o,fbt_libc.o

LIBNAME=libfastbt.so
# major version
LIBVERS=0
# minor version
LIBMIN=2.1

LIBDIR=../lib
# place for the static library
STATICLIB=$(LIBDIR)/libfastbt.a

.PHONY: clean uninstall includes library asmdefs-prog

library: fbt_asm_offsets.h $(LIBNAME).$(LIBVERS).$(LIBMIN) $(STATICLIB) includes

fbt_asm_offsets.h: asmdefs
	./asmdefs > fbt_asm_offsets.h

asmdefs: asmdefs.c fbt_datatypes.h fbt_private_datatypes.h
	gcc $(CFLAGS) -o asmdefs -m32 asmdefs.c

$(STATICLIB): $(LIBNAME).$(LIBVERS).$(LIBMIN)
	ar cru $(STATICLIB) $^

$(LIBNAME).$(LIBVERS).$(LIBMIN): *.h $(FILES) $(ASMFILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -combine -o libfastbt.o $(FILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -combine -o libfastbtasm.o $(ASMFILES)
	$(CC) -shared $(LDFLAGS) -Wl,-soname,$(LIBNAME).$(LIBVERS) -o $(LIBNAME).$(LIBVERS).$(LIBMIN) libfastbt.o libfastbtasm.o -lc -ldl -lpthread
	cp $(LIBNAME).$(LIBVERS).$(LIBMIN) $(LIBDIR)

includes: libfastbt.h fbt_datatypes.h fbt_actions.h
	cp -f libfastbt.h fbt_datatypes.h fbt_actions.h ../include/

libfastbt.o: $(FILES) $(ASMFILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -combine -o libfastbt.o $(FILES)
	${CC} ${CFLAGS} $(LIBFLAGS) -combine -o libfastbtasm.o $(ASMFILES)


install: library
	sudo cp $(LIBNAME).$(LIBVERS).$(LIBMIN) $(INSTPATH)/
	sudo ln -sf $(INSTPATH)/$(LIBNAME).$(LIBVERS).$(LIBMIN) $(INSTPATH)/$(LIBNAME).$(LIBVERS)
	sudo ln -sf $(INSTPATH)/$(LIBNAME).$(LIBVERS).$(LIBMIN) $(INSTPATH)/$(LIBNAME)
	sudo ldconfig -n $(INSTPATH)
	sudo install -c libfastbt.h  fbt_datatypes.h fbt_actions.h $(INCPATH)
	cp $(LIBNAME).$(LIBVERS).$(LIBMIN) ../lib
	ln -sf ../lib/$(LIBNAME).$(LIBVERS).$(LIBMIN) ../lib/$(LIBNAME).$(LIBVERS)
	ln -sf ../lib/$(LIBNAME).$(LIBVERS).$(LIBMIN) ../lib/$(LIBNAME)


clean:
	rm -f *.o *.lo *.la *~
	rm -f $(LIBNAME).$(LIBVERS).$(LIBMIN)
	rm -rf ../lib/*.a
	rm -rf ../include/*.h
	rm -f fbt_asm_offsets.h
	rm -f asmdefs

uninstall:
	sudo rm -f $(INSTPATH)/$(LIBNAME).$(LIBVERS).$(LIBMIN) $(INSTPATH)/$(LIBNAME).$(LIBVERS) $(INSTPATH)/$(LIBNAME) $(INCPATH)/libfastbt.h $(INCPATH)/fbt_datatypes.h $(INCPATH)/fbt_actions.h
	sudo ldconfig -n $(INSTPATH)

