/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libfastbt.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	fbt_init(NULL);
	int j = 14;
	fbt_start_transaction(&fbt_commit_transaction);
	volatile int i = 2;
	i = i + ++j;
	i++;
	i = i * 2;
	i--;
	i += j;
	i -= 15;
	i = i * i;
	i += j++;
	i++;
	fbt_commit_transaction();
	printf("value of i: %d\n", i);
	return 0;
}
