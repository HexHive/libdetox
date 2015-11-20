include ../../Makedefs

FILES = sl_loader.c sl_libsearch.c sl_map.c sl_so_chain.c sl_sym_lookup.c sl_libc.c sl_tls.c sl_load.c sl_reloc.c sl_libdetox.c sl_gscope.c

.PHONY= exec lib rtld clean

all: exec lib rtld

exec: $(FILES)
	$(CC) $(CFLAGS) $(CFLAGSEXEC) -c -I../fastbt/ $(FILES)

lib: $(FILES)
	$(CC) $(CFLAGS) $(CFLAGSLIB) -c -I../fastbt/ $(FILES)

rtld: $(FILES)
	$(CC) $(CFLAGS) $(CFLAGSRTLD) -c -I../fastbt/ $(FILES)

clean:
	rm -f *.o *~
