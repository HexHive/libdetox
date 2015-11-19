# don't invoke make in this directory! always invoke make in the libfastbt directory

CC=gcc

# use PositionIndependentCode in object files
LIBFLAGS=-fPIC -c

# our objects
OBJS=libfastbt.o fbt_translate.o fbt_asm_functions.o fbt_tcache.o fbt_disassemble.o fbt_actions.o fbt_trampoline.o fbt_debug.o fbt_signals.o fbt_statistic.o fbt_rbtree.o fbt_mem_protection.o fbt_sec_syscalls.o fbt_syscall_table.o fbt_llio.o fbt_mem_alloc.o

LIBNAME=libfastbt.so
# major version
LIBVERS=0
# minor version
LIBMIN=1.0

# place for the static library
STATICLIB=../lib/libfastbt.a

.PHONY: clean uninstall includes library asmdefs-prog

library: fbt_asm_offsets.h $(LIBNAME).$(LIBVERS).$(LIBMIN) $(STATICLIB) includes

fbt_asm_offsets.h: asmdefs
	./asmdefs > fbt_asm_offsets.h

asmdefs: asmdefs.c fbt_datatypes.h fbt_private_datatypes.h
	gcc $(CFLAGS) -o asmdefs -m32 asmdefs.c

$(STATICLIB): $(OBJS)
	ar cru $(STATICLIB) $^

$(LIBNAME).$(LIBVERS).$(LIBMIN): $(OBJS)
	$(CC) -shared $(LDFLAGS) -Wl,-soname,$(LIBNAME).$(LIBVERS) -o $(LIBNAME).$(LIBVERS).$(LIBMIN) $(OBJS) -lc -lpthread -ldl

includes: libfastbt.h fbt_datatypes.h fbt_actions.h
	cp -f libfastbt.h fbt_datatypes.h fbt_actions.h ../include/

libfastbt.o: *.c *.S
	${CC} ${CFLAGS} $(LIBFLAGS) *.c *.S

#.c.o:
#	${CC} ${CFLAGS} $(LIBFLAGS) $<
#
#.S.o:
#	${CC} ${CFLAGS} $(LIBFLAGS) $<


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
	rm -rf .libs
	rm -rf ../lib/*.a
	rm -rf ../include/*.h
	rm -f fbt_asm_offsets.h
	rm -f asmdefs

uninstall:
	sudo rm -f $(INSTPATH)/$(LIBNAME).$(LIBVERS).$(LIBMIN) $(INSTPATH)/$(LIBNAME).$(LIBVERS) $(INSTPATH)/$(LIBNAME) $(INCPATH)/libfastbt.h $(INCPATH)/fbt_datatypes.h $(INCPATH)/fbt_actions.h
	sudo ldconfig -n $(INSTPATH)

