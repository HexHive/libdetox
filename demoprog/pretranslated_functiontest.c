/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <libfastbt.h>
#include <stdio.h>

int foobar = 0;

void bar(void)
{
	foobar = 1337;
	set_return_to_translated();
}

void foo(void)
{
	foobar = -1;
}

int main(int argc, char **argv)
{
	fbt_init(NULL);

	fbt_add_translated_function(foo, bar);

	fbt_start_transaction(&fbt_commit_transaction);
	foo();
	fbt_commit_transaction();

	printf("value of foobar: %d (should be 1337)\n", foobar);

	return 0;
}
