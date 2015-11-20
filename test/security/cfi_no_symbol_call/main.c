#include <stdio.h>

#define SYS_exit     1

void mainFunc() {

__asm__ volatile("nop\n"         \
				 "nop\n"         \
				 "nop\n"         \
				 "nop\n"         \
				 "nop\n"         \
				 "nop\n"         \
				 "nop\n"         \
				 "movl %0, %%eax\n"         \
                 "movl %1, %%ebx\n"         \
                 "int $0x80"                \
                 : /* no return value */    \
                 : "i"(SYS_exit),           \
                   "g"((long)(0))      \
                 : "memory");
}

int main()
{
	void (*func)();
	func = (void*)((char*)mainFunc+3);
	printf("funcptr = %x\n", (unsigned int)func);
	func();
	return 0;
}
