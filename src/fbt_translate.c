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
#include "fbt_private_datatypes.h"
#include "fbt_asm_macros.h"
#include "fbt_translate.h"
#include "fbt_disassemble.h"
#include "fbt_trampoline.h"
#include "fbt_actions.h"
#include "fbt_mem_alloc.h"
#include "fbt_libc.h"

#include "fbt_debug.h"
#include "fbt_statistic.h"

#if defined(FBT_INLINE_CALLS)
#define FBT_INLINE_MAX_LENGTH 64
#endif

#include "fbt_mem_protection.h"

// function prototypes
static unsigned int check_inline(translate_struct_t *ts);
static void check_transl_allowed(void* tu_address, struct mem_info *info);

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

#if defined(DEBUG)
    cache_free_list_t *code_block;
    code_block = tld->cache_list;
    /* make sure that we don't translate translated code */
    while(code_block!=NULL) {
	if ((tu_address >= code_block->cache_block) && (tu_address <= code_block->cache_end)) {
            llprintf("Translating translated code: %p (%p - %p (%p))\n", tu_address, code_block->cache_block, code_block->cache_end, code_block);
	}
	code_block = code_block -> next;
    }
#endif

    /*
     * Check if the memory address to translate lies in an executable
     * section of a loaded library or the executable itself. We only allow
     * execution if this is the case.
     */

    struct mem_info curr_section;
    check_transl_allowed(tu_address, &curr_section);


    /* we need to translate TU, add to tcache index, jump to CCF */
    finalize_tu_t tu_state = tu_neutral;

    int bytes_translated = 0;

    ts->next_instr = (unsigned char*)tu_address;

#if defined(FBT_ALIGN_FUNCTIONS)
    /* check if the newly translated block is a function (starts with puhsl %ebp?) */
    if (*(unsigned char*)tu_address==0x55) { /* pushl ebp */
	unsigned char *addr = ts->transl_instr;
	int i, nrbytes = (16-((unsigned long)addr)%16)&0xf; /* 0..15 bytes needed for padding */
	for (i=0; i<nrbytes; i++) {
	    /* TODO: possible opt: multibyte opt instrs */
	    *addr++=0x90; /* nop */
	}
	ts->transl_instr = addr;
	bytes_translated+=nrbytes;
    }
#endif
    
#if defined(FBT_RET_CACHE)
    /* check if this the current location is right after a call. if so: add our prelude */
    unsigned char *addr = ts->transl_instr;
    ret_cache_entry_t *calls = ts->tld->untrans_calls;
    ret_cache_entry_t *prev = NULL;
    while (calls!=NULL) {
	if (calls->ret_addr == tu_address) {
	    /* we have a hit! */

	    calls->retprelude = addr;
	    addr = write_ret_trampoline(ts, calls->backpatch, addr, calls->ret_addr);
	    bytes_translated+=((int32_t)addr - (int32_t)ts->transl_instr);
	    ts->transl_instr = addr;

	    /* now lets remove that call from the list */
	    if (prev == NULL) {
		ts->tld->untrans_calls = calls->next;
	    } else {
		prev->next = calls->next;
	    }
	    // now enqueue in translated calls
	    calls->next = ts->tld->trans_calls;
	    ts->tld->trans_calls = calls;
	    break;
	}
	prev = calls;
	calls = calls->next;
    }
#endif
    
    /* we need to add the TU to tcache index */
    tcache_put(tld, tu_address);

    /* look up address in translation cache index */
    void *ccf_address = ts->transl_instr;


    /* we translate as long as we
     *  - stay in the limit (MAX_CCF_SIZE)
     *  - or if we have an open TU (could happen if we are translating a call or so)
     *  - or we are inlining, then we first need to finish it and spill all the frames!
     *     we need to check for inlining here because inlining could overrun the MAX_CCF_SIZE
     *     (this can happen if we translate short instructions in the inlined code to longer instructions in the
     *      translated code - but thats what the guard is there for!)
     */
#ifdef FBT_INLINE_CALLS
    while (((bytes_translated < MAX_CCF_SIZE) && (tu_neutral == tu_state)) || (tu_open == tu_state) || (ts->inlined_frames!=NULL)) {
#else
	while (((bytes_translated < MAX_CCF_SIZE) && (tu_neutral == tu_state)) || (tu_open == tu_state)) {
#endif /* FBT_INLINE_CALLS */
	    /* translate an instruction */

	    /*
	     * Check if we are still within the boundaries of the current section.
	     * Otherwise, do a complete check of the address to translate and
	     * update the current section information.
	     */
#ifdef SECU_ENFORCE_NX
	    if (((void*) ts->next_instr < curr_section.node.addr_begin)
		|| ((void*) ts->next_instr >= curr_section.node.addr_end)) {
		check_transl_allowed(ts->next_instr, &curr_section);
	    }
#endif /* SECU_ENFORCE_NX */

#if defined(FBT_CHECK_TRANSLATED)
	    // current instr already translated?
	    unsigned char *dst=NULL;
	    if (((dst=tcache_find(tld, ts->next_instr))!=NULL) && dst!=ts->transl_instr && ts->inlined_frames==NULL) {
		JMP_REL32(ts->transl_instr, (int32_t)dst);
		tu_state = tu_close;
		break;
	    }
#endif
	    
	    disasm_instr(ts);

	    unsigned char *old_transl_instr = ts->transl_instr;
#ifdef DEBUG
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
		if ((function_length = check_inline(ts)) && (bytes_translated+function_length<MAX_CCF_SIZE)) {
		    // if yes, then we construct the static call frame (with return address)
		    // we need this list to be able to inline recursive calls
		    struct inline_struct *myinline = (struct inline_struct*)fbt_smalloc(tld, sizeof(struct inline_struct));
		    myinline->ret_addr = ts->next_instr;
		    myinline->next = ts->inlined_frames;
		    ts->inlined_frames = myinline;
		}
	    }
#endif
	    /* call the action specified for this instruction */
	    tu_state = ts->action(ts);
	    bytes_translated+=(ts->transl_instr - old_transl_instr);

#if defined(FBT_STATISTIC)
	    fbt_nr_translated_instr++;
#endif
	    PRINT_DEBUG("orig_ins_addr: %p", ts->cur_instr);
	    PRINT_DEBUG("orig_ins:  %s", printnbytes(old_cur_instr, old_next_instr - old_cur_instr));
	    PRINT_DEBUG("transl_insn_addr: %p", old_transl_instr);
	    PRINT_DEBUG("trans_ins: %s", printnbytes(old_transl_instr, ts->transl_instr - old_transl_instr));
	    PRINT_DEBUG("tu_state: %d\n", tu_state); /* debug */
	    DUMP_CODE(ts, (old_next_instr_dump - ts->cur_instr), (ts->transl_instr - old_transl_instr_dump));
	    BREAK_ON_TRANSL(ts->cur_instr, old_transl_instr_dump);
	}

	/* if the TU was finished because the number of instructions hit the limit, we have to add glue code to the TU */
	if (tu_close != tu_state) {
	    if (tu_state != tu_close_glue) {
		PRINT_DEBUG("finishing TU because instruction limit was hit, invoking translation function on %p", ts->next_instr);
	    } else {
		PRINT_DEBUG("closing TU upon request, invoking translation function on %p", ts->next_instr);
	    }
	    /* takes care of the flag register! */
	    /*
	     * write: jump to trampoline that does chaining if next block needs to be translated
	     * or does fallthrough otherwise
	     */
	    JMP_REL32(ts->transl_instr, (int32_t)trampoline_put_chaining(ts->tld, (void*)ts->next_instr, ts->transl_instr));
	}

	/* make sure that we always stay in the limits, even if we overwrite the MAX_CCF_SIZE due to some optimizations */
	assert(bytes_translated < CCF_GUARD);
	assert((void*)(ts->transl_instr) < (void*)(tld->tcache_end+CCF_GUARD));

	PRINT_DEBUG_FUNCTION_END("-> %p,   next_tu=%p (len: %d)", ccf_address, ts->next_instr, bytes_translated);
	return ccf_address;
}

/**
 * checks if the instruction at tu_address should be translated.
 * If tu_address is in an elf section that is marked as executable, this
 * function returns the address of the last byte that belongs to this section.
 * Otherwise, an error message is printed and the program is terminated.
 * @param tu_address the address that shall be checked
 * @return the address of the last byte in the current section
 */
static void check_transl_allowed(void* tu_address, struct mem_info *info)
{
#if defined(SECU_ENFORCE_NX) || defined(INFO_OUTPUT)
    if (fbt_memprotect_execquery(tu_address)) {
        if (!fbt_memprotect_info(tu_address, info)) {
            fbt_suicide_str("Unknown error in check_transl_allowed (check_transl_allowed: fbt_translate.c).\n");
        }
    } else {
        if (fbt_memprotect_info(tu_address, info)) {
            llprintf("Tried to translate code at address %p in %s, section %s, "
		     "which is not marked as executable.\n",
		     tu_address,
		     info->obj_name,
		     info->sec_name);
        } else {
            llprintf("Tried to translate code at address %p, which was determined"
		     " not to be in a section of any loaded shared library or the"
		     " executable.\n",
		     tu_address);
        }
#ifdef SECU_ENFORCE_NX
#ifdef SLEEP_ON_FAIL
        sleep(10);
#endif /* SLEEP_ON_FAIL */
        fbt_suicide_str("Exiting Program! If you believe this occurs in error, disable "
                "the -DSECU_ENFORCE_NX CFLAG in the secuBT Makefile (check_transl_allowed: fbt_translate.c).\n");
#endif /* SECU_ENFORCE_NX */
    }
#endif

}

#if defined(FBT_INLINE_CALLS)
/**
 * Checks if a function is inlinable. Starting from a call we search for a ret
 * if we only find suitable (inlinable) instructions we tell the callee that this
 * function is inlinable, otherwise we fail.
 */
static unsigned int check_inline(translate_struct_t *ts)
{
    /* static translate struct for internal use */
    translate_struct_t myts;
    fbt_memcpy(&myts, ts, sizeof(translate_struct_t));
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
                // TODO continue here?
                unsigned int rec_call;
                if ((rec_call=check_inline(&myts))) {
                    function_length+=rec_call;
                    continue;
                } else {
                    return 0;
                }
            }
            // some action we don't know jack about - let's stop inlining and bail out
            return 0;
        }
    }
    // if the function is too long we might hit that
    return 0;
}
#endif
