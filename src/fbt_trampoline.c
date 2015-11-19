/**
 * The trampoline module. Trampolines are used for conditional jumps
 * Trampolines are kept in a linked list and can be recycled after usage.
 *
 * A trampoline is used if a code region is not alraedy translated and the
 * trampoline will then handle the translation of this region and backpatch
 * the old code with the newly generated target
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

#include "fbt_private_datatypes.h"
#include "fbt_trampoline.h"
#include "fbt_asm_functions.h"
#include "fbt_asm_macros.h"
#include "fbt_translate.h"

#include "fbt_debug.h"

/**
 * Allocate a series of trampoline slots
 */
void trampoline_allocate(thread_local_data_t *tld)
{
	cache_free_list_t *new_block;

	PRINT_DEBUG_FUNCTION_START("trampoline_allocate(tld=%p)", tld);
	
	/* allocate memory for trampoline list (page aligned, so we can make it executable) */
	posix_memalign((void*) &(tld->trampoline), sysconf(_SC_PAGESIZE), NUM_TRAMPOLINES * sizeof(trampoline_entry_t));
	
	/* fill with illegal HLT instructions */
#if defined(DEBUG) || !defined(NDEBUG)
	memset(tld->trampoline, 0xF4, NUM_TRAMPOLINES * sizeof(trampoline_entry_t));
#endif	
	/* make executable */
	if (mprotect(tld->trampoline, NUM_TRAMPOLINES * sizeof(trampoline_entry_t), (PROT_READ | PROT_WRITE | PROT_EXEC))!=0) {
		perror("Could not set trampoline memory executable");
		exit(1);
	}


	PRINT_DEBUG("initialized trampoline list at %p", tld->trampoline);

	/* link the list entries */
	int i = 0;
	trampoline_entry_t *current = tld->trampoline;
	for (i = 0; i < (NUM_TRAMPOLINES - 1); i++) {
		current->next = current + 1;
		current++;
	}
	current->next = NULL;	/* last element in list */

	/* TODO: add to list of allocated memory */

	/* add the tcache block to the free list */
	new_block = malloc(sizeof(cache_free_list_t));
	new_block->cache_block = tld->trampoline;
	new_block->next = tld->cache_list;
	tld->cache_list = new_block;


	PRINT_DEBUG_FUNCTION_END(" ");


}

/**
 * add a trampoline entry.
 * Add a trampoline entry that calls the translation function and changes the origin to jump directly to the translated code
 * @param call_target the target address of the original jump (the address that has to be translated)
 * @param origin the address of the jump argument
 * @param handler the function that is called by this trampoline to do work 
 */
void *trampoline_put_handler(thread_local_data_t *tld, void *call_target, void *origin, void (*handler)(thread_local_data_t *, void *, void *))
{
	void *ret_val;
	unsigned char *curr_addr;

	/* make sure, that our thread local data is the same as on our stack */
	assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));

	/* if no free slots, allocate more memory */
	if (tld->trampoline == NULL) {
		trampoline_allocate(tld);
	}

	/* get the address of the next trampoline */
	curr_addr = (unsigned char*) (ret_val = (void*) tld->trampoline);

	/* remove current slot from list of free slots */
	tld->trampoline = tld->trampoline->next;

	PRINT_DEBUG("return value: %p", ret_val);

	/*
	 * write:
	 * pushl origin
	 * pushl call_target
	 * pushl tld
	 * call handler
	 */

	/* if we have no origin, we must use an indirect jump always! (signals and thread creation) */
	if (origin!=NULL) {
	  PUSHL_IMM32(curr_addr, (uint32_t)origin);
	}
	PUSHL_IMM32(curr_addr, (uint32_t)call_target);
	PUSHL_IMM32(curr_addr, (uint32_t)tld);
	CALL_REL32(curr_addr, (uint32_t) handler);

	DUMP_CODE(ret_val, curr_addr - (unsigned char*)ret_val);
	return ret_val;
}


/**
 * add a trampoline entry.
 * Add a trampoline entry that calls the translation function and changes the origin to jump directly to the translated code
 * @param call_target the target address of the original jump (the address that has to be translated)
 * @param origin the address of the jump argument
 */
void *trampoline_put(thread_local_data_t *tld, void *call_target, void *origin)
{
	PRINT_DEBUG_FUNCTION_START("trampoline_put(*tld=%p, *call_target=%p, *origin=%p)", tld, call_target, origin);
	void *ret_val = trampoline_put_handler(tld, call_target, origin, ind_jump_backpatch);
	PRINT_DEBUG_FUNCTION_END(" -> %p", ret_val);
	return ret_val;
}

#ifdef FBT_RET_STACK
/**
 * add an absolute trampoline entry.
 * The only difference to trampoline_put is that an absolute address is backpatched instead of a relative one.
 * @param call_target the target address of the original jump (the address that has to be translated)
 * @param origin the address of the jump argument
 */
void *trampoline_put_abs(thread_local_data_t *tld, void *call_target, void *origin)
{
	PRINT_DEBUG_FUNCTION_START("trampoline_put_abs(*tld=%p, *call_target=%p, *origin=%p)", tld, call_target, origin);
	void *ret_val = trampoline_put_handler(tld, call_target, origin, ind_jump_backpatch_abs);
	PRINT_DEBUG_FUNCTION_END(" -> %p", ret_val);
	return ret_val;
}
#endif

/**
 * add a trampoline entry for chaining optimization.
 * The only difference to trampoline_put is that ind_jump_chaining will be called in the trampoline 
 * instead of ind_jump_backpatch.
 * @see put_trampoline
 * @param call_target the target address of the original jump (the address that has to be translated)
 * @param origin the address of the jump argument
 */
void *trampoline_put_chaining(thread_local_data_t *tld, void *call_target, void *origin)
{
	PRINT_DEBUG_FUNCTION_START("trampoline_put_chaining(*tld=%p, *call_target=%p, *origin=%p)", tld, call_target, origin);
	void *ret_val = trampoline_put_handler(tld, call_target, origin, ind_jump_chaining);
	PRINT_DEBUG_FUNCTION_END(" -> %p", ret_val);
	return ret_val;
}


void *trampoline_put_signal(void *call_target)
{
  thread_local_data_t *tld = pthread_getspecific(tld_key);
  PRINT_DEBUG_FUNCTION_START("trampoline_put_signal(*tld=%p, *call_target=%p)", tld, call_target);

  /* we cast the ind_jump (void ind_jump(tld, void)) to the other type
     trampoline_put_handler takes care of the missing argument! */
  void *ret_val = trampoline_put_handler(tld, call_target, NULL, (void (*)(thread_local_data_t *, void *, void *))ind_jump_signal);
  PRINT_DEBUG_FUNCTION_END(" -> %p", ret_val);
  return ret_val;
}

/**
 * add a trampoline that translates the return address on the stack
 * @param call_target the target address that will be jumped to after the return address has been translated
 */
void *trampoline_put_return_to_translated(thread_local_data_t *tld, void *call_target)
{
  PRINT_DEBUG_FUNCTION_START("trampoline_put_return_to_translated(*tld=%p, *call_target=%p)", tld, call_target);

  void *ret_val;
  unsigned char *curr_addr;

  /* make sure, that our thread local data is the same as on our stack */
  assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));

  /* if no free slots, allocate more memory */
  if (tld->trampoline == NULL) {
    trampoline_allocate(tld);
  }

  /* get the address of the next trampoline */
  curr_addr = (unsigned char*) (ret_val = (void*) tld->trampoline);

  /* remove current slot from list of free slots */
  tld->trampoline = tld->trampoline->next;

  PRINT_DEBUG("return value: %p", ret_val);

  /*
   * write:
   * pushl	%eax
   * pushl	%ecx
   * pushl	%edx
   * movl	12(%esp), %eax
   * pushl	%eax
   * pushl	$tld
   * call	tcache_find
   * test	%eax, %eax
   * jne +1instr
   * call	translate_noexecute
   * movl	%eax, 12(%esp)
   * popl	%edx
   * popl	%ecx
   * popl	%eax

   * call set_return_to_translated_asm_nostackframe
   * jmp call_target
   */
/*   PUSHL_EAX(curr_addr); */
/*   PUSHL_ECX(curr_addr); */
/*   PUSHL_EDX(curr_addr); */
/*   MOVL_SIBIMM8_R32(curr_addr, SIB_ESP, 12, EAX); // movl 12(%esp), %eax */
/*   PUSHL_EAX(curr_addr); */
/*   PUSHL_IMM32(curr_addr, (int32_t)tld); */
/*   CALL_REL32(curr_addr, (int32_t)tcache_find); */
/*   TEST_REG32_REG32(curr_addr, EAX, EAX); */
/*   JNE_I8(curr_addr, +5); */
/*   CALL_REL32(curr_addr, (int32_t)translate_noexecute); */
/*   MOVL_R32_SIBIMM8(curr_addr, EAX, SIB_ESP, 12); // movl %eax, 12(%esp) */
/*   POPL_EDX(curr_addr); */
/*   POPL_ECX(curr_addr); */
/*   POPL_EAX(curr_addr); */

  CALL_REL32(curr_addr, (int32_t)set_return_to_translated_asm_nostackframe);
  JMP_REL32(curr_addr, (int32_t)call_target);
  
  DUMP_CODE(ret_val, curr_addr - (unsigned char*)ret_val);
  
  PRINT_DEBUG_FUNCTION_END(" -> %p", ret_val);
  return ret_val;
}

void trampoline_free(thread_local_data_t *tld, trampoline_entry_t *slot)
{
	PRINT_DEBUG_FUNCTION_START("trampoline_free(*tld=%p, *slot=%p)", tld, slot);

	assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));

	slot->next = tld->trampoline;
	tld->trampoline = slot;

	PRINT_DEBUG_FUNCTION_END(" ");
}

