/**
 * This handles the debug output that can be customized in the Makefile
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include "fbt_debug.h"

#define PRINT__BUF__SIZE 5001

unsigned char print_buf[PRINT__BUF__SIZE];
unsigned char scan_half_byte(unsigned char hb)
{
	switch(hb & 0x0f) {
		case 10:
			return 'a';
		case 11:
			return 'b';
		case 12:
			return 'c';
		case 13:
			return 'd';
		case 14:
			return 'e';
		case 15:
			return 'f';
	}
	return hb + '0';
}
unsigned char* printnbytes(unsigned char *addr, unsigned int n)
{
	unsigned int i;
	for(i=0; i<n && i<(PRINT__BUF__SIZE-1)/2; ++i) {
		print_buf[2*i] = scan_half_byte(addr[i] >> 4);
		print_buf[2*i+1] = scan_half_byte(addr[i] & 0x0f);
	}
	print_buf[2*i] = '\0';
	return print_buf;
}



#ifdef DUMP_GENERATED_CODE

/* not a nice hack but lets you keep a nice and clean Makefile */
/* while beeing able to turn of dumping from one place */
#include "libdisasm/i386_invariant.c"
#include "libdisasm/i386.c"
#include "libdisasm/libdis.c"

int disasm_inst(unsigned char *addr, char *buf)
{
	struct instr i;
	int size;
	int firstArg = 1;

	size = disassemble_address(addr, &i);

	sprintf(buf, "%-7s", i.mnemonic);
	if (i.src[0]) {
		if (firstArg) {
			sprintf(buf+7, " %s", i.src);
		} else {
			sprintf(buf+7, ", %s", i.src);
		}
		firstArg = 0;
	}
	if (i.aux[0]) {
		if (firstArg) {
			sprintf(buf+7, " %s", i.aux);
		} else {
			sprintf(buf+7, ", %s", i.aux);
		}
		firstArg = 0;
	}
	if (i.dest[0]) {
		if (firstArg) {
			sprintf(buf+7, " %s", i.dest);
		} else {
			sprintf(buf+7, ", %s", i.dest);
		}
		firstArg = 0;
	}

	return (size);

}


/**
 * Prints instructions to the file (and disassembles it)
 */
void print_disasm_inst(FILE* f, char *addr, int len)
{
	int tot_size=0, size, j;
	char buf[100];

	while (tot_size < len) {
		size = disasm_inst((unsigned char*)(addr+tot_size), buf);
		fprintf(f, "%p:   ", addr+tot_size);
		for(j=0; j<size; ++j) {
			fprintf(f, "%.2x", (unsigned char)(*(addr+tot_size+j)));
		}
		for(j=0; j<26-2*size; ++j) {
			fprintf(f, " ");
		}
		fprintf(f, "%s\n", buf);

		tot_size += size;
		/* break out of the loop, if we have an invalid instr. */
		if (size==0) {
		  printf("fastBT print_disasm_inst: size=0\n");
		  break;
		}
	}
}

#endif

#ifdef PBREAKGDB

void * break_on_transl_addr = NULL;

void pbreakgdb(void *break_addr) __attribute__((noinline));
void pbreakgdb(void *break_addr) {
	break_on_transl_addr = break_addr;
}

void break_on_transl(void *qorg_addr, void *qtransl_addr) __attribute__((noinline));
void break_on_transl(void *qorg_addr, void *qtransl_addr) {
	//reference pbreakgdb to make sure it's not optimized away
	pbreakgdb(qorg_addr);
}

#endif
