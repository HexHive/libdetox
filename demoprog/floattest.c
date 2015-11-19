#include <stdio.h>
#include <libfastbt.h>

float fadd(float n1, float n2) {
	return n1 + n2;
}

int main(int argc, char **argv) {
	
	fbt_init(NULL);
	float a = 4.5;
	float b = 7.2;
	
	
	fbt_start_transaction(&fbt_commit_transaction);
	
	printf("result: %f, should be 11.7\n", fadd(a, b));
	
	fbt_commit_transaction();
	
	return 0;
}
