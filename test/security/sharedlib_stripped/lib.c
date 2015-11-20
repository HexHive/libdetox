#include <stdio.h>

int libInt = 0;

void libFunc() {
    printf("libFunc <libInt %d>\n", libInt);
	libInt = 4;
	printf("libFunc <libInt %d>\n", libInt);
}
