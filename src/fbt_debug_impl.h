
/**
 * the includes
 */
#include <pthread.h>
#include <assert.h>

#include "fbt_datatypes.h"




#ifdef PBREAKGDB

#define BREAK_ON_TRANSL_IMPL(org_addr, transl_addr)	\
    do {						\
	if(break_on_transl_addr == (org_addr))		\
	    break_on_transl(org_addr, transl_addr);	\
    } while (0)


/*
 * Useful in gdb to break on translation of a certain address:
 * break break_on_transl
 * p pbreak(address)
 */
extern void *break_on_transl_addr;
extern void pbreak(void *break_addr);
extern void break_on_transl(void *org_addr, void *transl_addr);

#endif
