#include <stdio.h>
#include <libfastbt.h>
#include <math.h>

int main(int argc, char **argv)
{
	fbt_init(NULL);
	volatile double j = 10.0;
	int k;
	volatile double i = 2, l = 2;
	fbt_start_transaction(&fbt_commit_transaction);
	for (k=0; k<100000; k++) {
  	i = pow(l, j);
  }
	fbt_commit_transaction();
	printf("value of i: %f (1024)\n", i);
	return 0;
}


