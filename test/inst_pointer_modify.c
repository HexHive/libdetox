#include <stdio.h>

/* Tests reading AND MODIFYING the return instruction pointer */

int instr_pointer = 0;
int i = 1;

void modify_instptr() 
{
	printf("entered function modify_instptr\n");
	printf("we try to modify the return instruction pointer such that an \"i++\" is skipped. i should still have the value 1 at the end.\n");
	__asm__ __volatile("pushl %edi");
	__asm__ __volatile("movl 4(%ebp), %edi");
	__asm__ __volatile("addl $11, %edi");	/* increase rip to skip instruction */
	__asm__ __volatile("movl %edi, 4(%ebp)");
	__asm__ __volatile("movl %edi, instr_pointer");
	__asm__ __volatile("popl %edi");
	printf("end of function modify_instptr\n");
}

int main(int argc, char** argv) 
{
	printf("value of i: %d\n", i);
	
	modify_instptr();

	i++; /* we will try to skip this instruction */

	printf("value of i at the end: %d\n", i);
	
	return 0;
}

