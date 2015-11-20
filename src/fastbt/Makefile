include ../../Makedefs

FILES=libfastbt.c fbt_mem_mgmt.c fbt_translate.c fbt_code_cache.c fbt_actions.c \
	fbt_llio.c fbt_libc.c fbt_debug.c fbt_trampoline.c fbt_syscall.c \
	fbt_dso.c fbt_mutex.c fbt_restart_transaction.c \
	fbt_algorithms.c fbt_mem_pool.c

.PHONY: generate_exec generate_lib generate_rtld exec lib rtld clean

generate_exec: *.h $(FILES)
	mkdir -p $(GEN_DIR)
	
	for file in $(FILES); do \
		gcc $(CFLAGS) $(CFLAGSEXEC) -E $$file > $(GEN_DIR)$$file; \
	done

	for file in $(FILES); do \
		$(DSL_PATH) $(GEN_DIR)$$file $(GEN_DIR)$$file ; \
	done
	
	cp *.h $(GEN_DIR)

generate_lib: *.h $(FILES)
	mkdir -p $(GEN_DIR)
	
	for file in $(FILES); do \
		gcc $(CFLAGS) $(CFLAGSLIB) -E $$file > $(GEN_DIR)$$file; \
	done

	for file in $(FILES); do \
		$(DSL_PATH) $(GEN_DIR)$$file $(GEN_DIR)$$file ; \
	done
	
	cp *.h $(GEN_DIR)
	
generate_rtld: *.h $(FILES)
	mkdir -p $(GEN_DIR)
	
	for file in $(FILES); do \
		gcc $(CFLAGS) $(CFLAGSRTLD) -E $$file > $(GEN_DIR)$$file; \
	done

	for file in $(FILES); do \
		$(DSL_PATH) $(GEN_DIR)$$file $(GEN_DIR)$$file ; \
	done
	
	cp *.h $(GEN_DIR)
	
exec: generate_exec
	$(CC) $(CFLAGS) $(CFLAGSEXEC) -c -I$(INCLUDEDIR) generated/*.c
	
lib: generate_lib
	$(CC) $(CFLAGS) $(CFLAGSLIB) -c -I$(INCLUDEDIR) generated/*.c
	
rtld: generate_rtld
	$(CC) $(CFLAGS) $(CFLAGSRTLD) -c -I$(INCLUDEDIR) generated/*.c

clean:
	rm -f *.o *~
	rm -rf $(GEN_DIR)
