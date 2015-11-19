#include <stdio.h>
#include <libfastbt.h>

int main(int argc, char **argv)
{
	volatile int i,j;
	j = 0;

	fbt_init(NULL);
	fbt_start_transaction(&fbt_commit_transaction);
	
	for(i = 0; i < 100; i++)
	{
		//printf("start\n");
		asm("leal label, %eax");
		asm("pushl %eax");
		asm("pushl %eax");

		asm("jmp *4(%esp)");
		j = 1;
		//printf("secret\n");
		asm("label:");
		//printf("label\n");
		asm("popl %eax");
		asm("popl %eax");

	//asm("leal go, %eax");
	//asm("jmp %eax");
	//asm("go:");

	}

	asm("leal go, %eax");
	asm("jmp *%eax");
	asm("go:");

	printf("j: %d (== 0)\n", j);

	fbt_commit_transaction();
	return 0;
}
