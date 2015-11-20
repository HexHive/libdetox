#include <stdio.h>
#include "lib.h"
                 
int main()
{
	void (*func)();
	func = normalFunc();
	printf("funcptr = %x\n", (unsigned int)func);
    asm volatile ( "jmp *%0" : : "m"(func) );
    return 0;
}
