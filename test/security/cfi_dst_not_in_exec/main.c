#include <stdio.h>
#include <sys/mman.h>

#define SYS_exit     1

unsigned char buf[] =
"\x55\x89\xe5\x90\x90\x90\x90\x90\x90\x90\xb8\x01\x00\x00\x00\xbb\x00\x00\x00\x00\xcd\x80\x5d\xc3";

/*
 * 080483e4 <mainFunc>:
 80483e4:	55                   	push   %ebp
 80483e5:	89 e5                	mov    %esp,%ebp
 80483e7:	90                   	nop
 80483e8:	90                   	nop
 80483e9:	90                   	nop
 80483ea:	90                   	nop
 80483eb:	90                   	nop
 80483ec:	90                   	nop
 80483ed:	90                   	nop
 80483ee:	b8 01 00 00 00       	mov    $0x1,%eax
 80483f3:	bb 00 00 00 00       	mov    $0x0,%ebx
 80483f8:	cd 80                	int    $0x80
 80483fa:	5d                   	pop    %ebp
 80483fb:	c3                   	ret 
*/

//void mainFunc() {
//__asm__ volatile("nop\n"         \
				 //"nop\n"         \
				 //"nop\n"         \
				 //"nop\n"         \
				 //"nop\n"         \
				 //"nop\n"         \
				 //"nop\n"         \
				 //"movl %0, %%eax\n"         \
                 //"movl %1, %%ebx\n"         \
                 //"int $0x80"                \
                 //: /* no return value */    \
                 //: "i"(SYS_exit),           \
                   //"g"((long)(0))      \
                 //: "memory");
//}

int main()
{
	void (*func)();
	char* addr = (char*) ((unsigned int)buf & 0xFFFFF000);
	
	printf("addr = %x\n", (unsigned int)addr);
	
	if(mprotect(addr, 4096, PROT_READ|PROT_WRITE|PROT_EXEC) != 0) {
			printf("mprotect call failed\n");
			return 0;
	}
	
	func = (void*)((char*)buf+3);
	printf("funcptr = %x\n", (unsigned int)func);
	func();
	return 0;
}
