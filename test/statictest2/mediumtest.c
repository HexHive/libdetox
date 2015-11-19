/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libstm.h>
#include <stdio.h>

int read_ebp();

int main(int argc, char **argv)
{
	int j = 14;
	printf("frame pointer before: %#x\n", read_ebp());
	stm_start_transaction(&stm_commit_transaction);
	//printf("frame pointer after: %#x\n", read_ebp());
	volatile int i = 2;
	i = i + ++j;
	i++;
	i = i * 2;
	i--;
	i += j;
	i -= 15;
	stm_commit_transaction();
	printf("value of i: %d\n", i);
	return 0;
}
