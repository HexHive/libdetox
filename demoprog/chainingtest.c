#include <stdio.h>
#include <libfastbt.h>

int main(int argc, char **argv)
{
	fbt_init(NULL);
	fbt_start_transaction(&fbt_commit_transaction);
	volatile int i = 2;
	if (i < 15) {
		i--;
	} else {
		i++;
	}
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	i++;
	fbt_commit_transaction();
	printf("value of i: %d (should be 19)\n", i);
	return 0;
}


