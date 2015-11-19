
#include <stdlib.h>
#include <stdio.h>
#include "libdis.h"


int fac(int n);

int main(int argc, char **argv)
{
	/* do something */
	printf("fac(5): %d\n", fac(5));
	
	/* now lets disassemble some instructions and see the result */
	/*************************************************************/
	
	x86_insn_t *my_instr = malloc(sizeof(x86_insn_t));
	
	/* disassemble instruction at some address; set 1st and 3rd argument to the address of the instruction */
	int instr_length = x86_disasm(0x08048552, 12, 0x08048552, 0, my_instr);
	
	/* let's print some info we retrieved */
	if (instr_length > 0) {
		printf(" addr: %#x\n mnemonic: %s\n prefix_string: %s\n", my_instr->addr, my_instr->mnemonic, my_instr->prefix_string);
		printf(" bytes:");
		int i = 0;
		for (i = 0; i < my_instr->size; i++) {
			printf(" %02X", my_instr->bytes[i]);
		}
		printf("\n");
		printf(" opcode size: %d\n", my_instr->opcode_size);
		/*printf(" operators:");
		for (i=0; i < */
	}	
	
	return 0;
}



/* returns the factorial of the argument */
int fac(int n)
{
	if (n < 1) {
		return 1;
	}
	return n * fac(n-1);
}
