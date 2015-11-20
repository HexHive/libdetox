#include <stdio.h>
#include "lib.h"
#include "helper.h"

int mainInt = 1;
static int mainStaticInt = 2;

void mainFunc();
static void mainStaticFunc();

int main()
{
	printf("main <mainInt %d, mainStaticInt %d>\n", mainInt, mainStaticInt);
	mainFunc();
	mainStaticFunc();
	helperFunc();
	libInt = 5;
	printf("main <helperInt %d, libInt %d>\n", helperInt, libInt);
	libFunc();
	printf("main <helperInt %d, libInt %d>\n", helperInt, libInt);
	return 0;
}

void mainFunc() {
	printf("mainFunc\n");
}

static void mainStaticFunc() {
	printf("mainStaticFunc\n");
}
