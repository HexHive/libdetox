/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libfastbt.h>
#include <stdio.h>

/* extern int read_ebp(); */

int main(int argc, char **argv)
{
	fbt_init(NULL);
	int j = 14;
	/* printf("frame pointer before: %#x\n", read_ebp()); */
	fbt_start_transaction(&fbt_commit_transaction);
	/* printf("frame pointer after: %#x\n", read_ebp()); */
	volatile int i = 2;
	i = i + ++j;
	i++;
	i = i * 2;
	i--;
	i += j;
	i -= 15;
	fbt_commit_transaction();
	printf("value of i: %d\n", i);
	return 0;
}
