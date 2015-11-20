/**
 * This file defines all the in assembly implemented functions that are
 * used mostly in the translate actions and when the bt is started or stopped
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
 *   Antonio Barresi <abarresi@student.ethz.ch>
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
#ifndef FBT_ASM_FUNCTIONS_H
#define FBT_ASM_FUNCTIONS_H

/**
 * read the return instruction pointer.
 * Reads the return instruction pointer of the *calling* function and returns it.
 * @return the return instruction pointer
 */
/*extern*/ void *read_rip();


/**
 * change the return instruction pointer.
 * Change the return instruction pointer such that after the *calling* function
 * returns, the execution will continue at the supplied address
 * @param rip_address the address where the return instruction pointer shall point to
 */
/*extern*/
void change_rip(void* rip_address);

/*extern*/
void set_return_to_translated_asm(void);

/*extern*/
void set_return_to_translated_asm_nostackframe(void);

/**
 * makes an jump to 'original_addr' in the translated code
 * @param original_addr pointer into the original code. will be translated if not already done so
 */
/*extern*/
void ind_jump(thread_local_data_t *tld, void *original_addr);

/**
 * makes an jump to 'original_addr' in the translated code
 * Used in signal handling trampolines to get to a tld
 * @param original_addr pointer into the original code. will be translated if not already done so
 */
/*extern*/
void ind_jump_signal(thread_local_data_t *tld, void *original_addr);

/**
 * makes an jump to 'original_addr' in the translated code and removes 'num_bytes' from the stack
 * @param num_bytes the number of bytes which should be removed from the stack by this function
 * @param original_addr pointer into the original code. will be translated if not already done so.
 */
/*extern*/
void ind_jump_remove(thread_local_data_t *tld, unsigned int num_bytes, void *original_addr);


/**
 * translate the 'tu_address' and executes it after translating it.
 * Also frees the trampoline slot which was used to jump to this instruction
 * @param tld the pointer to the thread local data
 * @param original_addr the address to jump to
 * @param origin pointer to the offset of the jump to the trampoline
 */
/*extern*/
void ind_jump_backpatch(thread_local_data_t *tld, void *original_addr, void *origin);

/**
 * used for FBT_RET_STACK optimization.
 * translate the 'tu_address' and executes it after translating it.
 * Also frees the trampoline slot which was used to jump to this instruction
 * @param tld the pointer to the thread local data
 * @param original_addr the address to jump to
 * @param origin pointer to the offset of the jump to the trampoline
 */
/*extern*/
void ind_jump_backpatch_abs(thread_local_data_t *tld, void *original_addr, void *origin);

/**
 * used for chaining with fallthrough optimization where possible.
 * The jump to the trampoline is either backpatched to point to the translated code if a translation
 * of the next code block already existed, or overwritten by the next code block if it still had
 * to be translated.
 * Also frees the trampoline slot which was used to jump to this instruction.
 * @see ind_jump_backpatch
 * @see put_trampoline_chaining
 * @param tld the pointer to the thread local data
 * @param original_addr the address to jump to
 * @param origin pointer to the offset of the jump to the trampoline
 */
/*extern*/
void ind_jump_chaining(thread_local_data_t *tld, void *original_addr, void *origin);

/**
 * makes an jump to 'original_addr' in the translated code
 * Uses the ind_jump backpatch routine for jmp r/m32 or call r/m32 optimizations
 * @param original_addr pointer into the original code. will be translated if not already done so
 */
/*extern*/
void fix_ind_call_prediction(void *cmptarget, thread_local_data_t *tld, void *original_addr);
/*extern*/
void fix_ind_call_prediction_fast(thread_local_data_t *tld, void *original_addr);

/**
 * makes a jump to 'original_addr' in the translated code
 * The indjump prediction fixup routine.
 * @param original_addr pointer into the original code. will be translated if not already done so
 */
/*extern*/
void fix_ind_jmp_prediction(void *cmptarget, thread_local_data_t *tld, void *original_addr);
/*extern*/
void tcache_find_fast(thread_local_data_t *tld, void *original_addr); // just a prototype definition so tcache_find_fast() can be used in fbt_actions.c

/**
 * returns to 'original_addr' in the translated code
 * @param eflags the saved EFLAGS register which should be restored by this function
 * @param original_addr pointer into the original code. will be translated if not already done so.
  */
/*extern*/
void fix_ret_predict(void *cmptarget, thread_local_data_t *tld);

/**
 * returns to 'original_addr' in the translated code
 * @param eflags the saved EFLAGS register which should be restored by this function
 * @param original_addr pointer into the original code. will be translated if not already done so.
  */
/*extern*/
void ret_predict(thread_local_data_t *tld, unsigned int eflags, void *original_addr);

/**
 * returns to 'original_addr' in the translated code and removes 'num_bytes' from the stack
 * @param num_bytes the number of bytes which should be removed from the stack by this function
 * @param eflags the saved EFLAGS register which should be restored by this function
 * @param original_addr pointer into the original code. will be translated if not already done so.
  */
/*extern*/
void ret_predict_remove(thread_local_data_t *tld, unsigned int num_bytes, unsigned int eflags, void *original_addr);

/**
 * called if FBT_RET_STACK shadow stack gets out of synch with real stack.
 * Tries to resynch the shadow stack by discarding elements or calling tcache_find.
   */
/*extern*/
void ret_stack_synchronize(thread_local_data_t *tld);

/**
 * called to prepare a sys enter
 * This function expects the following to be on the stack:
 *   eip
 *   eax
 *   ecx
 *   edx
 */
/*extern*/
void prepare_sysenter(thread_local_data_t *tld);

/*extern*/
void authorize_sysenter(thread_local_data_t *tld);

/*extern*/
void authorize_int0x80();

/**
 * print message on successful transaction commit
 */
/*extern*/
void end_transaction();

/**
 * Returns to untranslated code and cleans memory allocated by the BT for
 * this thread.
 */
/*extern*/
void end_transaction_clean(void *tld);


void *dlsym_handler(void *handle, const char *symbol);

void *dlvsym_handler(void *handle, char *symbol, char *version);

void fbt_do_nothing();

#endif /* FBT_ASM_FUNCTIONS_H */
