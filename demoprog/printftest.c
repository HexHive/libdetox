#include <stdio.h>
#include <string.h>
#include <libfastbt.h>

int main(int argc, char **argv)
{
	fbt_init(NULL);
	int j = 14;
	//char str[] = { "handcrafted hello from kernel SYS_WRITE in transaction\n" };
	//int len = strlen(str);
	//printf("This should be printed: %s", str);
	fbt_start_transaction(&fbt_commit_transaction);
	j -= argc;
	printf("printing in translated code successful\n");
	//__asm__("movl %0, %%ecx" : : "r"(str) : "%ecx"); // load string
	//__asm__("movl %0, %%edx" : : "r"(len) : "%edx"); // length
	// these two must be first, because stupid GCC clobbers the eax register!
	//__asm__("movl $4, %%eax" : : : "%eax"); // SYS_WRITE
	//__asm__("movl $1, %%ebx" : : : "%ebx"); // stdout
	//__asm__("int $0x80");
	//printf("and now dynamic printf from libc\n");
	j += argc;
	fbt_commit_transaction();
	printf("j after transaction: %d (should be 14)\n", j);
	return 0;
}


