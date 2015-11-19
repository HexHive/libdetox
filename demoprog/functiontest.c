/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libfastbt.h>
#include <stdio.h>

int bar(int n)
{
	n -= 3;
	n = n * 2;
	return n;
}

int foo(int n)
{
	n += 5;
	n += bar(n);
	return n;
}

int main(int argc, char **argv)
{
	fbt_init(NULL);
	/* char *red = "\033[31m"; */
	/* char *normal = "\033[0m"; */
	int j = 14;
	/* printf("frame pointer before: %s%#x%s\n", red, read_ebp(), normal); */
	fbt_start_transaction(&fbt_commit_transaction);
	//printf("frame pointer during: %s%#x%s\n", red, read_ebp(), normal);
	volatile int i = 2;
	i = i + ++j;
	i++;
	i = i * 2;
	i--;
	/* volatile unsigned int ebp = read_ebp(); */
	i += j;
	i += foo(5);
	i -= 15;
	j += foo(9);
	fbt_commit_transaction();
	printf("value of i: %d\n", i);
	printf("value of j: %d\n", j);

	printf("foo of 5: %d\n", foo(5));
	/* printf("frame pointer during: %s%#x%s\n", red, ebp, normal); */
	/* printf("frame pointer after: %s%#x%s\n", red, read_ebp(), normal); */
	return 0;
}
