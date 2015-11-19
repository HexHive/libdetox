#include <stdio.h>
#include <libfastbt.h>

int main(int argc, char **argv)
{
	fbt_init(NULL);
	int j = 14;
	fbt_start_transaction(&fbt_commit_transaction);
	volatile int i = 2;
	i = i + ++j;
	i++;
	if (i < 15) {
		i--;
	} else {
		i++;
	}
	fbt_commit_transaction();
	printf("value of i: %d (should be 19)\n", i);
	return 0;
}


