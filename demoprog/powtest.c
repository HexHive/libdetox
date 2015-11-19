#include <stdio.h>
#include <libfastbt.h>

#include <math.h> 

int main(int argc, char **argv)
{
	fbt_init(NULL);
	fbt_start_transaction(&fbt_commit_transaction);
	volatile int i = 0;

	int j = 14;
/*
	int n = 10000000;
	for(j = 0; j < n; j++) {
		i += (int)pow(j/100.0,j/1000.0);
	}
*/
	i = (int)pow(j,i+2);

	fbt_commit_transaction();
	printf("value of i: %d (should be 196)\n", i);
	return 0;
}


