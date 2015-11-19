/**
 * This file contains all the actions that are used in the bt to
 * translate each instruction.
 * Each action takes an instruction and 'translates' it into the code buffer
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/mman.h>

#include "fbt_private_datatypes.h"
#include "fbt_translate.h"
#include "fbt_actions.h"
#include "fbt_asm_functions.h"
#include "fbt_tcache.h"
#include "fbt_trampoline.h"
#include "fbt_mem_alloc.h"

#include "fbt_debug.h"

#include "fbt_statistic.h"

#include "fbt_asm_macros.h"

#ifdef SECU_MPROTECT_IDS
#include "fbt_mem_protection.h"
#endif
extern void (*fbt_commit_address)();

extern void print_eip();
/**
 * Copies the current instruction verbatim from the TU to the CCF
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_copy(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char* transl_addr = ts->transl_instr;
    int length = ts->next_instr - ts->cur_instr;

    PRINT_DEBUG_FUNCTION_START("action_copy(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
    /* copy instruction verbatim to translated version */
    memcpy(transl_addr, addr, length);

    PRINT_DEBUG_FUNCTION_END("-> neutral, transl_length=%i", length);
    ts->transl_instr += length;
    if (*(ts->cur_instr) == 0xcc || *(ts->cur_instr) == 0xcd || *(ts->cur_instr) == 0xce) {
	/* 
	 * we just copied an interrupt
	 * 0xcc - int3 (debug)
	 * 0xcd - int imm8
	 * 0xce - into (int 4) if overflow flag is set to 1
	 * because we might execute a kernel routine (which might not return or
	 * might do something funny to our stack) we finish the TU here and
	 * issue some glue code to restart the translation if we resume after the
	 * int itself
	 */
	PRINT_DEBUG("Encountered an intterupt - closing TU with some glue code\n");
	return tu_close_glue;
    }
    return tu_neutral;
}

#if defined(FBT_RET_STACK) && defined(FBT_RET_STACK_OVERFLOW_CHECK)
static void ret_stack_handle_overflow()
{
    PRINT_ERROR("Translated return address stack overflow!");
    exit(1);
}
#endif

/**
 * Handles call instruction to relative memory address.
 * Copies the call instruction, but changes the target address such that the translated
 * version of the function is called.
 * If the function is not already translated, the function translate_function is invoked.
 * Calls to fbt_commit_transaction are replaced by calls to end_transaction, and the translation
 * routine will not be invoked after such a call.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_call(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char* transl_addr = ts->transl_instr;
    int length = ts->next_instr - ts->cur_instr;

    PRINT_DEBUG_FUNCTION_START("action_call(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
#if defined(FBT_STATISTIC)
    fbt_nr_translated_call++;
#endif

    /* total length of a call we handle must be 5, otherwise we have prefixes and such in our stream */
    assert(length==5 && !HAS_PREFIX(*addr));

    /* our opcode should be 0xE8, a near relative call */

    /*
     * read call argument (32bit immediate) and add EIP (EIP = addr + length) to argument
     * --> absolute target address
     */
    int32_t call_target = *((int32_t*) (addr + 1)) + (int32_t) addr + length;
    PRINT_DEBUG("original call_target: %p", (void*)call_target);

    /* write: push original EIP (we have to do this either way) */
    PUSHL_IMM32(transl_addr, ((int32_t)addr) + length);
	
#ifdef FBT_RET_STACK
    /*		(pushl eip)				# see before #if
     *		pushl %eax
     *		subl $8, (&tld->tos)
     *		movl (&tld->tos), %eax
     *		movl "trampoline_put", (%eax) # trampoline first time, later translated absolute call target
     * 		movl orig_return_addr, 4(%eax)
     * 
     *#ifdef FBT_RET_STACK_OVERFLOW_CHECK
     *		cmpl %eax, (&tld->stack)
     *		jne +5
     *		call stack_overflow
     *	+5:
     *#endif
     *		popl %eax
     * 		([jmp ...])				# see after #endif 
     */

    PUSHL_EAX(transl_addr);
    SUBL_IMM8_MEM32(transl_addr, 0x8, (int32_t)&(ts->tld->translated_call_stack_tos));
    MOVL_MEM32_EAX(transl_addr, (int32_t)&(ts->tld->translated_call_stack_tos));
    /* movl imm32, (%eax) */
    MOVL_IMM32_RM32(transl_addr, (int32_t)trampoline_put_abs(ts->tld, (void*)ts->next_instr, transl_addr), 0x00);
    /* movl imm32, 0x4(%eax) */
    MOVL_IMM32_RM32I8(transl_addr, (int32_t)ts->next_instr, 0x40, 0x04);

#ifdef FBT_RET_STACK_OVERFLOW_CHECK
    /* cmpl %eax, mem32 */
    CMPL_R32_RM32I32(transl_addr, 0x05, (int32_t)&(ts->tld->translated_call_stack));
    JNE_I8(transl_addr, 0x05);
    CALL_REL32(transl_addr, (int32_t)&ret_stack_handle_overflow);
#endif /* FBT_RET_STACK_OVERFLOW_CHECK */

    POPL_EAX(transl_addr);

#endif /* FBT_RET_STACK */

#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_call);
#endif

#if defined(FBT_INLINE_CALLS)
    if (ts->inlined_frames!=NULL) {
        /* are we inlining the current function? if so, then we bail out and
            * let the translate function handle the recursion */

        PRINT_DEBUG_FUNCTION_END("-> open, inlined transl_length=%i", transl_addr - ts->transl_instr);
        ts->transl_instr = transl_addr;
        /*
            * No need to actually call the function.
            * Simply change the next instr pointer to the first instr of the function.
            * This will put the body of the function right as the next instr in the translated code
            */
        ts->next_instr = (unsigned char*) call_target;

        return tu_open;
    }
#endif
    
    /* check if target is already translated; if not, do so now */
    void *transl_target = tcache_find(ts->tld, (void*) call_target);
    if (transl_target == NULL) {
        /* we still have to translate the call target */
        PRINT_DEBUG_FUNCTION_END("-> open, transl_length=%i", transl_addr-ts->transl_instr);
        ts->transl_instr = transl_addr;
        /*
        * No need to actually call the function.
        * Simply change the next instr pointer to the first instr of the function.
        * This will put the body of the function right as the next instr in the translated code
        */
        ts->next_instr = (unsigned char*) call_target;

        /* put the target into the tcache so later calls can use the translated code */
        tcache_put(ts->tld, (void*)call_target);

        return tu_open;
    }

    /* target is already translated */
    PRINT_DEBUG("translated call_target: %p", transl_target);

#if defined(FBT_RET_CACHE)
    unsigned int retcache_offset = (((int32_t)ts->next_instr))&0xff; /* lowest byte */
    /**
     * MOVL ret_cache_trampo, tld->retcache+retcache_offset
     * Whereas the offset is determined by the 'hash' function in the ret (movzbl)
     */
    MOVL_IMM32_IMM32RM32(transl_addr, 0x05, (int32_t)(ts->tld->retcache_jump), (int32_t)(ts->tld->retcache+retcache_offset));

    void *backpatch = (void*)((int32_t)transl_addr-4);
    void *retaddr = (void*)((int32_t)ts->next_instr);
    if (tcache_find(ts->tld, ts->next_instr)!=0) {
	/* so this call might have already been translated */
	/* let's check if we already cached a return trampoline and can reuse it! */
	ret_cache_entry_t *trans = ts->tld->trans_calls;
	while (trans!=NULL) {
	    if (trans->ret_addr == retaddr) {
		// yes we can!
		*(int32_t*)backpatch = (int32_t)trans->retprelude;
		break;
	    }
	    trans = trans->next;
	}
	// there was no already translated prelude we could use, so make a new one!
	if (trans==NULL) {
	    /* write: jump instruction to translated target */
	    JMP_REL32(transl_addr, (int32_t)transl_target);
	    
	    /* enqueue this location in the _translated_ calls that are handled in the translate module (for the prelude) */
	    ret_cache_entry_t *newcall = (ret_cache_entry_t*)fbt_smalloc(ts->tld, sizeof(ret_cache_entry_t));
	    newcall->next = ts->tld->trans_calls;
	    newcall->backpatch = backpatch;
	    newcall->ret_addr = retaddr;
	    newcall->retprelude = transl_addr;
	    ts->tld->trans_calls = newcall;

	    // our return target is already translated, so we just fix a jmp to it
	    transl_addr = write_ret_trampoline(ts, backpatch, transl_addr, retaddr);
	    /* set the target to the next location after the return of the call (so we fix this cache) */
	    transl_target = tcache_find(ts->tld, ts->next_instr);
	}
    } else {
	/* enqueue this location in the untranslated calls that are handled in the translate module (for the prelude) */
	ret_cache_entry_t *newcall = (ret_cache_entry_t*)fbt_smalloc(ts->tld, sizeof(ret_cache_entry_t));
	newcall->next = ts->tld->untrans_calls;
	newcall->backpatch = backpatch;
	newcall->ret_addr = retaddr;
	ts->tld->untrans_calls = newcall;
    }
#endif
    
    /* write: jump instruction to translated target */
    JMP_REL32(transl_addr, (int32_t)transl_target);

    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_close;
}

/**
 * Prepares near indirect calls. Only pushes the arguments to the ind_jump(_*) routine.
 * This separation is needed in libSTM.
 */
void prepare_action_call_indirect(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
    unsigned char* transl_addr = ts->transl_instr;
    int length = ts->next_instr - ts->cur_instr;
    int prefix_length = 0;


    /*
     * check for prefixes:
     * we handle only segment override prefixes, all others produce an error
     */
    while (HAS_PREFIX(*addr)) {
	/* we don't handle 16bit calls and strange prefixes */
	assert(*addr!=PREFIX_LOCK && *addr!=PREFIX_REPNEZ && *addr!=PREFIX_MISC && *addr!=PREFIX_ADDR_SZ_OVR && *addr!=PREFIX_OP_SZ_OVR && *addr!=PREFIX_SIMD_EXT); 
	/* copy the freaking prefix! */
	PRINT_DEBUG("Copying prefix: 0x%x\n", *addr);
	*transl_addr++ = *addr++;
	
	prefix_length++;
    }

    /* write: push indirect target */
    *transl_addr++ = 0xFF;

    /*
     * 0xFF expects a ModR/M byte following the opcode
     * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
     * so we copy the ModR/M byte, but modify the opcode extension to 110
     */
    *transl_addr++ = (*(first_byte_after_opcode) & 0xC7) | 0x30;

    /* if there follows a SIB byte and/or displacement, copy this to the CCF */
    if ((length - prefix_length) > 2) {
	memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 - prefix_length);
	transl_addr += length - 2 - prefix_length;
	
	
	/**
	 * DANGER: we have a SIB replacement
	 * if we have e.g. call *0x44(%esp) or call *(%esp) then we have a
	 * problem, because we change %esp by pushing the original EIP
	 * onto it beforehand!
	 * So we must introduce an additional displacement of 4 bytes to
	 * get the right offset
	 */
	char modrm = (*(first_byte_after_opcode) & 0xC7) | 0x30;
	char sib = *(first_byte_after_opcode+1);
		
	if (((modrm&0x7)==4) && ((sib&0x7)==4)) {
	    /* ModR/M SIB replacement (R/M==100) */
	    /* SIB Byte (Base) == 100 -> ESP */
	    /* if we do not need ESP then we are lucky and can continue,
	       otherwise, we are screwed and have to take
	       counter measures
	    */
	    PRINT_DEBUG("We have a ModR/M and maybe SIB that uses the ESP register - we need to fix this mess!");
	    /* we use ESP but not with an 8bit displacement!
	       either the SIB is scaled or we have no/wrong
	       displacement in the modrm
	       Maybe we can latter add 4 special handling for
	       4b displacement (change offset for displacement)
	       and no displacement (change to 1b displacement).
	       Currently ONLY 1b displacement is handled!
	    */
	    assert(((modrm>>6)&0x3)==1);
	    /* just change the following displacement and add 4bytes to it! */
	    *(transl_addr-1) = (*(transl_addr-1))+4;
	}
    }

    /* write: push pointer to the thread local data */
    PUSHL_IMM32(transl_addr, (int32_t) ts->tld);

    ts->transl_instr = transl_addr;
}

/**
 * Finishes near indirect calls. Calls the ind_jump(_*) routine.
 * Also does any target/return address lookup optimizations.
 * (Such optimizations should not use the target address of the original instruction!
 *  Use the prepared value on the stack instead.)
 * This is needed in libSTM.
 */
void finish_action_call_indirect(translate_struct_t *ts)
{
    unsigned char* transl_addr = ts->transl_instr;

#ifdef FBT_RET_STACK
    /*		(pushl eip)				# see before #if
     * 		(pushl ..call_target..)                 # see before #if
     * 		(pushl tld)				# see before #if
     *		pushl %eax
     *		subl $8, (&tld->tos)
     *		movl (&tld->tos), %eax
     *		movl "trampoline_put", (%eax)
     * 		movl orig_return_addr, 4(%eax)
     * 
     *#ifdef FBT_RET_STACK_OVERFLOW_CHECK
     *		cmpl %eax, (&tld->stack)
     *		jne +5
     *		call stack_overflow
     *	+5:
     *#endif
     *		popl %eax
     * 		(call ind_jump)				# see after #endif 
     */

    PUSHL_EAX(transl_addr);
    SUBL_IMM8_MEM32(transl_addr, 0x08, (int32_t)&(ts->tld->translated_call_stack_tos));
    MOVL_MEM32_EAX(transl_addr, (int32_t) &(ts->tld->translated_call_stack_tos));
    /* movl imm32, (%eax) */
    MOVL_IMM32_RM32(transl_addr, (int32_t)trampoline_put_abs(ts->tld, (void*)ts->next_instr, transl_addr), 0x00);
    /* movl imm32, 0x4(%eax) */
    MOVL_IMM32_RM32I8(transl_addr, (int32_t)ts->next_instr, 0x40, 0x04);

#ifdef FBT_RET_STACK_OVERFLOW_CHECK
    /* cmpl %eax, mem32 */
    CMPL_R32_RM32I32(transl_addr, 0x05, (int32_t)&(ts->tld->translated_call_stack));
    JNE_I8(transl_addr, 0x05);
    CALL_REL32(transl_addr, (int32_t)&ret_stack_handle_overflow);
#endif /* FBT_RET_STACK_OVERFLOW_CHECK */

    POPL_EAX(transl_addr);

#else
    CALL_REL32(transl_addr, (int32_t)&ind_jump);
#endif

    ts->transl_instr = transl_addr;
}

/**
 * Handles near indirect calls.
 * If the function is not already translated, the function translate_function is invoked.
 * Calls to fbt_commit_transaction are replaced by calls to end_transaction, and the translation
 * routine will not be invoked after such a call.
 * NOTE: This code does not handle indirect calls to fbt_commit_transaction.
 *   This means that a transaction cannot be closed by an indirect near call
 *   (A transaction MUST be closed by call fbt_commit_transaction)
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_call_indirect(translate_struct_t *ts)
{

    PRINT_DEBUG_FUNCTION_START("action_call_indirect(*addr=%p, *transl_addr=%p, length=%i)",
			       ts->cur_instr, ts->transl_instr, ts->next_instr - ts->cur_instr);

#if defined(FBT_STATISTIC)
    fbt_nr_translated_call_ind++;
    /* WARNING: this statement destroys the FLAGS */
    INCL_M64(ts->transl_instr, (int32_t)&fbt_nr_ind_calls);
#endif


    /* only translate ind calls that target a different segment or a specific memory address */
    unsigned char* transl_addr = ts->transl_instr;
    unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
    int length = ts->next_instr - ts->cur_instr;
    int prefix_length = 0;

    unsigned char *addr = ts->cur_instr;
    while (HAS_PREFIX(*addr)) {
	/* we don't handle 16bit calls and strange prefixes */
	assert(*addr!=PREFIX_LOCK && *addr!=PREFIX_REPNEZ && *addr!=PREFIX_MISC && *addr!=PREFIX_ADDR_SZ_OVR && *addr!=PREFIX_OP_SZ_OVR && *addr!=PREFIX_SIMD_EXT); 
	/* copy the freaking prefix! */
	PRINT_DEBUG("Copying prefix: 0x%x\n", *addr);
	//*transl_addr++ = *addr++;
	addr++;
	prefix_length++;
    }

    /* write: push original EIP */
    PUSHL_IMM32(transl_addr, (int32_t)ts->next_instr);
    PRINT_DEBUG("original eip: %p", addr + length);

#if defined(FBT_RET_CACHE)
    unsigned int retcache_offset = (((int32_t)ts->next_instr))&0xff; /* lowest byte */
    /**
     * MOVL ret_cache_trampo, tld->retcache+retcache_offset
     * Whereas the offset is determined by the 'hash' function in the ret (movzbl)
     */
    MOVL_IMM32_IMM32RM32(transl_addr, 0x05, (int32_t)(ts->tld->retcache_jump), (int32_t)(ts->tld->retcache+retcache_offset));
    
    void *backpatch = (void*)((int32_t)transl_addr-4);
    void *retaddr = (void*)((int32_t)ts->next_instr);
    int insert_retcache = 0;
    if (tcache_find(ts->tld, ts->next_instr)==0) {
	/* enqueue this location in the untranslated calls that are handled in the translate module (for the prelude) */
	ret_cache_entry_t *newcall = (ret_cache_entry_t*)fbt_smalloc(ts->tld, sizeof(ret_cache_entry_t));
	newcall->next = ts->tld->untrans_calls;
	newcall->backpatch = backpatch;
	newcall->ret_addr = retaddr;
	ts->tld->untrans_calls = newcall;
    } else {
	insert_retcache=1;
    }
#endif

    
#if defined(FBT_IND_CALL_PREDICTION)
    unsigned char *cmptarget, *jmptarget;
    unsigned char *cmptarget2, *jmptarget2;

    unsigned char modrm = *(((char*)addr)+1);
    //   prefix (syscall)     disp32             no esp reg in sib                   disp8(reg) (not ebp, esi edi) - TODO remove
    //if (HAS_PREFIX(*addr) || (modrm==0x15)  || (( (modrm&0xF)==4) && ((*(((char*)addr)+2)&0xF)!=4)) ) { //(((modrm>>4)==0x5) && (modrm!=0x54))) {
    if (HAS_PREFIX(*addr) ||  !(( (modrm&0xF)==4) && ((*(((char*)addr)+2)&0xF)==4)) ) {
    
#if defined(FBT_STATISTIC)
    fbt_nr_translated_call_ind_pred++;
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_calls_pred);
#endif

    /*  (pushl srcip)
     *	cmpl $AABBCCDD, xx(%src)	# predicted ret. addr. =?= actual ret. addr.
     *	je $transAABBCCDD
     *  cmpl $EEFF0011, xx(%src)	# second prediction
     *	je $transEEFF0011
     *	pushl $targetip
     *	pushl $tld
     *	pushl $addrOf(AABBCCDD)
     *	pushl $addrOf(EEFF0011)
     *	call fix_ind_call_prediction
     *	0x00000000 (nr of mispredictions)
     */
    
    /* first compare */
    memcpy(transl_addr, ts->cur_instr, prefix_length); /* copy prefixes */
    transl_addr+=prefix_length;
    *transl_addr++ = 0x81; /* cmpl r/m32, imm32 */
    *transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x38; /* use modrm, but modify opcode extension to 111 */
    /* if there follows SIB byte and/or displacement copy this to the CCF */
    if ((length-prefix_length) > 2) {
	memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 - prefix_length);
	transl_addr += length - 2 - prefix_length;
    }
    *((int32_t*) transl_addr) = 0x0;		/* to be patched. predicted target addr. */
    cmptarget = transl_addr;
    transl_addr += 4;

    JE_I32(transl_addr, 0x00); /* je (patched later) */
    jmptarget = transl_addr-4;
    
    
    /* second compare */
    memcpy(transl_addr, ts->cur_instr, prefix_length); /* copy prefixes */
    transl_addr+=prefix_length;
    *transl_addr++ = 0x81; /* cmpl r/m32, imm32 */
    *transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x38; /* use modrm, but modify opcode extension to 111 */
    /* if there follows SIB byte and/or displacement copy this to the CCF */
    if ((length-prefix_length) > 2) {
	memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 - prefix_length);
	transl_addr += length - 2 - prefix_length;
    }
    *((int32_t*) transl_addr) = 0x0;		/* to be patched. predicted 2nd target addr. */
    cmptarget2 = transl_addr;
    transl_addr += 4;

    JE_I32(transl_addr, 0x00); /* je (patched later) */
    jmptarget2 = transl_addr-4;
    
    /** MISS **/
    /**********/
    memcpy(transl_addr, ts->cur_instr, prefix_length); /* copy prefixes */
    transl_addr+=prefix_length;
    *transl_addr++ = 0xFF; /* write: push indirect target */
    *transl_addr++ = (*(first_byte_after_opcode) & 0xC7) | 0x30; /* use modrm, but modify opcode extension to 110 */
    /* if there follows a SIB byte and/or displacement, copy this to the CCF */
    if ((length - prefix_length) > 2) {
	memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 - prefix_length);
	transl_addr += length - 2 - prefix_length;
    }
    /* write: push pointer to the thread local data & call dislocate function*/
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    PUSHL_IMM32(transl_addr, (int32_t)cmptarget);
    PUSHL_IMM32(transl_addr, (int32_t)cmptarget2);
    /* if we don't have prefixes, then we might disable the prediction */
    CALL_REL32(transl_addr, (int32_t)&fix_ind_call_prediction);

    /* ind jump mispredict rate */
    *((int32_t*) transl_addr) = 0x0;		/* to be incremented */
    transl_addr += 4;

    ts->transl_instr = transl_addr;
    } else {
#endif
	/* if we don't have a syscall (segment prefix), then we can't use the predictor
	   but use a fast ind call dispatch */
#if defined(FBT_IND_CALL_FAST)
	/* this is a fast version of the ind call - handoptimized assembler code
	 * which does a fast lookup in the hashtable and dispatches if it hits
	 * otherwise it recovers to an indirect jump
	 */
	
	/**
	 * (pushl srcRIP)
	 * jmpl  tld->ind_call_trampoline
	 */

	/* write: push indirect target */
	memcpy(transl_addr, ts->cur_instr, prefix_length); /* copy prefixes */
	transl_addr+=prefix_length;
	*transl_addr++ = 0xFF;
	
	/*
	 * 0xFF expects a ModR/M byte following the opcode
	 * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	 * so we copy the ModR/M byte, but modify the opcode extension to 110
	 */
	*transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x30;
	
	/* if there follows a displacement copy this to the ccf */
	if ((length - prefix_length) > 2) {
	    memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 - prefix_length);
	    transl_addr += length - 2 - prefix_length;
	
	    
	    /**
	     * DANGER: we have a SIB replacement
	     * if we have e.g. call *0x44(%esp) or call *(%esp) then we have a
	     * problem, because we change %esp by pushing the original EIP
	     * onto it beforehand!
	     * So we must introduce an additional displacement of 4 bytes to
	     * get the right offset
	     */
	    char modrm = (*(first_byte_after_opcode) & 0xC7) | 0x30;
	    char sib = *(first_byte_after_opcode+1);
	    
	    if (((modrm&0x7)==4) && ((sib&0x7)==4)) {
		/* ModR/M SIB replacement (R/M==100) */
		/* SIB Byte (Base) == 100 -> ESP */
		/* if we do not need ESP then we are lucky and can continue,
		   otherwise, we are screwed and have to take
		   counter measures
		*/
		PRINT_DEBUG("We have a ModR/M and maybe SIB that uses the ESP register - we need to fix this mess!");
		/* we use ESP but not with an 8bit displacement!
		   either the SIB is scaled or we have no/wrong
		   displacement in the modrm
		   Maybe we can latter add 4 special handling for
		   4b displacement (change offset for displacement)
		   and no displacement (change to 1b displacement).
		   Currently ONLY 1b displacement is handled!
		*/
		assert(((modrm>>6)&0x3)==1);
		/* just change the following displacement and add 4bytes to it! */
		*(transl_addr-1) = (*(transl_addr-1))+4;
	    }
	}

	JMP_REL32(transl_addr, (int32_t)(ts->tld->ind_call_trampoline));

 	ts->transl_instr = transl_addr;
#else
	/* neither fast ind call nor predict ind call */
	prepare_action_call_indirect(ts);
	finish_action_call_indirect(ts);
#endif
#if defined(FBT_IND_CALL_PREDICTION)
    }
#endif

#if defined(FBT_RET_CACHE)
    if (insert_retcache == 1) {
	/* so this call might have already been translated */
	/* let's check if we already cached a return trampoline and can reuse it! */
	ret_cache_entry_t *trans = ts->tld->trans_calls;
	while (trans!=NULL) {
	    if (trans->ret_addr == retaddr) {
		// yes we can!
		*(int32_t*)backpatch = (int32_t)trans->retprelude;
		break;
	    }
	    trans = trans->next;
	}
	// there was no already translated prelude we could use, so make a new one!
	if (trans==NULL) {
	    transl_addr = ts->transl_instr;
	    
	    /* enqueue this location in the _translated_ calls that are handled in the translate module (for the prelude) */
	    ret_cache_entry_t *newcall = (ret_cache_entry_t*)fbt_smalloc(ts->tld, sizeof(ret_cache_entry_t));
	    newcall->next = ts->tld->trans_calls;
	    newcall->backpatch = backpatch;
	    newcall->ret_addr = retaddr;
	    newcall->retprelude = transl_addr;
	    ts->tld->trans_calls = newcall;

	    // our return target is already translated, so we just fix a jmp to it
	    transl_addr = write_ret_trampoline(ts, backpatch, transl_addr, retaddr);
	    /* set the target to the next location after the return of the call (so we fix this cache) */
	    void *transl_target = tcache_find(ts->tld, ts->next_instr);
	    
	    /* write: jump instruction to translated target */
	    JMP_REL32(transl_addr, (int32_t)transl_target);
	    
	    ts->transl_instr = transl_addr;

	}
	
    }
#endif

    PRINT_DEBUG_FUNCTION_END("-> close");
    return tu_close;
}


/**
 * This function handles a jump conditional (Jcc).
 * Short jumps are rewritten to near jumps, as we cannot guarantee that the offset for the translated
 * version can fit into 8 bit.<br />
 * JECXZ are handled separately (within this function), as there exists only a short jump version.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_jcc(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char* transl_addr = ts->transl_instr;
    int length = ts->next_instr - ts->cur_instr;


    PRINT_DEBUG_FUNCTION_START("action_jcc(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
#if defined(FBT_STATISTIC)
    fbt_nr_translated_jcc++;
#endif
    int32_t jump_target;
    int32_t fallthru_target;
    void *transl_target;
    assert(!HAS_PREFIX(*addr) && ((*addr==0x0F && length==6) || (length==2))); /* no prefixes handled */

    /* check if we have jecxz (jump if ecx register is zero) */
    if (0xE3 == *addr) {
	PRINT_DEBUG("processing jecxz");

	/*
	 * This is a little tricky. because jecxz has only a 8bit offset we can not jmp directly to the trampoline
	 * therefore two unconditional jumps are inserted. first the unconditional jmp for the fall through target
	 * and then the jmp for the original jump target. A jecxz inst is then used to jump over the jump of the
	 * fall through target if the CX register is zero
	 */

	/* calculate the jump targets */
	fallthru_target = ((int32_t)addr) + length;
	jump_target = *((char*) (addr + 1)) + fallthru_target;

	/* insert a jecxz to jump over the fall through jump if CX is 0 */
	JECXZ_I8(transl_addr, 0x05);

	/* write: jump to trampoline for fallthrough address */
	/* create trampoline if one is needed, otherwise lookup and go */
	transl_target = tcache_find(ts->tld, (void*)fallthru_target);
	if ( transl_target != NULL ) {
	    JMP_REL32(transl_addr, (int32_t)transl_target);
	} else {
	    JMP_REL32(transl_addr, (int32_t)trampoline_put(ts->tld, (void*)fallthru_target, transl_addr));
	}

	/* if we have an jecxz then the jump target comes second (switchted) 
	   but we can use the code from the normal jcc, so we don't need to copy
	   the jmp_rel32 and all */
	fallthru_target = jump_target;

    } else {
	int16_t jcc_type;
	if (*(unsigned char*) addr != 0x0F) {	/* find out if we have a one-byte or a two-byte opcode */
	    PRINT_DEBUG("processing one-byte jcc");
	    /* write two-byte jcc equivalent to the one-byte jcc */
	    /* the trick is: opcode + 0x10 = second byte of twobyte Jcc instruction */
	    jcc_type = 0x0F + ((*addr+0x10)<<8);

	    fallthru_target = ((int32_t)addr) + length;
	    jump_target = *((char*) (addr + 1)) + fallthru_target;

	} else {
	    PRINT_DEBUG("processing two-byte jcc");
	    /* write: copy of two-byte jcc */
	    jcc_type = *((int16_t*)addr);

	    fallthru_target = ((int32_t)addr) + length;
	    jump_target = *((int32_t*) (addr + 2)) + fallthru_target;

	}
	/* write: jump address to trampoline; create trampoline if one is needed, otherwise lookup and go */
	transl_target = tcache_find(ts->tld, (void*)jump_target);
	if ( transl_target != NULL ) {
	    JCC_2B(transl_addr, jcc_type, (int32_t)transl_target);
	} else {
	    JCC_2B(transl_addr, jcc_type, (int32_t)trampoline_put(ts->tld, (void*)jump_target, transl_addr));
	}

    }
    /* write: jump to trampoline for fallthrough address */
    transl_target = tcache_find(ts->tld, (void*)fallthru_target);
    if ( transl_target != NULL ) {
	JMP_REL32(transl_addr, (int32_t)transl_target);
    } else {
	JMP_REL32(transl_addr, (int32_t)trampoline_put_chaining(ts->tld, (void*)fallthru_target, transl_addr));
    }

    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_close;
}


/**
 * Handles relative jump instructions.
 * Rewrites relative jump instructions such that the jump is relative to the translated code.
 * Short jumps are rewritten to near jumps, as we cannot guarantee that the offset for the translated
 * version can fit into 8 bit.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_jmp(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char* transl_addr = ts->transl_instr;
    int length = ts->next_instr - ts->cur_instr;

    PRINT_DEBUG_FUNCTION_START("action_jmp(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
#if defined(FBT_STATISTIC)
    fbt_nr_translated_jmp++;
#endif

    /*
     * read call argument (either 8bit or 32bit offset) and add EIP (EIP = addr + length) to argument
     * --> absolute target address = addr + length + offset
     */

    assert(!HAS_PREFIX(*addr)); /* no prefixes allowed */

    int32_t jump_target=0;
    if (*addr == 0xE9) {
	/* 32bit offset */
	jump_target = *((int32_t*)(addr + 1)) + (int32_t)addr + length;
    } else {
	/* our argument is only an 8bit offset */
	jump_target = (int32_t)(*((char*)(addr + 1)) + addr + length);
    }
    
    PRINT_DEBUG("original jmp_target: %p", (void*)jump_target);

    /* check if the target is already translated; if it is not, do so now */
    void *transl_target = tcache_find(ts->tld, (void*)jump_target);
    if (transl_target == NULL) {
	/* we still have to translate the call target */
	PRINT_DEBUG_FUNCTION_END("-> open, transl_length=%i", transl_addr-ts->transl_instr);
	ts->transl_instr = transl_addr;

	/*
	 * no need to actually jump
	 * simply change the next instr pointer to the first instr of the function
	 * this will put the body of the function right as the next instr in the translated code
	 */
	ts->next_instr = (unsigned char*)jump_target;

	/* put the target into the tcache so later jumps can use the translated code */
	tcache_put(ts->tld, (void*)jump_target);

	return tu_open;
    }

    PRINT_DEBUG("translated jmp_target: %p", transl_target);

    /* write: jmp */
    JMP_REL32(transl_addr, (int32_t)transl_target);
	
    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_close;
}

/**
 * Finishes near indirect jmp. Only calls the ind_jump routine.
 * This is needed in libSTM.
 */
void finish_action_jmp_indirect(translate_struct_t *ts)
{
    unsigned char* transl_addr = ts->transl_instr;

    /* write: call to ind_jump */
    CALL_REL32(transl_addr, (int32_t)&ind_jump);

    ts->transl_instr = transl_addr;
}

/**
 * Handles indirect jumps.
 * Indirect jumps are translated into a PUSH of the addr and a call to the
 * function "ind_jump"
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this
 * instruction
 */
finalize_tu_t action_jmp_indirect(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char* transl_addr = ts->transl_instr;
    unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
    int length = ts->next_instr - ts->cur_instr;

    PRINT_DEBUG_FUNCTION_START("action_jmp_indirect(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);

#if defined(FBT_STATISTIC)
    fbt_nr_translated_jmp_ind++;
#endif

    assert(!HAS_PREFIX(*addr)); /* no prefixes allowed */

#if defined(FBT_IND_JUMP_MULTIPLE)
    /**
     * IND_JUMP_MULTIPLE combines 3 different ind jmp optimizations
     * depending on the indirect jump one of them is used
     * - ind_prediction to predict the target statically (one predictor)
     * - ind_jumptable that builds a shadow jumptable along
     * - fast ind jump otherwise
     * 
     */

    /* if the Mod R/M byte equals 0x24, the target is specified using SIB
       and a displacement value -> use jumptable optimization */
    /* check for base register == 101 = non && check for scale == 4,
       check for index register != none = 100 */
    if ((*first_byte_after_opcode == 0x24) &&
	((*(first_byte_after_opcode+1) & 0xC7) == 0x85) &&
	((*(first_byte_after_opcode+1) & 0x38) != 0x20)) {

#if defined(FBT_STATISTIC)
	fbt_nr_ind_jump_jumptable++;
#endif
	
	PRINT_DEBUG("indirect jump multiple optimizations: *first_byte_after_opcode == 0x24 -> jumptable optimization\n");
	
	/* get index register */
	char ireg = (*(first_byte_after_opcode+1) & 0x38) >> 3; /* 0x38 = 00111000, get bits 3-5 -> index of SIB byte */
	unsigned int jmpbase = *((unsigned int *)(first_byte_after_opcode+2)); /* get displacement/jump base value */

	/**
	 * Emit indirect jump into code cache
	 **/
	
	/**
	 *	pushfl				# save flags
	 *	cmpl JUMPTABLE_SIZE, %xxx	#
	 *	jge jmpind			#
	 *	popfl 				# restore flags
	 *	jmp table_base(,%xxx, 4)	#
	 *jmpind:	 			#
	 *	popfl				# restore flags
	 *	pushl $target
	 *	pushl $tld
	 *	call ind_jmp
	 */

	PUSHFL(transl_addr);
#if defined(FBT_STATISTIC)
	INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_jump_jumptable_used);
#endif
	
	CMPL_IMM32_RM32(transl_addr, MODRM(MODE_REG, ireg, 0x07), JUMPTABLE_SIZE-1);
	
	/* 0F 8F -> JG rel32, jump if greater */
	/* 0F 8D -> JGE rel32, jump if greater or equal */
	/* 0F 8C -> JL rel32, jump if less */
	/* 0F 8E -> JLE rel32, jump if less or equal */
	JCC_2B(transl_addr, 0x8D0F, (int32_t)(0x08+transl_addr+4));
	
	POPFL(transl_addr);

	/* copy indirect jump instruction and modify displacement -> jumptable base address */
	memcpy(transl_addr, addr, length);
	transl_addr += length;
	int32_t* jmptbl = ((int32_t*)(transl_addr-4));
	
#if defined(FBT_STATISTIC)
	INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_jump_jumptable_miss);
#endif
	
	POPFL(transl_addr);
	
	/* push parameters for ind_jmp */
	/* write: push indirect target */
	*transl_addr++ = 0xFF;
	
	/*
	 * 0xFF expects a ModR/M byte following the opcode
	 * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	 * so we copy the ModR/M byte, but modify the opcode extension to 110
	 */
	*transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x30;
	
	/* if there follows a displacement copy this to the ccf */
	if (length > 2) {
	    memcpy(transl_addr, (addr + 2), length - 2);
	    transl_addr += length - 2;
	}
	
	PUSHL_IMM32(transl_addr, (int32_t)ts->tld); // pushl tld
	CALL_REL32(transl_addr, (int32_t)&ind_jump);

	/**
	 * Now look for the jumptable or create one if it was not used jet!
	 */
	
	/* iterate through linked list and find the corresponding jumptable */
	jumptable_list_t *jt_el = ts->tld->jumptable_list;
	while (jt_el != NULL) {
	    if ((jt_el->orig_base == jmpbase) && (jt_el->index_reg == ireg)) break; /* jumptable found -> break */
	    jt_el = jt_el->next;
	}
	

	/* if jumptable does not exist yet, create it and emit fixup code */
	if(jt_el == NULL) {
#if defined(FBT_STATISTIC)
	    fbt_nr_ind_jump_jumptables_created++;
#endif
	    PRINT_DEBUG("creating new jumptable\n");
	    
	    /* allocate memory for new list element */
	    jt_el = fbt_smalloc((struct thread_local_data *)ts->tld, sizeof(jumptable_list_t));
#ifdef TRUST_MEMPROTECT
	    fbt_ids_unlockmem(ts->tld, jt_el, 1, PROT_EXEC);
#endif
	    jt_el->orig_base = jmpbase; /* assign address of original jumptable */
	    /* TODO: replace by lalloc (1page)? */
	    //jt_el->table_base = fbt_smalloc((struct thread_local_data *)ts->tld, JUMPTABLE_SIZE*(sizeof(char*)));
	    jt_el->table_base = fbt_lalloc((struct thread_local_data *)ts->tld, 1);
	    //jt_el->table_base = malloc(JUMPTABLE_SIZE*(sizeof(char*)));
	    //jt_el->table_base = malloc(sysconf(_SC_PAGESIZE));
#ifdef TRUST_MEMPROTECT
	    fbt_ids_unlockmem(ts->tld, jt_el->table_base, sysconf(_SC_PAGESIZE), PROT_EXEC);
#endif
	    /* allocate memory for fixup code and emit fixup code */
	    unsigned char* fixupaddr = transl_addr;

	    /* fillup new jumptable with pointers to the start of the fixup code */
	    int i;
	    for(i = 0; i < JUMPTABLE_SIZE; i++) jt_el->table_base[i] = fixupaddr;
	    
	    jt_el->index_reg = ireg; /* set index register */
	    
	    PRINT_DEBUG("indirect jump multiple optimizations: creating new fixup code @ %p \n", fixupaddr);
	    
	    /** used index register = %xxx
	     *	pushfl					# save flags (1byte)
	     *	pushl %eax				# save registers (1byte)
	     *	pushl %ecx				# (1byte)
	     *	pushl %edx				# (1byte)
	     *	pushl $target				# (7bytes)
	     *	pushl $tld				# (5bytes)
	     *	#ifdef MPROTECT_IDS
	     *  call fbt_ids_unlock
	     *  #endif
	     *	call tcache_find_fast			# (5bytes) tcache_find_fast
	     *	testl %eax, %eax			# (2bytes)
	     *	jne translated				# (2bytes)
	     *	call translate_noexecute		# (5bytes)
	     *translated:
	     *
	     *	if index register == eax
	     *		movl 16(esp), %ecx		# (4bytes)
	     *	else if index register == ecx
	     *		movl 12(esp), %ecx		# (4bytes)
	     *	else if index register == edx
	     *		movl 8(esp), %ecx		# (4bytes)
	     *
	     *	if index register == eax or ecx or edx
	     *		movl %eax, table_base(,%ecx, 4)	# backpatch jumptable (7bytes)
	     *	else
	     *		movl %eax, table_base(,%xxx, 4)	# backpatch jumptable (7bytes)
	     *
	     *	#ifdef MPROTECT_IDS
	     *  call fbt_ids_lockdown;
	     *  movl $0, (%esp);
	     *  #endif
	     *	addl $8, %esp				# readjust stack (3bytes)
	     *	popl %edx				# (1byte)
	     *	popl %ecx				# (1byte)
	     *	popl %eax				# (1byte)
	     *	popfl					# restore flags (1byte)
	     *	jmp table_base(,%xxx, 4)		# jmp to the translated target, use jumptable (7bytes)
	     * total of 54 bytes
	     */

	    PUSHFL(fixupaddr);
	    PUSHL_EAX(fixupaddr);
	    PUSHL_ECX(fixupaddr);
	    PUSHL_EDX(fixupaddr);
	    
#if defined(FBT_STATISTIC)
	    INCL_M64(fixupaddr, (int32_t)&fbt_nr_ind_jump_jumptable_fixup);
#endif
	    /* write: push indirect target */
	    *fixupaddr++ = 0xFF;
	    
	    /*
	     * 0xFF expects a ModR/M byte following the opcode
	     * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	     * so we copy the ModR/M byte, but modify the opcode extension to 110
	     */
	    *fixupaddr++ = (*first_byte_after_opcode & 0xC7) | 0x30;
	    
	    /* if there follows a displacement copy this to the ccf */
	    if (length > 2) {
		memcpy(fixupaddr, (addr + 2), length - 2);
		fixupaddr += length - 2;
	    }
	    
	    /* write; pushl $tld */
	    PUSHL_IMM32(fixupaddr, (int32_t)ts->tld);
#ifdef SECU_MPROTECT_IDS
#ifdef TRUST_MEMPROTECT
	    CALL_REL32(fixupaddr, (int32_t)&fbt_ids_unlocktld);
#else
	    CALL_REL32(fixupaddr, (int32_t)&fbt_ids_unlock);
#endif
#endif
	    /* call find fast function */
	    CALL_REL32(fixupaddr, (int32_t)&tcache_find_fast);
	    
	    TEST_REG32_REG32(fixupaddr, EAX, EAX);

	    JNE_I8(fixupaddr, 0x05);
	    
	    CALL_REL32(fixupaddr, (int32_t)&translate_noexecute);
	    
	    /* LABEL: translated */
	    if (ireg == EAX) { MOVL_SIBIMM8_R32(fixupaddr, SIB_ESP, 16, ECX); } /* esp+16 -> ECX */
	    else if (ireg == ECX) { MOVL_SIBIMM8_R32(fixupaddr, SIB_ESP, 12, ECX); } /* esp+12 -> ECX */
	    else if (ireg == EDX) { MOVL_SIBIMM8_R32(fixupaddr, SIB_ESP, 8, ECX); } /* esp+8 -> ECX */

#ifdef TRUST_MEMPROTECT
/*	    PUSHFL(fixupaddr);
	    PUSHL_EAX(fixupaddr);
	    PUSHL_ECX(fixupaddr);
	    PUSHL_EDX(fixupaddr);
*/
	    PUSHL_IMM32(fixupaddr, (int32_t)jt_el->table_base);
	    PUSHL_IMM32(fixupaddr, (int32_t)ts->tld);
	    CALL_REL32(fixupaddr, (int32_t)&fbt_ids_unlock_page);
	    ADDL_IMM8_RM32(fixupaddr, MODRM(MODE_REG, ESP, 0x0), 0x08);
/*
	    POPL_EDX(fixupaddr);
	    POPL_ECX(fixupaddr);
	    POPL_EAX(fixupaddr);
	    POPFL(fixupaddr);
*/
#endif

	    if (ireg == EAX || ireg == ECX || ireg == EDX) { MOVL_R32_SIBIMM32(fixupaddr, EAX, 0x8D, (int32_t)jt_el->table_base); } /* movl %eax, $base(, %ecx, 4) */
	    else { MOVL_R32_SIBIMM32(fixupaddr, EAX, (0x85 | (ireg<<3)), (int32_t)jt_el->table_base); } /* movl %eax, $base(, %xxx, 4) */

#ifdef SECU_MPROTECT_IDS
#ifdef TRUST_MEMPROTECT
	    CALL_REL32(fixupaddr, (int32_t)&fbt_ids_lock_list);
#else
	    CALL_REL32(fixupaddr, (int32_t)&fbt_ids_lockdown);
#endif
	    MOVL_IMM32_RM32SIB(fixupaddr, 0x04, 0x24, 0x0); /* movl $0, (%esp) */
#endif

	    /* readjust stack */
	    ADDL_IMM8_RM32(fixupaddr, MODRM(MODE_REG, ESP, 0x0), 0x08);
	    POPL_EDX(fixupaddr);
	    POPL_ECX(fixupaddr);
	    POPL_EAX(fixupaddr);
	    POPFL(fixupaddr);
	    
	    /* copy indirect jump instruction and modify displacement -> jumptable base address */
	    memcpy(fixupaddr, addr, length);
	    fixupaddr += length;
	    *((int32_t*)(fixupaddr-4)) = (int32_t)jt_el->table_base;
	    
	    /* insert new element */
	    jt_el->next = ts->tld->jumptable_list;
	    ts->tld->jumptable_list = jt_el;
	    
	    /* restore the pointer to the correct location */
	    transl_addr = fixupaddr;

	    PRINT_DEBUG("indirect jump multiple optimizations: new jumptable linked list entry orig_base: %x, table_base: %p, next list element: %p\n", jt_el->orig_base, jt_el->table_base, jt_el->next);
	}

	/* set jmptable location in the code cache */
	*jmptbl = (int32_t)jt_el->table_base;

	ts->transl_instr = transl_addr;
	
    } else /* in any other case -> use the jump fast optimization */
	PRINT_DEBUG("indirect jump multiple optimizations: -> jump fast\n");
#endif // FBT_IND_JUMP_MULTIPLE

#if defined(FBT_IND_JUMP_MULTIPLE)
    
    /* if the Mod R/M byte equals 0x25, the address of the target follows as immediate value -> use indirect jump prediction */
    //if(*first_byte_after_opcode == 0x25) {
 {
	PRINT_DEBUG("indirect jump multiple optimizations: *first_byte_after_opcode == 0x25 -> indirect jump prediction\n");
#endif
#if defined(FBT_IND_JUMP_MULTIPLE) || defined(FBT_IND_PREDICTION)
	
	unsigned char *cmptarget, *pushfltarget;
	/**
	 *	pushfl				# save flags
	 *	cmpl $AABBCCDD, r/m32		# predicted ret. addr. =?= actual ret. addr.
	 *	jne fixup			#
	 *	popfl 				# restore flags
	 *	jmp $AABBCCDD			# patched in translated predicted jmp.addr.
	 *fixup:	 			#
	 *      incl $nrofmispred
	 *	popfl				# restore flags
	 *	pushl $targetip
	 *	pushl $tld
	 *	pushl $addrOf(AABBCCDD)
	 *	call fix_ind_jmp_prediction
	 *	$nrofmispred
	 *	$addrOfPushfl
	 */

#if defined(FBT_STATISTIC)
	fbt_nr_translated_jump_ind_pred++;
	INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_jump_pred);
#endif

	pushfltarget = transl_addr;
	PUSHFL(transl_addr);
	*transl_addr++ = 0x81; /* cmpl $AABBCCDD, r/m32 */
	/*
	* 0x81 expects a ModR/M byte following the opcode
	* The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	* so we copy the ModR/M byte, but modify the opcode extension to 111
	*/
	*transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x38;
	/* if there follows a displacement copy this to the ccf */
	if (length > 2) {
		memcpy(transl_addr, (addr + 2), length - 2);
		transl_addr += length - 2;
		/* no need for SIB replacement because no SIB is used! */
	}

	*((int32_t*) transl_addr) = 0x0;	/* to be patched. predicted target addr. */
	cmptarget = transl_addr;
	transl_addr += 4;
	JNE_I8(transl_addr, 0x06);
	POPFL(transl_addr);
	JMP_REL32(transl_addr, (int32_t)0x0);  /* to be patched. predicted target addr. */
	
	/* here starts fixup */
	POPFL(transl_addr);
	
	/* write: push indirect target */
	*transl_addr++ = 0xFF;
	
	/*
	 * 0xFF expects a ModR/M byte following the opcode
	 * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	 * so we copy the ModR/M byte, but modify the opcode extension to 110
	 */
	*transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x30;

	/* if there follows a displacement copy this to the ccf */
	if (length > 2) {
		memcpy(transl_addr, (addr + 2), length - 2);
		transl_addr += length - 2;
	}
	
	/* write: push pointer to the thread local data and the to be backpatched cmptarget */
	PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
	PUSHL_IMM32(transl_addr, (int32_t)cmptarget);
	CALL_REL32(transl_addr, (int32_t)&fix_ind_jmp_prediction);
	
	*((int32_t*)transl_addr) = 0x0;
	transl_addr+=4;
	*((int32_t*)transl_addr) = (int32_t)pushfltarget;
	transl_addr+=4;

	ts->transl_instr = transl_addr;
#endif // FBT_IND_JUMP_MULTIPLE || FBT_IND_PREDICT
#if defined(FBT_IND_JUMP_MULTIPLE)
     }
#endif

#if !defined(FBT_IND_JUMP_MULTIPLE) && !defined(FBT_IND_PREDICTION)
	
	/* this is a fast version of the ind jmp - handoptimized assembler code
	 * which does a fast lookup in the hashtable and dispatches if it hits
	 * otherwise it recovers to an indirect jump
	 */
	
	/**
	 * pushl $target
	 * jmpl  tld->ind_jump_trampoline
	 */
	
	/* write: push indirect target */
	*transl_addr++ = 0xFF;
	
	/*
	 * 0xFF expects a ModR/M byte following the opcode
	 * The bits 3-5 are part of the opcode (opcode extension into ModR/M byte),
	 * so we copy the ModR/M byte, but modify the opcode extension to 110
	 */
	*transl_addr++ = (*first_byte_after_opcode & 0xC7) | 0x30;
	
	/* if there follows a displacement copy this to the ccf */
	if (length > 2) {
	    memcpy(transl_addr, (addr + 2), length - 2);
	    transl_addr += length - 2;
	}

	JMP_REL32(transl_addr, (int32_t)(ts->tld->ind_jump_trampoline));
	
	ts->transl_instr = transl_addr;
#endif	
    
    /* length bytes push target + 5 bytes push tld + 5 bytes call */
    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);

    return tu_close;
}

/**
 * Handles ret instructions.
 * Replaces ret instructions with a call to "ind_jump"
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction (after ret, that is alway the case)
 */
finalize_tu_t action_ret(translate_struct_t *ts)
{
    unsigned char *addr = ts->cur_instr;
    unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
    unsigned char* transl_addr = ts->transl_instr;

#if defined(DEBUG) || !defined(NDEBUG)
    int length = ts->next_instr - ts->cur_instr;
#endif
#if defined(FBT_STATISTIC)
    fbt_nr_translated_ret++;
#endif

    PRINT_DEBUG_FUNCTION_START("action_ret(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);

    /* ret plus evtl imm16 - no opcode prefixes */
    assert((!HAS_PREFIX(*addr) && ((*addr==0xC2 && length==3) || (*addr==0xC3 && length==1))) || \
	   (*addr==PREFIX_MISC &&  ((*(addr+1)==0xC2 && length==4) || (*(addr+1)==0xC3 && length==2)))); 
#if defined(DEBUG)
    /* see: http://readlist.com/lists/gcc.gnu.org/gcc-help/1/8765.html */
    if (!(!HAS_PREFIX(*addr) && ((*addr==0xC2 && length==3) || (*addr==0xC3 && length==1))) && *addr==PREFIX_MISC) {
	PRINT_DEBUG("Useless REPZ Prefix found (and removed) in RET (ugly gcc hack for Athlon and K8)");
    }
#endif

    // useless prefix found, skip to the real ret instr
    if (*addr == PREFIX_MISC) { addr++; }
    
#if defined(FBT_INLINE_CALLS)
    /* are we currently inlining a function
     * and do we need to unwrap the current stack frame
     * (e.g. fast return to the callee instead of going
     * through the ind_jump feature)
     */
    if (ts->inlined_frames!=NULL) {
        struct inline_struct *myinline = ts->inlined_frames;
        ts->inlined_frames = ts->inlined_frames->next;
        /* restore the stack to the point before the call */
        if(*addr == 0xC2) {
            /* leal 4+$imm(%esp), %esp  TODO: watch out for sign bit extension */
            PRINT_DEBUG("we must remove additional bytes: %d\n", *((int16_t*)first_byte_after_opcode));
            LEAL_IMM32RM32_R(transl_addr, MODRM(MODE_MEM_REG_IMM32, OP1_SIB, ESP), SIB_ESP, 0x04+(int32_t)*((int16_t*)first_byte_after_opcode));
        } else {
            /* leal 4(%esp), %esp */
            LEAL_IMM8RM32_R(transl_addr, MODRM(MODE_MEM_REG_IMM8, OP1_SIB, ESP), SIB_ESP, 0x04);
        }

        ts->next_instr = myinline->ret_addr;
        // TODO: free(myinline);

#if defined(FBT_STATISTIC)
        fbt_nr_trans_inlined_calls++;
        INCL_M64(transl_addr, (int32_t)&fbt_nr_inlined_calls);
#endif
        PRINT_DEBUG_FUNCTION_END("-> open, inlined, transl_length=%i", transl_addr-ts->transl_instr);

        ts->transl_instr = transl_addr;
        return tu_open;
    }
#endif

#if defined(FBT_RET_PREDICT)
    /*  addl $4+imm, %esp               # 3b
     *	cmpl $AABBCCDD, 4+imm(%esp)	# 8b predicted ret. addr. =?= actual ret. addr.
     *	je $transAABBCCDD               # 6b
     *  cmpl $EEFF0011, 4+imm(%esp)	# 8b second prediction
     *	je $transEEFF0011               # 6b
     *	incl nr of pispred              # 6b
     *	subl $4+imm, %esp               # 3b
     *	[pushl imm]
     *	pushl tld                       # 5b
     *	call fix_ret_predict[_imm]      # 5b
     *	0x000000 (nr of mispredictions)
     */
    if (*addr==0xc3) {
	ADDL_IMM8_RM32(transl_addr, 0xc4, 0x04); /* addl imm8, %esp (0xc4) */
	CMPL_IMM32_RM32I8SIB(transl_addr, 0x7c, 0x24, 0xfc, 0x0); /* cmpl -4(%esp), imm32 */
	JE_I32(transl_addr, 0x0);
	CMPL_IMM32_RM32I8SIB(transl_addr, 0x7c, 0x24, 0xfc, 0x0); /* cmpl -4(%esp), imm32 */
	JE_I32(transl_addr, 0x0);
	
	ADDL_IMM8_RM32(transl_addr, 0xec, 0x04); /* subl imm8, %esp (0xec) */
	
	/* write: push pointer to the thred local data & call dislocate function*/
	PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
	CALL_REL32(transl_addr, (int32_t)&fix_ret_predict);
	
	/* nr mispredictions */
	*((int32_t*)transl_addr) = 0x0;
	transl_addr += 4;

    } else {
#endif
    
#if defined(FBT_RET_CACHE)
    /*  pushl %ebx                      # 3b
     *	movzx 4(%esp), %ebx             # 5b
     *	jmpl  *retcache(, %ebx, 4)      # 7b
     */
    if (*addr==0xc3) {
	//PUSHL_EBX(transl_addr);
	//MOVZBL_IMM8RM8SIB_R32(transl_addr, 0x5c, 0x24, 0x04); /* movzbl 0x4(%esp), %ebx */
	//JMP_IND_IMM32RM(transl_addr, 0x9d, (int32_t)(ts->tld->retcache));  /* jmpl *retcache(, %ebx, 4) */
	MOVZBL_RM8SIB_R32(transl_addr, 0x0c, 0x24); /* movzbl (%esp), %ecx */
	JMP_IND_IMM32RM(transl_addr, 0x8d, (int32_t)(ts->tld->retcache));  /* jmpl *retcache(, %ecx, 4) */

	/* we are done here */
    } else {
#endif

#if defined(FBT_RET_PREDICTION)
    /*  cmpl $AABBCCDD, (%esp)	# predicted ret. addr. =?= actual ret. addr. */
    /*	je hit_ret				# */
    /*	[pushl $AABBCCDD]		# [if ret wants to pop: number of bytes to remove */
    /*	pushl $AABBCCDD			# tld */
    /*	call ret_predict[_remove]	# */
    /*hit_ret:		 			# */
    /*	addl $4	[+xx], %esp		# remove untranslated actual ret. addr. */
    /*							# [and requested additional pop] */
    /*  jmp $AABBCCDD			# patched in translated predicted ret.addr. */

    /* to be patched. predicted ret. addr. */
    CMPL_IMM32_RM32SIB(transl_addr, 0x3c, 0x24, 0x0); /* cmpl (%esp), imm32 */

    int32_t call_target;
    if(*addr == 0xC2) {
	/* this ret wants to pop some bytes of the stack */
	PRINT_DEBUG("we must remove additional bytes: %d\n", *((int16_t*) first_byte_after_opcode));

	JE_I8(transl_addr, 0x0F); /* je +15 */
	/* TODO: watch out for sign extension */
	PUSHL_IMM32(transl_addr, (int32_t)(*((int16_t*) first_byte_after_opcode)));

	call_target = (int32_t) &ret_predict_remove;
    } else {
	JE_I8(transl_addr, 0x0A); /* je +10 */

	call_target = (int32_t) &ret_predict;
    }
	
    /* write: push pointer to the thred local data & call dislocate function*/
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    CALL_REL32(transl_addr, call_target);

    if(*addr == 0xC2) {
	/* addl imm32, %esp (0xc481) */
	/* TODO: watch out for sign extension */
	ADDL_IMM32_RM32(transl_addr, 0xc4, 0x04 + (int32_t)*((int16_t*) first_byte_after_opcode));
    } else {
	/* addl imm8, %esp (0xc483) */
	ADDL_IMM8_RM32(transl_addr, 0xc4, 0x04);
    }

    /* jmp rel to backpatched address */
    JMP_REL32(transl_addr, (int32_t)transl_addr);

#elif defined(FBT_RET_STACK)
    /* pushl %eax
     * movl (&tld->tos), %eax
     * movl 4(%eax), %eax
     * cmpl 4(%esp), %eax
     * je +10
     * pushl tld
     * call ret_stack_synchronize
     * +10:
     * movl (&tld->tos), %eax
     * movl (%eax),%eax
     * movl %eax, 4(%esp)
     * addl $8, (&tld->tos)
     * popl %eax
     * ret [n]
     */

    PUSHL_EAX(transl_addr);
    MOVL_MEM32_EAX(transl_addr, (int32_t)&(ts->tld->translated_call_stack_tos));
    MOVL_IMM8RM32_R32(transl_addr, 0x40, 0x04); /* movl 4(%eax), %eax */
    CMPL_I8RM32SIB_R32(transl_addr, 0x44, 0x24, 0x04); /* cmpl 4(%esp), %eax */
    JE_I8(transl_addr, 0x0a); /* je +10 */
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    CALL_REL32(transl_addr, (int32_t)ret_stack_synchronize);
    PUSHL_IMM32(transl_addr, 0);        /* overwrite eip */
    ADDL_IMM8_R32(transl_addr, 0xc4, 4);    /* addl $4, %esp */
    /* +10: */
    MOVL_MEM32_EAX(transl_addr, (int32_t)&(ts->tld->translated_call_stack_tos));
    MOVL_RM32_R32(transl_addr, 0x00);
    MOVL_R32_IMM8RM32SIB(transl_addr, 0x44, 0x24, 0x04); /* movl %eax, 4(%esp) */
    ADDL_IMM8_MEM32(transl_addr, 0x08, (int32_t)&(ts->tld->translated_call_stack_tos));
    POPL_EAX(transl_addr);
    if(*addr == 0xC2) {
	RET_IMM16(transl_addr, *((int16_t*)first_byte_after_opcode)); /* ret imm16 */
    } else {
	RET(transl_addr); /* ret */
    }

#elif defined(FBT_RET_FAST)
    /* this is a fast version of the ret - handoptimized assembler code
     * which does a fast lookup in the hashtable and dispatches if it hits
     * otherwise it recovers to an indirect jump
     */

    /**
     * pushl  %ebx
     * pushl  %ecx
     * movl   8(%esp), %ebx       # load rip
     * movl   %ebx, %ecx          # duplicate rip
     * andl   HASH_PATTERN, %ebx  # hash function
     * cmpl hashtable_start(0, %ebx, 8), %ecx
     * jne  nohit
     *
     * movl hastable_start+4(0, %ebx, 8), %ebx # load target
     * movl %ebx, tld->ind_jump_target
     * popl %ecx
     * popl %ebx
     * leal 4+$imm(%esp), %esp
     * jmp *(tld->ind_jump_target)
     *
     * nohit:
     * popl   %ecx
     * popl   %ebx
     * [pushl size]
     * pushl  tld
     * call ind_jump/ind_jump_remove
     * 
     */
#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ret_fast);
#endif
    
    /* lookup in the first hashtable line */
    PUSHL_EBX(transl_addr);
    PUSHL_ECX(transl_addr);
    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x08); /* 0x8(%esp), %ebx - load return addr */
    //    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x04); /* 0x4(%esp), %ebx - load return addr */
    //    MOVL_R32_IMM8RM32SIB(transl_addr, 0x4c, 0x24, 0x04); /* %ecx, 0x4(%esp) - overwrite return addr */
    MOVL_R32_RM32(transl_addr, 0xd9); /* movl %ebx, %ecx - duplicate rip */

    /* andl HASH_PATTERN, %ebx - use special hash_pattern (unshifted) to get offset right */
    ANDL_IMM32_RM32(transl_addr, 0xe3, (int32_t)(HASH_PATTERN>>3));
    /* subl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
    CMPL_R32_IMM32RM32SIB(transl_addr, 0x0c, 0xdd, (int32_t)(ts->tld->hashtable));
    /* TODO: replaced 9d -> dd */
    

    /********************
     * Hit or no hit?   *
     * jump if ecx is 0 *
     ********************/
    JNE_I8(transl_addr, 0x0); /* offset and additional 5 bytes if ret removes bytes */
    char *hitloc = ((char*)transl_addr)-1;

    /**************************************************
     *  we hit it - lets jump to the correct location *
     **************************************************/
    
#ifdef TRUST_MEMPROTECT
    //PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    //CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld);
    PUSHL_EAX(transl_addr);
    PUSHL_ECX(transl_addr);
    PUSHL_EDX(transl_addr);
    //PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld_noarg);
    //CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld);
    //ADDL_IMM8_RM32(transl_addr, MODRM(MODE_REG, ESP, 0x0), 0x04);
    POPL_EDX(transl_addr);
    POPL_ECX(transl_addr);
    POPL_EAX(transl_addr);
#endif

    /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
    MOVL_IMM32RM32SIB_R32(transl_addr, 0x1c, 0xdd, (int32_t)(ts->tld->hashtable)+4);

    /* save CCF to ind_jump_target */
    /* movl %ebx, &(tld->ind_jump_target) */
    MOVL_R32_IMM32RM32(transl_addr, 0x1d, (int32_t) &(ts->tld->ind_jump_target));
    
    POPL_ECX(transl_addr);
    //MOVL_RM32SIB_R32(transl_addr, 0x0c, 0x24); /* movl (%esp), %ebx */
    POPL_EBX(transl_addr);
    //MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x04); /* movl 0x4(%esp), %ebx */
    // POPL_ECX(transl_addr);

    if (*addr == 0xc3) {
	LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);   /* leal 4(%esp), %esp */
    } else {
        /* ret removes additional nr of bytes */
        PRINT_DEBUG("we must remove additional bytes: %d\n", *((int16_t*)first_byte_after_opcode));
	LEAL_IMM32RM32_R(transl_addr, 0xa4, 0x24, (0x04+*((int16_t*)first_byte_after_opcode)));   /* leal 4(%esp), %esp */
	// LEAL_IMM32RM32_R(transl_addr, 0xa4, 0x24, (*((int16_t*)first_byte_after_opcode)));   /* leal 4(%esp), %esp */
    }
    JMP_IND_M32(transl_addr, (int32_t)&(ts->tld->ind_jump_target));   /* jmp *(tld->ind_jump_target) */
    
    /************************************
     * recover mode - there was no hit! *
     ************************************/
    *hitloc = (char)(((int32_t)transl_addr)-(((int32_t)hitloc)+1));

#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ret_fast_miss);
#endif	
    POPL_ECX(transl_addr);
    // MOVL_R32_RM32(transl_addr, 0xcb); /* movl %ecx, %ebx - 2b */
    // MOVL_IMM8RM32SIB_R32(transl_addr, 0x4c, 0x24, 0x04); /* movl 0x4(%esp), %ecx - 4b */
    // MOVL_R32_IMM8RM32SIB(transl_addr, 0x5c, 0x24, 0x04); /* movl %ebx, 0x4(%esp) - 4b */
    POPL_EBX(transl_addr);

    if (*addr == 0xc2) {
        /* TODO: watch out for sign bit extension */
        PUSHL_IMM32(transl_addr, *((int16_t*)first_byte_after_opcode));
    }
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld); /* pushl tld */

    if (*addr!=0xc2 && *addr!=0xc3) {
        printf("something weird happened: 0x%x %p\n", *addr, addr);
    }
    /* call slow function */
    if (*addr == 0xc2) {
        /* 0xE8 expects 4 bytes relative offset to the instuction pointer following the opcode */
        CALL_REL32(transl_addr, (int32_t)&ind_jump_remove);
    } else {
        /* 0xE8 expects 4 bytes relative offset to the instuction pointer following the opcode */
        CALL_REL32(transl_addr, (int32_t)&ind_jump);
    }
#if defined(FBT_RET_PREDICT) || defined(FBT_RET_CACHE)
    }
#endif
    
#else /* !FBT_RET_PREDICTION && !FBT_RET_STACK && !FBT_RET_FAST */
    /*
     * This is the simple case, we just replace the ret
     * with an indirect jump that translates the control flow
     * back to the callee (no optimization)
     */
    int32_t jmp_target = 0;
    if(*addr == 0xC2) {
        /* this ret wants to pop some bytes of the stack */
        PRINT_DEBUG("we must remove additional bytes: %d\n", *((int16_t*)first_byte_after_opcode));
        /* TODO: watch out for sign extension */
        PUSHL_IMM32(transl_addr, *((int16_t*)first_byte_after_opcode));
        jmp_target = (int32_t)&ind_jump_remove;
    } else {
        jmp_target = (int32_t)&ind_jump;
    }
    /* write: push pointer to the thred local data */
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    
    /* ind_jump, because return address is not necessarily already translated (if push, jmp in original code instead of call) */
    CALL_REL32(transl_addr, jmp_target);
#endif /* !FBT_RET_PREDICTION && !FBT_RET_STACK && !FBT_RET_FAST */

    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_close;
}


/**
 * This function catches a sysenter and repares the EIP on the stack
 * so that SYSEXIT returns to the correctly translated code
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_sysenter(translate_struct_t *ts)
{
#if !defined(NDEBUG)
    unsigned char *addr = ts->cur_instr;
    int length = ts->next_instr - ts->cur_instr;
#endif
    unsigned char* transl_addr = ts->transl_instr;


    PRINT_DEBUG_FUNCTION_START("action_sysenter(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
    assert(!HAS_PREFIX(*addr) && length==2); /* sysenter is 2 bytes long */

    /* write: push pointer to the thread local data */
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);

    /* write: jump to authorize_sysenter or prepare_sysenter */
#ifdef SECU_SYSCALL_AUTH
    JMP_REL32(transl_addr, (int32_t)&authorize_sysenter);
#else
    JMP_REL32(transl_addr, (int32_t)&prepare_sysenter);
#endif /* SECU_SYSCALL_AUTH */

    /* call sysenter itself */
    /* SYSENTER(transl_addr); */ 
    /* sysenter instruction has been put into authorize_sysenter */

    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_close;
}

/**
 * This function writes a call to an authorization function in front of an 
 * int 0x80 instruction.
 * If the system call is not authorized, function will modify its return 
 * instruction pointer to jump over the int 0x80 instruction.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_int(translate_struct_t *ts)
{
#ifdef DEBUG
    unsigned char *addr = ts->cur_instr;
#endif
    unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
    unsigned char* transl_addr = ts->transl_instr;
    /* if this is not a system call interrupt, copy the instruction */
    if (0x80 != *first_byte_after_opcode) {
        return action_copy(ts);
    }
#if !defined(NDEBUG)
    int length = ts->next_instr - ts->cur_instr;
#endif
    PRINT_DEBUG_FUNCTION_START("action_int(*addr=%p, *transl_addr=%p, length=%i)", addr, transl_addr, length);
#ifdef SECU_SYSCALL_AUTH
    PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    CALL_REL32(transl_addr, (int32_t) &authorize_int0x80);
#endif /* SECU_SYSCALL_AUTH */
    INT80(transl_addr);
    
    PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i", transl_addr-ts->transl_instr);
    ts->transl_instr = transl_addr;
    return tu_open;
}


/**
 * Prints warning and copies instruction.
 * Is to be used for unhandled opcodes. Tries to recover by copying the instruction verbatim.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_warn(translate_struct_t *ts)
{
    PRINT_DEBUG_FUNCTION_START("action_warn(*ts=%p)", ts);

    PRINT_ERROR("unhandled opcode encountered in TU at %p: %s", ts->cur_instr, printnbytes(ts->cur_instr, ts->next_instr-ts->cur_instr));
    PRINT_ERROR("Will try if it works to simply copy the instruction into the CCF, but something bad could happen now...");

    PRINT_DEBUG_FUNCTION_END("-> ???");
    return action_copy(ts);
}

/**
 * This action just fails and stops the program with an error.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_fail(translate_struct_t *ts)
{
    PRINT_DEBUG_FUNCTION_START("action_fail(*ts=%p)", ts);

    PRINT_ERROR("unhandled opcode encountered in TU at %p: %s", ts->cur_instr, printnbytes(ts->cur_instr, ts->next_instr-ts->cur_instr));
    PRINT_ERROR("giving up!!!");
    PRINT_DEBUG_FUNCTION_END("-> FAIL");

    PRINT_ERROR("\n\n\t\tERROR: unhandled opcode encountered in TU at %p: %s\n\n", ts->cur_instr, printnbytes(ts->cur_instr, ts->next_instr-ts->cur_instr));
    printf("\nERROR: unhandled opcode encountered in TU at %p: %s\nsecuBT will exit now!\n", ts->cur_instr, printnbytes(ts->cur_instr, ts->next_instr-ts->cur_instr));
#if defined(SLEEP_ON_FAIL)
    printf("Something bad happened (action_fail) - attach a debugger now!\n");
    sleep(5);
#endif
    assert(0==1);
    _exit(139);
    return tu_close;
}


/**
 * Does nothing.
 * @param ts is a pointer to the translation struct of the current thread
 * @return enum that determines if the TU shall be finalized after this instruction
 */
finalize_tu_t action_none(translate_struct_t *ts)
{
    PRINT_DEBUG_FUNCTION_START("action_none(*ts=%p)", ts);
    /* do nothing */
    PRINT_DEBUG_FUNCTION_END("-> neutral");
    return tu_neutral;
}

