# don't invoke make in this directory! always invoke make in the libstm directory

cflags = -m32 -O -ggdb

statictest: functiontest.o libstm.o stm_actions.o stm_disassemble.o stm_tcache.o stm_asm_functions.o stm_translate.o stm_util.o
	gcc $(cflags) -o statictest libstm.o functiontest.o stm_actions.o stm_disassemble.o stm_tcache.o stm_asm_functions.o stm_translate.o stm_util.o

functiontest.o: functiontest.c
	gcc $(cflags) -c -o functiontest.o functiontest.c

libstm.o: libstm.c libstm.h stm_settings.h
	gcc $(cflags) -c -o libstm.o libstm.c

stm_translate.o: stm_translate.c stm_translate.h stm_settings.h
	gcc $(cflags) -c -o stm_translate.o stm_translate.c

stm_asm_functions.o: stm_asm_functions.S
	gcc $(cflags) -c -o stm_asm_functions.o stm_asm_functions.S
	
stm_tcache.o: stm_tcache.c stm_tcache.h stm_settings.h
	gcc $(cflags) -c -o stm_tcache.o stm_tcache.c
	


stm_disassemble.o: stm_disassemble.c stm_disassemble.h stm_opcode_tables.h stm_settings.h
	gcc $(cflags) -c -o stm_disassemble.o stm_disassemble.c
	

stm_actions.o: stm_actions.c stm_actions.h stm_settings.h
	gcc $(cflags) -c -o stm_actions.o stm_actions.c


stm_util.o: stm_util.c stm_util.h stm_settings.h
	gcc $(cflags) -c -o stm_util.o stm_util.c

read_ebp.o: read_ebp.h read_ebp.S
	gcc $(cflags) -c -o read_ebp.o read_ebp.S

# gcc $(cflags) -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -c -o stm_util.o stm_util.c


.PHONY: clean uninstall
clean: 
	rm -f *.o 
	rm -f statictest
	
