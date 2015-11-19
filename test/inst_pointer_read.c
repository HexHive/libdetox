#include <stdio.h>

/* Tests reading the instruction pointer. 
 * Cannot be accessed directly (as a register), 
 * only as the return pointer on the stack */

int instr_pointer = 0;

void read_instptr() 
{
	printf("entered function\n");
	__asm__ __volatile("pushl %edi");
	__asm__ __volatile("movl 4(%ebp), %edi");
	__asm__ __volatile("movl %edi, instr_pointer");
	__asm__ __volatile("popl %edi");
	printf("end of function\n");
}

int main(int argc, char** argv) 
{
	
	read_instptr();
	printf("instruction pointer first invocation: %#x\n", instr_pointer);
	read_instptr();
	printf("instruction pointer second invocation: %#x\n", instr_pointer);
  return 0;	
}

