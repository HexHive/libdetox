#include <stdio.h>
#include <stdlib.h>
                                             
static void staticFunc() {
    printf("staticFunc() called, not allowed!\n");
	exit(0);
}

void (*normalFunc()) () {
	printf("normalFunc() called, Ok\n");
	return staticFunc;
}
