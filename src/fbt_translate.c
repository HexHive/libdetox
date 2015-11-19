/**
 * This modules translates one code region and puts the result into the tcache
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
#include <stdint.h>
#include <pthread.h>
#include <assert.h>

#include "fbt_private_datatypes.h"
#include "fbt_asm_functions.h"
#include "fbt_translate.h"
#include "fbt_disassemble.h"
#include "fbt_tcache.h"
#include "fbt_trampoline.h"
#include "fbt_actions.h"

#include "fbt_debug.h"

#include "fbt_statistic.h"

#include "fbt_asm_macros.h"

#if defined(FBT_INLINE_CALLS)
#define FBT_INLINE_MAX_LENGTH 64
#endif

#ifdef DISABLE_ME
/* these functions are implemented in the fbt_asm_functions.S and replicated where needed */

/**
 * Push caller-save registers and EFLAGS register onto the stack.
 * We need to do this if a function where the call to that function is inserted, such that the caller
 * does not save these registers.
 */
#define push_reg()		\
	do {			\
	__asm__("pushfl");	\
	__asm__("pushl %eax");	\
 	__asm__("pushl %ecx");	\
	__asm__("pushl %edx");	\
	} while (0)


/**
 * Pop caller-save registers and EFLAGS register from stack.
 * The converse thing to push_reg().
 */
#define pop_reg()		\
	do {			\
	__asm__("popl %edx");	\
	__asm__("popl %ecx");	\
	__asm__("popl %eax");	\
	__asm__("popfl");	\
	} while (0)
#endif /* DISABLE_ME */

unsigned int check_inline(translate_struct_t *ts);

/**
 * translates a translation unit without jumping to the translated code.
 * If the translation unit (TU) has already been translated and is in the translation cache, the
 * function returns immediately. Otherwise, the TU is translated first and stored in the
 * translation cache.
 * @param tu_address the address where the TU begins
 * @return pointer to the CCF that corresponds to this TU
 */
void *translate_noexecute(thread_local_data_t *tld, void *tu_address)
{
	PRINT_DEBUG_FUNCTION_START("translate_noexecute(*tld=%p, *tu_address=%p)", tld, tu_address);

	assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));

	translate_struct_t *ts = &(tld->ts);

	/*
	 *check if this address has already been translated. If this is the case
	 * jump to the translated code. Else, translate the code at this address first.
	 */

	assert(tcache_find(tld, tu_address)==0); /* if we call this function, we must be sure that the adress was not already translated */

	/* we need to translate TU, add to tcache index, jump to CCF */
	finalize_tu_t tu_state = tu_neutral;

	int insn_translated = 0;

	ts->next_instr = (unsigned char*)tu_address;

	/* we need to add the TU to tcache index */
	tcache_put(tld, tu_address);
	
	/* look up address in translation cache index */
	void *ccf_address = ts->transl_instr;

	while (((insn_translated < MAX_CCF_SIZE) && (tu_neutral == tu_state)) || (tu_open == tu_state)) {
// 		PRINT_DEBUG("tu_insn_addr:  %#x", tu_insn_addr); /* debug */
// 		PRINT_DEBUG("ccf_insn_addr: %#x", ccf_insn_addr); /* debug */
		/* translate an instruction */
		disasm_instr(ts);

#ifdef INFO
		unsigned char *old_transl_instr = ts->transl_instr;
		unsigned char *old_cur_instr = ts->cur_instr;
		unsigned char *old_next_instr = ts->next_instr;
#endif
#ifdef DUMP_GENERATED_CODE
		unsigned char *old_transl_instr_dump = ts->transl_instr;
		unsigned char *old_next_instr_dump = ts->next_instr;
#endif

#if defined(FBT_INLINE_CALLS)
		/* if the current instruction is a call, then we check if it is inlinable */
		if (ts->action == action_call) {
		    unsigned int function_length;
		    // inlinable ?
		    if ((function_length = check_inline(ts))) {
			// if yes, then we construct the static call frame (with return address)
			// we need this list to be able to inline recursive calls
			struct inline_struct *myinline = (struct inline_struct*)malloc(sizeof(struct inline_struct));
			myinline->ret_addr = ts->next_instr;
			myinline->next = ts->inlined_frames;
			ts->inlined_frames = myinline;
		    }
		}
#endif
		/* call the action specified for this instruction */
		tu_state = ts->action(ts);
		insn_translated++;

#if defined(FBT_STATISTIC)
		fbt_nr_translated_instr++;
#endif
		PRINT_INFO("orig_ins_addr: %p", ts->cur_instr);
		PRINT_DEBUG("orig_ins:  %s", printnbytes(old_cur_instr, old_next_instr - old_cur_instr));
		PRINT_INFO("transl_insn_addr: %p", old_transl_instr);
		PRINT_DEBUG("trans_ins: %s", printnbytes(old_transl_instr, ts->transl_instr - old_transl_instr));
		PRINT_INFO("tu_state: %d\n", tu_state); /* debug */
		DUMP_CODE_BOTH(ts->cur_instr, old_next_instr_dump - ts->cur_instr,
			  old_transl_instr_dump, ts->transl_instr - old_transl_instr_dump);
		BREAK_ON_TRANSL(ts->cur_instr, old_transl_instr_dump);
	}

	/* if the TU was finished because the number of instructions hit the limit, we have to add glue code to the TU */
	if (tu_neutral == tu_state || tu_close_glue == tu_state) {
   	        if (tu_state == tu_neutral) {
		  PRINT_DEBUG("finishing TU because instruction limit was hit, invoking translation function on %p", ts->next_instr);
		} else {
		  PRINT_DEBUG("closing TU upon request, invoking translation function on %p", ts->next_instr);
		}
		
		/* 
		 * write: jump to trampoline that does chaining if next block needs to be translated 
		 * or does fallthrough otherwise
		 */
		JMP_REL32(ts->transl_instr, (int32_t)trampoline_put_chaining(ts->tld, (void*)ts->next_instr, ts->transl_instr));
	}

	
	PRINT_DEBUG_FUNCTION_END("-> %p,   next_tu=%p", ccf_address, ts->next_instr);
	return ccf_address;
}

#if defined(FBT_INLINE_CALLS)
/**
 * Checks if a function is inlinable. Starting from a call we search for a ret
 * if we only find suitable (inlinable) instructions we tell the callee that this
 * function is inlinable, otherwise we fail.
 */
unsigned int check_inline(translate_struct_t *ts)
{
    /* static translate struct for internal use */
    translate_struct_t myts;
    memcpy(&myts, ts, sizeof(translate_struct_t));
    unsigned int function_length = 0;

    /* extract relative call target from call. */
    /* we assume that this call will not have any prefixes */
    assert(!HAS_PREFIX(*(myts.cur_instr)));
    myts.next_instr = (unsigned char*)(*((int32_t*) (myts.cur_instr + 1)) + (int32_t) myts.cur_instr + 5);
    
    while (function_length < FBT_INLINE_MAX_LENGTH) {
	disasm_instr(&myts);
	function_length += (myts.next_instr - myts.cur_instr);
	
	if (myts.action!=action_copy) {
	    // if we found a ret, then we are done, let's return the length of this function (in bytes)
	    if (myts.action==action_ret) return function_length;
	    // if we have a call, then we might go into recursion and inline this call as well
	    if (myts.action==action_call) {
		if (check_inline(&myts)) break; else return 0;
	    }
	    // some action we don't know jack about - let's stop inlining and bail out
	    return 0;
	}
    }
    // if the function is too long we might hit that
    return 0;
}
#endif
