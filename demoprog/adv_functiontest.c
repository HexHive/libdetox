/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libfastbt.h>
#include <stdio.h>

/* extern int read_ebp(); */


int fac(int n)
{
	if (n < 2) {
		return 1;
	} else {
		return n * fac(n - 1);
	}
}

int main(int argc, char **argv)
{
	fbt_init(NULL);
	int j = 14;
	/* printf("frame pointer before: %#x\n", read_ebp()); */
	fbt_start_transaction(&fbt_commit_transaction);
	/* printf("frame pointer after: %#x\n", read_ebp()); */
	volatile int i = 2;
	i = i + ++j; // 17
	i++; // 18
	i = i * 2; // 36
	i--; // 35
	i += j; // 50
	i += fac(4); // 74
	i -= 15; // 59
	fbt_commit_transaction();
	printf("value of i: %d (should be 59)\n", i);
	printf("faculty of 5: %d\n", fac(5));
	return 0;
}
