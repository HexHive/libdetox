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
#include "fbt_mem_alloc.h"
#include "fbt_mem_protection.h"
#include "fbt_statistic.h"



/**
 * Allocate a series of trampoline slots
 */
void trampoline_allocate(thread_local_data_t *tld)
{
    PRINT_DEBUG_FUNCTION_START("trampoline_allocate(tld=%p)", tld);
    
    /* allocate memory for trampoline list (page aligned, so we can make it executable) */
    tld->trampoline = fbt_lalloc(tld, TRAMPOLINE_PAGES);
    int num_trampolines = TRAMPOLINE_PAGES * sysconf(_SC_PAGESIZE) / sizeof(trampoline_entry_t);
    
#if defined(DEBUG) || !defined(NDEBUG)
    /* fill with illegal HLT instructions */
    memset(tld->trampoline, 0xF4, TRAMPOLINE_PAGES * sysconf(_SC_PAGESIZE));
#endif
    
    /* make executable */
    fbt_ids_setexec(tld, tld->trampoline);
    
    
    PRINT_DEBUG("initialized trampoline list at %p", tld->trampoline);
    
    /* link the list entries */
    int i = 0;
    trampoline_entry_t *current = tld->trampoline;
    for (i = 0; i < (num_trampolines - 1); i++) {
	current->next = current + 1;
	current++;
    }
    current->next = NULL;	/* last element in list */
    
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
    
#ifdef TRUST_MEMPROTECT
    fbt_ids_unlockmem(tld, tld, 1, 0);
#endif
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

#ifdef TRUST_MEMPROTECT
    /* unlock 2 pages if curr_addr is near a page boundary */
    fbt_ids_unlockmem(tld, curr_addr, 2, PROT_EXEC);
#endif

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

void trampoline_free(thread_local_data_t *tld, trampoline_entry_t *slot)
{
    PRINT_DEBUG_FUNCTION_START("trampoline_free(*tld=%p, *slot=%p)", tld, slot);
    
    assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));
    
#ifdef TRUST_MEMPROTECT
    fbt_ids_unlockmem(tld, slot, 1, PROT_EXEC);
    fbt_ids_unlockmem(tld, tld, 1, 0);
#endif
    slot->next = tld->trampoline;
    tld->trampoline = slot;
    
    PRINT_DEBUG_FUNCTION_END(" ");
}

/*
 * create jump-back trampoline.
 * This function creates a special, persistent, per-thread trampoline that is
 * used to jump back from secuBT functions to the translated code without
 * exposing the real instruction pointer to the guest code.
 * Without such a trampoline, the effective instruction pointer (that is
 * located in the translation cache) would have to be pushed onto the stack.
 * The control transfer back to the translated code would occur using a ret
 * instruction. This would expose the instruction pointer to the guest
 * code at -4(%esp).
 */
void create_jumpback_trampoline(thread_local_data_t *tld)
{
    /* allocate memory for the trampoline, possibly later more structures */
    int pagesize = sysconf(_SC_PAGESIZE);
    tld->ret_jumpback_tramp = fbt_lalloc(tld, 1);
    fbt_ids_setexec(tld, tld->ret_jumpback_tramp);

    /* write the ret jump-back trampoline */
    /*
     * This trampoline is used as follows:
     * 1. the ind_jump_target is set to the desired location
     * 2. the RIP is backpatched with the address of this trampoline
     * 3. the ret instruction then jumps to this trampoline
     */
    unsigned char *curr_addr = (unsigned char*) tld->ret_jumpback_tramp;
    PUSHL_IMM32(curr_addr, 0x0);   /* overwrite the address of this trampoline on the stack */
    LEAL_IMM8RM32_R(curr_addr, 0x64, 0x24, 0x04);   /* leal 4(%esp), %esp */
    JMP_IND_M32(curr_addr, (int32_t) &(tld->ind_jump_target));   /* jmp *(tld->ind_jump_target) */

    /*
     * Make memory executable and readonly. The use of this call is to make it
     * _readonly_ if write protection of internal data structures is not enabled,
     * because we don't need to be able to write here anymore.
     */
    if (mprotect(tld->ret_jumpback_tramp, pagesize, (PROT_READ | PROT_EXEC)) != 0) {
        perror("Could not set jump-back trampoline memory executable");
        exit(1);
    }

}

void create_indjump_fast_trampoline(thread_local_data_t *tld)
{
    unsigned char* transl_addr = tld->ts.transl_instr;
    transl_addr = (unsigned char*)(((unsigned long)transl_addr+15)&(~15)); /* align transl_addr to 16byte */
    tld->ind_jump_trampoline = (unsigned int*)transl_addr;
    
    /**
     * pushl $target - this is done in the CC
     * ==== that's where we start ====
     * pushfl
     * pushl  %ebx
     * pushl  %ecx
     * movl   12(%esp), %ebx      # load target
     * movl   %ebx, %ecx          # duplicate rip
     * andl   HASH_PATTERN, %ebx  # hash function
     * cmpl   hashtable_start(0, %ebx, 8), %ecx
     * jne    nohit
     *
     * movl   hastable_start+4(0, %ebx, 8), %ebx # load target
     * movl   %ebx, (tld->ind_jump_target)       # set ind_jump_target
     * popl   %ecx
     * popl   %ebx
     * popfl
     * leal 4(%esp), %esp
     * jmp *(tld->ind_jump_target)
     *
     * nohit:
     * popl   %ecx
     * popl   %ebx
     * popfl
     * pushl  tld
     * call ind_jump
     * 
     */
    
    /* lookup in the first hashtable line */
    PUSHFL(transl_addr);
    
    PUSHL_EBX(transl_addr);
    PUSHL_ECX(transl_addr);

#ifdef TRUST_MEMPROTECT
    //PUSHL_IMM32(transl_addr, (int32_t)ts->tld);
    //CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld);
    PUSHL_EAX(transl_addr);
    PUSHL_ECX(transl_addr);
    PUSHL_EDX(transl_addr);
    //PUSHL_IMM32(transl_addr, (int32_t)tld);
    CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld_noarg);
    //CALL_REL32(transl_addr, (int32_t)&fbt_ids_unlocktld);
    //ADDL_IMM8_RM32(transl_addr, MODRM(MODE_REG, ESP, 0x0), 0x04);
    POPL_EDX(transl_addr);
    POPL_ECX(transl_addr);
    POPL_EAX(transl_addr);
#endif

    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x0C); /* 0xc(%esp), %ebx - load target addr */
    //    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x08); /* 0x8(%esp), %ebx - load return addr */
    //    MOVL_R32_IMM8RM32SIB(transl_addr, 0x4c, 0x24, 0x08); /* %ecx, 0x8(%esp) - overwrite return addr */
    MOVL_R32_RM32(transl_addr, 0xd9); /* movl %ebx, %ecx - duplicate rip */
	
#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_jump_fast);
#endif
    
    /* andl HASH_PATTERN, %ebx - use special hash_pattern (unshifted) to get offset right */
    ANDL_IMM32_RM32(transl_addr, 0xe3, (int32_t)(HASH_PATTERN>>3));
    /* cmpl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
    CMPL_R32_IMM32RM32SIB(transl_addr, 0x0c, 0xdd, (int32_t)(tld->hashtable));
    /* TODO: replaced 9d (4*) -> dd (8*) */
	
    /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
    MOVL_IMM32RM32SIB_R32(transl_addr, 0x1c, 0xdd, (int32_t)(tld->hashtable)+4);
    
    /********************
     * Hit or no hit?   *
     * jump if ecx is 0 *
     ********************/
    JNE_I8(transl_addr, 0x0); /* offset  */
    char *hitloc = ((char*)transl_addr)-1;


    /**************************************************
     *  we hit it - lets jump to the correct location *
     **************************************************/
    /* save CCF to ind_jump_target */
    /* movl %ebx, &(tld->ind_jump_target) */
    MOVL_R32_IMM32RM32(transl_addr, 0x1d, (int32_t) &(tld->ind_jump_target));
    
    POPL_ECX(transl_addr);
    POPL_EBX(transl_addr);
    POPFL(transl_addr);
    // POPL_ECX(transl_addr);
    
    LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);   /* leal 4(%esp), %esp */
    JMP_IND_M32(transl_addr, (int32_t) &(tld->ind_jump_target));   /* jmp *(tld->ind_jump_target) */

    /************************************
     * recover mode - there was no hit! *
     ************************************/
    *hitloc = (char)(((int32_t)transl_addr)-(((int32_t)hitloc)+1));
#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_jump_fast_miss);
#endif
    
    POPL_ECX(transl_addr);
    // MOVL_R32_RM32(transl_addr, 0xcb); /* movl %ecx, %ebx - 2b */
    // MOVL_IMM8RM32SIB_R32(transl_addr, 0x4c, 0x24, 0x08); /* movl 0x8(%esp), %ecx - 4b */
    // MOVL_R32_IMM8RM32SIB(transl_addr, 0x5c, 0x24, 0x08); /* movl %ebx, 0x8(%esp) - 4b */
    POPL_EBX(transl_addr);
    POPFL(transl_addr);
    
    PUSHL_IMM32(transl_addr, (int32_t)tld); /* pushl tld */
    
    /* 0xE8 expects 4 bytes relative offset to the instuction pointer following the opcode */
    CALL_REL32(transl_addr, (int32_t)&ind_jump);
    

    tld->ts.transl_instr = transl_addr;
}

void create_indcall_fast_trampoline(thread_local_data_t *tld)
{
    unsigned char* transl_addr = tld->ts.transl_instr;
    transl_addr = (unsigned char*)(((unsigned long)transl_addr+15)&(~15)); /* align transl_addr to 16byte */
    tld->ind_call_trampoline = (unsigned int*)transl_addr;
    
    /**
     * pushl $target - this is done in the CC
     * ==== that's where we start ====
     * pushl  %ebx
     * pushl  %ecx
     * movl   12(%esp), %ebx      # load target
     * movl   %ebx, %ecx          # duplicate rip
     * andl   HASH_PATTERN, %ebx  # hash function
     * cmpl   hashtable_start(0, %ebx, 8), %ecx
     * 
     * jne nohit
     * movl   hastable_start+4(0, %ebx, 8), %ebx # load target
     * movl   %ebx, (tld->ind_jump_target)       # set ind_jump_target
     * popl   %ecx
     * popl   %ebx
     * leal   4(%esp), %esp
     * jmp    *(tld->ind_jump_target)
     *
     * nohit:
     * popl   %ecx
     * popl   %ebx
     * pushl  tld
     * call   ind_jump
     *
     * 
     */
    
    /* lookup in the first hashtable line */
    PUSHL_EBX(transl_addr);
    //PUSHL_ECX(transl_addr);
    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x04); /* 0x8(%esp), %ebx - load return addr */
    //    MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x04); /* 0x4(%esp), %ebx - load return addr */
    //    MOVL_R32_IMM8RM32SIB(transl_addr, 0x4c, 0x24, 0x04); /* %ecx, 0x4(%esp) - overwrite return addr */
    MOVL_R32_RM32(transl_addr, 0xd9); /* movl %ebx, %ecx - duplicate rip */
	
#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_calls_fast);
#endif
    
    /* andl HASH_PATTERN, %ebx - use special hash_pattern (unshifted) to get offset right */
    ANDL_IMM32_RM32(transl_addr, 0xe3, (int32_t)(HASH_PATTERN>>3));
    /* cmpl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
    CMPL_R32_IMM32RM32SIB(transl_addr, 0x0c, 0xdd, (int32_t)(tld->hashtable));
    /* TODO: replaced 9d (4*) -> dd (8*) */

    /********************
     * Hit or no hit?   *
     * jump if ecx is 0 *
     ********************/
    JNE_I8(transl_addr, 0x0); /* offset  */
    char *hitloc = ((char*)transl_addr)-1;

    /**************************************************
     *  we hit it - lets jump to the correct location *
     **************************************************/
    /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
    MOVL_IMM32RM32SIB_R32(transl_addr, 0x1c, 0xdd, (int32_t)(tld->hashtable)+4);
    
    /* save CCF to ind_jump_target */
    /* movl %ebx, &(tld->ind_jump_target) */
    MOVL_R32_IMM32RM32(transl_addr, 0x1d, (int32_t) &(tld->ind_jump_target));
    
    //POPL_ECX(transl_addr);
    POPL_EBX(transl_addr);

    LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);   /* leal 4(%esp), %esp */
    JMP_IND_M32(transl_addr, (int32_t) &(tld->ind_jump_target));   /* jmp *(tld->ind_jump_target) */

    /************************************
     * recover mode - there was no hit! *
     ************************************/
    *hitloc = (char)(((int32_t)transl_addr)-(((int32_t)hitloc)+1));
#if defined(FBT_STATISTIC)
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ind_calls_fast_miss);
#endif	
    //POPL_ECX(transl_addr);
    // MOVL_R32_RM32(transl_addr, 0xcb); /* movl %ecx, %ebx - 2b */
    // MOVL_IMM8RM32SIB_R32(transl_addr, 0x4c, 0x24, 0x04); /* movl 0x4(%esp), %ecx - 4b */
    // MOVL_R32_IMM8RM32SIB(transl_addr, 0x5c, 0x24, 0x04); /* movl %ebx, 0x4(%esp) - 4b */
    POPL_EBX(transl_addr);
    
    PUSHL_IMM32(transl_addr, (int32_t)tld); /* pushl tld */
    
    /* 0xE8 expects 4 bytes relative offset to the instuction pointer following the opcode */
    CALL_REL32(transl_addr, (int32_t)&ind_jump);
    
    tld->ts.transl_instr = transl_addr;
}


#ifdef FBT_RET_CACHE
void create_ret_cache(thread_local_data_t *tld)
{
    unsigned char *transl_addr = tld->ts.transl_instr;
    transl_addr = (unsigned char*)(((unsigned long)transl_addr+15)&(~15)); /* align transl_addr to 16byte */
    /* generate the ret trampoline (default) */
    /* the simple default is a recovery that removes the ebx register from the stack
       and defaults to an ind_call */
    tld->retcache_jump = (unsigned int*)((int32_t)tld->ind_call_trampoline);

#if defined(FBT_STATISTIC)
    //POPL_EBX(transl_addr);
    tld->retcache_jump = (unsigned int*)transl_addr;
    INCL_M64(transl_addr, (int32_t)&fbt_nr_ret_cache_miss_default);
    JMP_REL32(transl_addr, (int32_t)(tld->ind_call_trampoline)+1);
    tld->ts.transl_instr = transl_addr;
#endif

    /* generate the cache */
    assert(RET_CACHE_NR_ENTRIES*sizeof(*void)<=sysconf(_SC_PAGESIZE));
    unsigned int *retcache = fbt_lalloc(tld, 1);
    tld->retcache = retcache;
    
    /* initialize all entries with the default */
    int i;
    for (i=0; i<RET_CACHE_NR_ENTRIES; i++) {
	*retcache++=(int32_t)tld->retcache_jump;
    }
}

unsigned char* write_ret_trampoline(translate_struct_t *ts, void *backpatch, unsigned char *target, void *retaddr)
{
    unsigned char *addr = target;
    /* overwrite target for trampoline in call to _here_ */
    *(unsigned int*)(backpatch)=(int32_t)target;
    
    /* now lets emit the opt. trampo code:
     * 
     * cmpl 4(%esp), ret_addr
     * jne   *retcache_jump     # this assumes that ebx is on the stack!
     * popl  %ebx
     * leal  4(%esp), %esp
     */
#if defined(FBT_STATISTIC)
    INCL_M64(addr, (int32_t)&fbt_nr_ret_cache);
#endif
    //CMPL_IMM32_RM32I8SIB(addr, 0x7c, 0x24, 0x04, (int32_t)retaddr);
    CMPL_IMM32_RM32SIB(addr, 0x3c, 0x24, (int32_t)retaddr);

    // if we want statistic we can't use a direct jump to the
    // trampoline but must use a local jump to do the statistics
#if !defined(FBT_STATISTIC)
    JNE_I32(addr, ((int32_t)ts->tld->ind_call_trampoline));
#else
    JE_I8(addr, 0x0);
    char *dst = (char*)addr-1;
    //POPL_EBX(addr);   // 1b
    INCL_M64(addr, (int32_t)&fbt_nr_ret_cache_miss);
    JMP_REL32(addr, (int32_t)ts->tld->ind_call_trampoline+1); // 6b
    *dst = (char)((int32_t)addr-((int32_t)dst+1));
#endif

    //POPL_EBX(addr);
    LEAL_IMM8RM32_R(addr, 0x64, 0x24, 0x04);   /* leal 4(%esp), %esp */

    return addr;
    
}
#endif
