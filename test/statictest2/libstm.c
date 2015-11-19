#include "stm_settings.h"
#include <stdio.h>
#include "stm_translate.h"
#include "stm_util.h"
#include "stm_tcache.h"

void (*stm_commit_address)() = 0;

void stm_start_transaction(void (*commit_function)()) 
{
	
	/* initialize data structure for tcache (hash table) */
	tcache_init();
	
	/* set memory address of the stub of stm_commit_transaction in the client program */
	stm_commit_address = commit_function;

	/* find out return instruction pointer (=beginning of first TU)*/
	void *tu_begin = read_rip();
	
	/* translate first TU without jumping to it */
	void *ccf_begin = translate_noexecute(tu_begin);
	
	printf("starting transaction at 0x%x (orig: 0x%x)\n", ccf_begin, tu_begin);
	
	/* overwrite return instruction pointer with address of first instruction in CCF */
	change_rip(ccf_begin);
	
	printf("changed eip: 0x%x\n", read_rip());
}


void stm_commit_transaction()
{
	printf("If you can read this message, this means that untranslated code was executed between stm_start_transaction() and stm_commit_transaction().\n");
	
}


