include ../Makedefs

FILES = sl_loader.c sl_libsearch.c sl_map.c sl_so_chain.c sl_sym_lookup.c sl_libc.c sl_tls.c sl_load.c sl_reloc.c sl_libdetox.c sl_gscope.c

.PHONY=all clean

all: loader

loader: $(FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDEDIR) -combine -o loader $(FILES) ../../src/libfastbt.o
	cp loader ../../lib

clean:
	rm -f loader *.o *~
