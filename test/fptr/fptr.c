#include "opcode_actions.h"
#include "table.map"

/* test if we can use function pointers in the opcode table to perform actions */

void process_instr(int opcode, int pos);

int main(int argc, char **argv)
{
	int instr_stream[] = { 0, 1, 4, 2, 4, 3 };
	
	int i = 0;
	
	for (i = 0; i < sizeof(instr_stream) / sizeof(*instr_stream); i++) {
		process_instr(instr_stream[i], i);
	}
	
	return 0;
}

void process_instr(int opcode, int pos)
{
	instr curr_instr = opcode_tbl[opcode];
	curr_instr.action(pos);
}
