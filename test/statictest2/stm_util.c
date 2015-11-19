#include "stm_settings.h"
#include <stdio.h>
#include "stm_util.h"

/* change the return instruction pointer so that it points to address */

/**
 * change the return instruction pointer.
 * Change the return instruction pointer such that after the *calling* function
 * returns, the execution will continue at the supplied address
 * @param rip_address the address where the return instruction pointer shall point to
 */
#ifdef DISABLE_ME
void change_rip(void *rip_address)
{
	/* go to the stack frame of the caller */
	__asm__("movl (%ebp), %ecx");
	
	/* write address to 4(%ebp) in stack frame of caller */
	__asm__("movl 8(%ebp), %edx");	/* move rip_address to %edx */
	__asm__("movl %edx, 4(%ecx)");	/* overwrite rip */
	
}
#endif /* DISABLE_ME */

/**
 * read the return instruction pointer.
 * Reads the return instruction pointer of the *calling* function and returns it.
 * @return the return instruction pointer
 */
void *read_rip()
{
	/* go to the stack frame of the caller */
	__asm__("movl (%ebp), %ecx");
	
	/* return rip */
	__asm__("movl 4(%ecx), %eax");
}
	

/**
 * print message on successful transaction commit
 */
#ifdef DISABLE_ME
void end_transaction()
{
	printf("Successfully executed translated code until stm_commit_transaction.\n");
}
#endif /* DISABLE_ME */
