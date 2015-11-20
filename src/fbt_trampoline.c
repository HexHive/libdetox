/**
 * @file fbt_trampoline.c
 * Implementation of thread local trampolines.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-01-11 11:21:20 +0100 (Wed, 11 Jan 2012) $
 * $LastChangedDate: 2012-01-11 11:21:20 +0100 (Wed, 11 Jan 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1154 $
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
 * You should have received a copy of thce GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <stddef.h> /* offsetof */

#include "fbt_trampoline.h"

#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_translate.h"
#include "fbt_mem_mgmt.h"
#include "fbt_syscall.h"

// TODO: conditional
#define _GNU_SOURCE
#include <link.h>

#if defined(TRACK_CFTX)
#include "fbt_restart_transaction.h"
#endif /* TRACK_CFTX */

#ifdef TRACK_BASIC_BLOCKS
#include "fbt_mutex.h"
#endif /* TRACK_BASIC_BLOCKS */

#ifdef ONLINE_PATCHING
#include "patching/fbt_patching.h"
#endif

/**
 * Initializes the trampoline that controls context transfers from unmanaged to
 * managed code, used in, e.g., fbt_start_transaction.
 * @param tld thread local data.
 */
static void initialize_unmanaged_code_trampoline(struct thread_local_data *tld);

/**
 * Initializes the ret2app trampoline that transfers control back to translated
 * code (from the context of the BT).
 * @param tld thread local data.
 */
static void initialize_ret2app_trampoline(struct thread_local_data *tld);

/**
 * Initializes the indirect jump trampoline that does a fast table lookup and
 * transfers control to the translated counterpart.
 * @param tld thread local data.
 */
static void initialize_ijump_trampoline(struct thread_local_data *tld);

/**
 * Initializes the indirect call trampoline that does a fast table lookup and
 * transfers control to the translated counterpart.
 * @param tld thread local data.
 */
static void initialize_icall_trampoline(struct thread_local_data *tld);


/**
 * Initializes the return trampoline that does a fast table lookup and
 * transfers control to the translated counterpart of the return instruction
 * (RIP) on the stack.
 * @param tld thread local data.
 */
static void initialize_ret_trampolines(struct thread_local_data *tld);

/**
 * Initializes the sysenter trampoline that handles a sysenter system call.
 * @param tld thread local data.
 */
static void initialize_sysenter_trampoline(struct thread_local_data *tld);

#if defined(ICF_PREDICT)
/** max number of mispredictions before we rewrite the prediction into a fast
    lookup. */
#define ICF_PREDICT_MAX_MISPREDICTIONS 20

/**
 * Initializes the indirect jump fixup trampoline that fixes a misprediction of
 * an indirect jump instruction
 * @param tld thread local data.
 */
static void initialize_ijump_predict_fixup(struct thread_local_data *tld);

/**
 * Initializes the indirect call fixup trampoline that fixes a misprediction of
 * an indirect call instruction
 * @param tld thread local data.
 */
static void initialize_icall_predict_fixup(struct thread_local_data *tld);

/**
 * This function fixes a wrong prediction and updates the cache. The current
 * (missed) target is added to the top of the cache.
 * @param tld thread local data.
 * @param icf_predict pointer to the prediction struct.
 * @param target pointer to the new target of the indirect control flow
 * transfer.
 * @return pointer to the translated version of the target parameter.
 */
static void *icf_predict_fixup(struct thread_local_data *tld,
                               struct icf_prediction *icf_predict,
                               void *target);
#endif  /* ICF_PREDICT */

#if defined(SEL_DEBUG)
static void initialize_ccache_flush_trampoline(struct thread_local_data *tld);
static void initialize_fast_hashmap_search_trampoline(struct thread_local_data *tld);
#endif

#if defined(HANDLE_SIGNALS)
/**
 * This trampoline is used for internal signals: we can compare the 
 * fbt_siginfo->si_ptr value to the tld to check whether it is an internal
 * message. This cannot be done in the handler itself, as we have no 
 * way of knowing the actual tld.
 * 
 * @param tld Thread-local data
 */
static void initialize_signal_trampoline(struct thread_local_data *tld);

static void initialize_bootstrap_thread_trampoline(struct thread_local_data *tld);
#endif /* HANDLE_SIGNALS */

#if defined(TRACK_CFTX)
static void initialize_interrupt_cft_trampoline(struct thread_local_data *tld);
static void initialize_interrupt_cft_from_ijump_trampoline(struct thread_local_data *tld);
#endif /* TRACK_CFTX */

/**
 * Special static function that is called from inside trampolines to translate
 * some block of code. Check source!
 * @param tld thread local data.
 * @param trampo pointer to the trampoline.
 */
static void translate_execute(struct thread_local_data *tld,
                              struct trampoline *trampo);
#if defined(SHADOWSTACK)

/**
 * This function tries to reauthenticate the original thread stack if there is a
 * (fixable) mismatch between the shadow stack and the thread stack.
 * @param tld thread local data.
 * @param new_target pointer to mismatched target.
 * @param stack_pointer %esp at the time of the return
 * @param bytes_removed Number of bytes removed from the caller stack frame
 * @return Function pointer where control is redirected next. Either the fixed
 * location or an error function that terminates the process.
 */
static void* shadowstack_reauthenticate(struct thread_local_data *tld,
                                        void *new_target,
                                        void *stack_pointer,
                                        ulong_t bytes_removed);
#endif  /* SHADOWSTACK */

#if defined(AUTHORIZE_SYSCALLS)
/**
 * Initializes the int80 trampoline that handles a sysenter system call.
 * @param tld thread local data.
 */
static void initialize_int80_trampoline(struct thread_local_data *tld);
#endif  /* AUTHORIZE_SYSCALLS */
  
enum ASM_CACHE_LOOKUP_FLAGS {
  CACHE_LOOKUP_NONE = 0,
  CACHE_LOOKUP_POPFL = 1
};    
  
#define ASM_FAST_CACHE_LOOKUP(target, query, index) \
  BEGIN_ASM(target) \
  /* A cache hit at this point might be a superficial miss (i.e. there exists \
   * an entry, but the queried entry isn't at the first possible location). \
   * We therefore scan the code cache to make sure there doesn't already exist \
   * an entry, before jumping into the binary translator. */ \
    loop: \
      cmpl {tld->mappingtable}(, index, 8), query \
      je hit \
      cmp $0, {tld->mappingtable}(, index, 8) \
      je nohit_fallback \
      incl index \
      jmp loop \
       \
    nohit_fallback:     \
  END_ASM

#define ASM_CACHE_TEST(addr, target) \
    /* Use special hash pattern (unshifted) to get offset right */ \
    andl ${MAPPING_PATTERN >> 3}, addr; \
    /* Load hashline (eip element) */ \
    cmpl {tld->mappingtable}(, addr, 8), target; \
  
#define SWITCH_TO_SECURED_STACK \
    movl %esp, {tld->stack-1}; \
    movl ${tld->stack-1}, %esp;
    
#define SWITCH_TO_USER_STACK \
    movl {tld->stack-1}, %esp;
  
 /** Generates machine code that performs a cache sweep, after we encounter the
  * 'nohit' case. This allows as to avoid jumping into the libdetox context
  * 
  * @section registers Registers
  * in %ebx: target address
  * in %ecx: 
  * Will overwrite %ebx, %ecx
  *
  * @section stack Stack
  * Assumes that the stack looks like:
  * [old %ebx]
  * [old %ecx]
  * ...
  * and will restore those register values if a hit occurs
  *
  * @section control-flow Control flow
  * If a cache hit occurs, we redirect control flow to the target instruction
  * If not, the control falls through to the instruction after the code
  * generated by asm_cache_lookup
  *
  * @param  tld Thread-local data
  * @param  transl_instr Address to the memory to which the machine code will
            be written
  * @return One byte after the last byte that was written
  */ 
static unsigned char *asm_cache_lookup(struct thread_local_data *tld,
                                       unsigned char *transl_instr,
                                       long flags) {
  /* The reason that we use JNE_I8 instead of label support in the DSL, is 
   * this allows us to more easily generate machine code that has modular 
   * components in it, as the DSL needs to know at compile time how long
   * our machine code is.
   */

  BEGIN_ASM(transl_instr)
    // Duplicate RIP
    movl %ebx, %ecx
  END_ASM  

  BEGIN_ASM(transl_instr)
    ASM_CACHE_TEST(%ebx, %ecx)
  END_ASM        

  /* Hit or no hit? jump if ecx is 0 */
  /***********************************/
  JNE_I8(transl_instr, 0x0); /* offset  */
  char *hitloc = ((char*)transl_instr)-1;

  /*  we hit it - lets jump to the correct location */
  /**************************************************/
  unsigned char *label_hit = transl_instr;
  
  BEGIN_ASM(transl_instr)
  hit:
    // Load target    
    movl {tld->mappingtable+4}(, %ebx, 8), %ebx   
  
    movl %ebx, {&tld->ind_target}
    popl %ecx
    popl %ebx
  END_ASM
  
  if (flags & CACHE_LOOKUP_POPFL) {
    BEGIN_ASM(transl_instr)
      popfl
    END_ASM
  }  
      
  BEGIN_ASM(transl_instr)
    leal 4(%esp), %esp
    jmp *{&tld->ind_target}
  END_ASM  

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */  

  /* recover mode - there was no hit! */
  /************************************/
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));
  
  BEGIN_ASM(transl_instr)
  nohit:
    // recover mode - there was no hit!
    // ************************************
  END_ASM  
    
#ifdef FAST_CACHE_LOOKUP
  /* A cache hit at this point might be a superficial miss (i.e. there exists
   * an entry, but the queried entry isn't at the first possible location).
   * We therefore scan the code cache to make sure there doesn't already exist
   * an entry, before jumping into the binary translator. */
  unsigned char *label_loop = transl_instr;
  BEGIN_ASM(transl_instr)
    loop:
      cmpl %ecx, {tld->mappingtable}(, %ebx, 8)
  END_ASM
  

  /* Hash table hit: goto 'hit' label */
  JE_I8(transl_instr, (char)((long)label_hit - (long)transl_instr - 2)); 

  BEGIN_ASM(transl_instr)
      cmp $0, {tld->mappingtable}(, %ebx, 8)
  END_ASM
  
  /* We encounter a zero entry: jump to 'nohit_fallback' */
  unsigned char *nohit_fallback_jmp = transl_instr;
  JE_I8(transl_instr, 0x0);

  BEGIN_ASM(transl_instr)
      incl %ebx
  END_ASM

  JMP_I8(transl_instr, (char)((long)label_loop - (long)transl_instr - 2));
      
  /* Backpatch label */  
  unsigned char *label_nohit_fallback = transl_instr;
  *(nohit_fallback_jmp + 1) = (char)((long)label_nohit_fallback - (long)nohit_fallback_jmp - 2);
#endif
  
  return transl_instr;
}

void fbt_initialize_trampolines(struct thread_local_data *tld) {
  initialize_unmanaged_code_trampoline(tld);
  initialize_ret2app_trampoline(tld);
  initialize_ijump_trampoline(tld);
  initialize_icall_trampoline(tld);
#if defined(ICF_PREDICT)
  initialize_ijump_predict_fixup(tld);
  initialize_icall_predict_fixup(tld);
#endif  /* ICF_PREDICT */
  initialize_ret_trampolines(tld);
  initialize_sysenter_trampoline(tld);

#if defined(AUTHORIZE_SYSCALLS)
  initialize_int80_trampoline(tld);
#endif  /* AUTHORIZE_SYSCALLS */

#if defined(SEL_DEBUG)
  initialize_ccache_flush_trampoline(tld);
  initialize_fast_hashmap_search_trampoline(tld);
#endif

#if defined(HANDLE_SIGNALS)
  initialize_signal_trampoline(tld);
  initialize_bootstrap_thread_trampoline(tld);
#endif /* HANDLE_SIGNALS */

#if defined(TRACK_CFTX)
  initialize_interrupt_cft_trampoline(tld);
  initialize_interrupt_cft_from_ijump_trampoline(tld);
#endif /* TRACK_CFTX */

}

static void initialize_unmanaged_code_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->unmanaged_code_trampoline = (void*)transl_instr;
  PRINT_DEBUG("unmanaged code trampoline is at %p\n", transl_instr);
  
  /* all trampolines transfer control to this unmanaged code trampoline */
  
  /* Generate trampoline.
   * The stack looks as follows:
   *   [ esp           ]
   *   [ rip (trampo)  ] <- removed by leal 4(%esp), %esp
   *   [ flags         ]
   *   [ 32b regs      ]
   *   [ trampo        ] <-- removed by leal 8(%esp), %esp
   *   [ tld           ] <-/
   *   [ rip (to here) ]
   * Prerequisites:
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   call tld->umanaged_code_trampoline
   */
  BEGIN_ASM(transl_instr)
    // save flags & registers
    pushfl  
    pusha 

    // push target trampoline
    pushl 36(%esp)
    addl $-16, (%esp)
    
    // push pointer to thread local data
    pushl ${tld}
    
    // needs to figure out and free trampoline
    call_abs {translate_execute}
    
    // adjust %esp
    leal 8(%esp), %esp
    
    // restore flags & registers
    popa
    popfl
    
    // remove rip from trampoline
    leal 4(%esp), %esp

    // restore esp to original stack frame
    popl %esp
    
    jmp *{&tld->ind_target}

  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_ret2app_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->ret2app_trampoline = (void*)transl_instr;
  PRINT_DEBUG("ret2app trampoline (for fbt_start_transaction) is at %p\n",
              transl_instr);

  /* Generate trampoline */
  BEGIN_ASM(transl_instr)
    // Overwrite address of the trampoline
    pushl $0x0
    // Adjust %esp
    leal 4(%esp), %esp
    jmp *({&tld->ind_target})    
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void translate_execute(struct thread_local_data *tld,
                              struct trampoline *trampo) {
  void *transl_addr = fbt_ccache_find(tld, trampo->target);
    
  if (transl_addr == NULL) {
    /* so, if the origin of the trampoline is just one void* away from the
       current transl_instr position, then we can just remove the preceding
       1byte jump as we'll continue right here anyway. btw we also must not
       backpatch in that case! */
    if ((ulong_t)trampo->origin ==
        ((ulong_t)tld->trans.transl_instr)-sizeof(void*)) {
      tld->trans.transl_instr = (void*)(((ulong_t)tld->trans.transl_instr) -
                                        (sizeof(void*) + 1));
      trampo->origin = NULL;
    }
        
    transl_addr = fbt_translate_noexecute(tld, trampo->target);
  }
  
  /* use a jump-back trampoline to jump to the translated code in the code
     cache */
  tld->ind_target = transl_addr;

  /* do we need to backpatch the newly translated code (e.g. remove the jump to
     the trampoline and redirect the control flow transfer to the newly
     translated code) */
  if (trampo->origin != NULL) {
    uint32_t *origin = (uint32_t*)(trampo->origin);
    switch (trampo->origin_t) {
      case ORIGIN_CLEAR:
        *origin = 0x0;
        break;
      case ORIGIN_RELATIVE:
        *origin = (uint32_t)((ulong_t)transl_addr - (ulong_t)origin - 4);
        break;
      case ORIGIN_ABSOLUTE:
        *origin = (uint32_t)(transl_addr);
        break;
#if defined(SHADOWSTACK)
      case ORIGIN_SHADOWSTACK: {
        struct shadowstack_entry *shadowstack = tld->shadowstack;
        
        /* backpatch all occurances on the shadowstack */
        while (shadowstack < tld->top_of_shadowstack) {
          if (shadowstack->return_address == (uint32_t)trampo->target &&
              shadowstack->translated_return_address == (ulong_t)trampo->code)
          {
            shadowstack->translated_return_address = (ulong_t)transl_addr;
          }
          ++shadowstack;
        }
        /* backpatch original call instruction */
        *origin = (ulong_t)transl_addr;
      }
        break;
#endif
      default:
        fbt_suicide_str("Illegal origin in trampoline (fbt_trampoline.c).\n");
    }
    /* free trampoline if we were able to backpatch*/
    fbt_trampoline_free(tld, trampo);
  }
}

static void initialize_ijump_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_ijump_trampoline = (void*)transl_instr;
  PRINT_DEBUG("indirect jump trampoline is at %p\n", transl_instr);

  /* Generate trampoline:
   *   pushl $target - this is done in the CC
   * ==== that's where we start ====
   *   pushfl
   *   pushl  %ebx
   *   pushl  %ecx
   *   movl   12(%esp), %ebx      # load target
   *   movl   %ebx, %ecx          # duplicate rip
   *   andl   MAPPING_PATTERN, %ebx  # hash function
   *   cmpl   mappingtable_start(0, %ebx, 8), %ecx
   *   jne    nohit
   *
   *   movl   mappingtable_start+4(0, %ebx, 8), %ebx # load target
   *   movl   %ebx, (tld->ind_target)       # set ind_target
   *   popl   %ecx
   *   popl   %ebx
   *   popfl
   *   leal 4(%esp), %esp
   *   jmp *(tld->ind_target)
   *
   * nohit:
   *   movl $target, (tld->stack-1)  # target is still in %ecx
   *   popl   %ecx
   *   popl   %ebx
   *   movl %esp, (tld->stack-2)
   *   movl tld->stack-2, %esp
   *   pushad
   *   pushl $target
   *   pushl  tld
   *   call fbt_translate_noexecute
   *   movl %eax, (tld->ind_target)
   *   leal 8(%esp), %esp
   *   popad
   *   popl %esp  ; restore esp (to original stack frame)
   *   popfl
   *   leal 4(%esp), %esp
   *   jmp *(tld->ind_target)
   */
  
  /* lookup in the first hashtable line */
  BEGIN_ASM(transl_instr)
    pushfl
    pushl %ebx
    pushl %ecx
    
    // Load target address
    movl 12(%esp), %ebx
    // Duplicate RIP
    movl %ebx, %ecx    
  END_ASM

#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_jump);
#endif    

  transl_instr = asm_cache_lookup(tld, transl_instr, CACHE_LOOKUP_POPFL);

  /* recover mode - there was no hit! */
  /************************************/
  ASM_FAST_CACHE_LOOKUP(transl_instr, %ecx, %ebx)  
  
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_jump_miss);
#endif

  BEGIN_ASM(transl_instr)
    movl %ecx, {tld->stack-1} // target
  
    popl %ecx
    popl %ebx
    
    // now left on stack: FLAGS and target
    // switch to secured stack
    movl %esp, {tld->stack-2}
    movl ${tld->stack-2}, %esp
    pusha
    
    pushl 36(%esp) // target
    pushl ${tld}
    call_abs {&fbt_translate_noexecute}

    movl %eax, {&tld->ind_target}
    leal 8(%esp), %esp  
    
    popa
    popl %esp
    popfl
    
    // Now left on stack: target
    leal 4(%esp), %esp
    jmp *{&tld->ind_target}
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */
      
  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_icall_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_icall_trampoline = (void*)transl_instr;
  PRINT_DEBUG("indirect call trampoline is at %p\n", transl_instr);

  /* this trampoline is basically the same as ijump but without pushfl */
  
  /* Generate trampoline:
   *   pushl $target - this is done in the CC
   * ==== that's where we start ====
   */
  BEGIN_ASM(transl_instr)
    pushl %ebx
    pushl %ecx
    movl 8(%esp), %ebx // Load target address
    movl %ebx, %ecx // Duplicate RIP
  END_ASM
  
  transl_instr = asm_cache_lookup(tld, transl_instr, CACHE_LOOKUP_NONE);
  
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_call_miss);
#endif

  BEGIN_ASM(transl_instr)
    movl %ecx, {tld->stack-1} // target is still in %ecx
    popl %ecx
    popl %ebx
    
    // Now left on stack: target
    leal 4(%esp), %esp
    
    // Switch to secured stack
    movl %esp, {tld->stack-2}
    movl ${tld->stack-2}, %esp
    
    pusha
    pushl 36(%esp) // target
    pushl ${tld}
    
    call_abs {&fbt_translate_noexecute}
    
    movl %eax, {&tld->ind_target}
    leal 8(%esp), %esp
    popa
    
    popl %esp // restore esp (to original stack frame)
    jmp *{&tld->ind_target}
  END_ASM    

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */  
  
  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_ret_trampolines(struct thread_local_data *tld) {
#if !defined(SHADOWSTACK)
  /* we use the same trampoline as for indirect calls */
  tld->opt_ret_trampoline = tld->opt_icall_trampoline;

  /* special trampoline that removes a couple of bytes from the stack */
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_ret_remove_trampoline = (void*)transl_instr;
  PRINT_DEBUG("(removing) ret trampoline is at %p\n", transl_instr);

  /* this trampoline is basically the same as ijump but without pushfl and
     the addition that we pop additional bytes from the stack */
  
  /* Generate trampoline:
   *   pushl $target             - this is done in the CC
   *   pushl $nr_bytes_to_remove - this is done in the CC
   * ==== that's where we start ====
   *   pushl  %ebx
   *   pushl  %ecx
   *   movl   12(%esp), %ebx      # load target
   *   movl   %ebx, %ecx          # duplicate rip
   *   andl   MAPPING_PATTERN, %ebx  # hash function
   *   cmpl   mappingtable_start(0, %ebx, 8), %ecx
   *   jne    nohit
   *
   *   movl   mappingtable_start+4(0, %ebx, 8), %ebx # load target
   *   movl   %ebx, (tld->ind_target)       # set ind_target
   *   popl   %ecx
   *   popl   %ebx
   *   addl (%esp), %esp          # remove additional bytes from stack
   *   leal 8(%esp), %esp
   *   jmp *(tld->ind_target)
   *
   * nohit:
   *   movl $target, (tld->stack-10) # store to location right after pushad
   *   popl   %ecx
   *   popl   %ebx   
   *   addl (%esp), %esp          # remove additional bytes from stack
   *   leal 8(%esp), %esp
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   pushad
   *   leal -4(%esp), %esp
   *   pushl  tld
   *   call fbt_translate_noexecute
   *   movl %eax, (tld->ind_target)
   *   leal 8(%esp), %esp
   *   popad
   *   popl %esp  ; restore esp (to original stack frame)
   *   jmp *(tld->ind_target)
   */
  BEGIN_ASM(transl_instr)
    pushl %ebx
    pushl %ecx
    
    // Load target address
    movl 12(%esp), %ebx
    // Duplicate RIP
    movl %ebx, %ecx
  END_ASM  

#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ret_remove);
#endif

  BEGIN_ASM(transl_instr)
    ASM_CACHE_TEST(%ebx, %ecx)
  END_ASM        

  /* Hit or no hit? jump if ecx is 0 */
  /***********************************/
  JNE_I8(transl_instr, 0x0); /* offset  */
  char *hitloc = ((char*)transl_instr)-1;

  /*  we hit it - lets jump to the correct location */
  /**************************************************/
  BEGIN_ASM(transl_instr)
    // Load target    
    movl {tld->mappingtable+4}(, %ebx, 8), %ebx 
      
    movl %ebx, {&tld->ind_target}
    popl %ecx
    popl %ebx
    addl (%esp), %esp
    leal 8(%esp), %esp
    
    jmp *{&tld->ind_target}
  END_ASM  
  
  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  /* recover mode - there was no hit! */
  /************************************/
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));

  ASM_FAST_CACHE_LOOKUP(transl_instr, %ecx, %ebx)  
  
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ret_remove_miss);
#endif

  BEGIN_ASM(transl_instr)
    movl %ecx, {tld->stack-10} // target is still in %ecx
    popl %ecx
    popl %ebx
    
    // Now left on stack: nr_bytes and target
    addl (%esp), %esp
    leal 8(%esp), %esp
    
    SWITCH_TO_SECURED_STACK
    
    pusha
      leal -4(%esp), %esp  // skip over target
    pushl ${tld}
    
    call_abs {&fbt_translate_noexecute}
    
    movl %eax, {&tld->ind_target}
    
    leal 8(%esp), %esp
    popa
    
    popl %esp
    jmp *{&tld->ind_target}
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */
  
#else  /* else: we use the shadow stack */
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_ret_trampoline = (void*)transl_instr;
  PRINT_DEBUG("shadowstack ret trampoline is at %p\n", transl_instr);

  #if defined(SHADOWSTACK_DEBUG)
  BEGIN_ASM(transl_instr)
    movl %esp, {tld->stack-1}
    movl ${tld->stack-1}, %esp
    
    pusha
    pushl $0
    pushl $0
    pushl ${tld}
    
    call_abs {&fbt_shadowstack_debug_ret}
    
    leal 12(%esp), %esp
    popa

    popl %esp
  END_ASM
  #endif /* SHADOWSTACK_DEBUG */

  BEGIN_ASM(transl_instr)  
    pushl %ebx
    
    /* Retrieve return address of top-most entry */
    movl {&tld->top_of_shadowstack}, %ebx
    movl (SHADOWSTACK_ENTRY_OFFSETOF_RETURN_ADDRESS - SHADOWSTACK_ENTRY_SIZE)(%ebx), %ebx
    
    /* Compare target to return address */
    cmpl 4(%esp), %ebx
    jne authorization_error

    /* Retrieve translated return address and overwrite indirection target */
    movl {&tld->top_of_shadowstack}, %ebx
    movl (SHADOWSTACK_ENTRY_OFFSETOF_TRANSLATED_RETURN_ADDRESS - SHADOWSTACK_ENTRY_SIZE)(%ebx), %ebx
    movl %ebx, {&tld->ind_target}
    subl $ SHADOWSTACK_ENTRY_SIZE, {&tld->top_of_shadowstack}
    popl %ebx
    leal 4(%esp), %esp
    
    jmp *{&tld->ind_target}
    
  authorization_error:  
  END_ASM
  
  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */  

  BEGIN_ASM(transl_instr)
    movl 0x4(%esp), %ebx // load target addr  
    movl %ebx, {tld->stack-1}  // $target is still in %ebx  
    
    popl %ebx
    
    // now left on stack: nr_bytes and target
    leal 4(%esp), %esp

    // Store %esp first
    // There still is target on the stack, so we need to add 4 bytes
    //movl %esp, -0x8(%esp)
    //leal 8(%esp), %esp
    movl %esp, {tld->stack-2}
    //leal -8(%esp), %esp

    //subl $0x4, %esp
    
    // Switch to secured stack
    movl %esp, {tld->stack-3}
    movl ${tld->stack-3}, %esp
    
    pusha
    pushl $0 // number of bytes removed
    pushl {tld->stack-2} // stack pointer
    pushl 48(%esp) // target
    pushl ${tld}
    
    call_abs {&shadowstack_reauthenticate}
    
    movl %eax, {&tld->ind_target}
    leal 16(%esp), %esp
    popa
    popl %esp
    
    jmp *{&tld->ind_target}
    
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  /* TODO: fix transfers and checks */
  //CALL_REL32(transl_instr, shadowstack_authorization_error);
  //CALL_REL32(transl_instr, &(tld->opt_icall_trampoline));
  /* end of normal return */
  
  /* special trampoline that removes a couple of bytes from the stack */
  tld->opt_ret_remove_trampoline = (void*)transl_instr;
  PRINT_DEBUG("shadowstack (removing) ret trampoline is at %p\n", transl_instr);
   
  /* pushl $target             - this is done in the CC
   * pushl $nr_bytes_to_remove - this is done in the CC
   * === that's where we start ===
   */

  #if defined(SHADOWSTACK_DEBUG)
  BEGIN_ASM(transl_instr)
    movl %esp, {tld->stack-1}
    movl ${tld->stack-1}, %esp
    
    pusha
    pushl $0
    pushl $0
    pushl ${tld}
    
    call_abs {&fbt_shadowstack_debug_ret}
    
    leal 12(%esp), %esp
    popa

    popl %esp
  END_ASM
  #endif /* SHADOWSTACK_DEBUG */
   
  BEGIN_ASM(transl_instr)
    pushl %ebx
    
    /* Retrieve return address of top-most entry */
    movl ${&tld->top_of_shadowstack}, %ebx
    movl (SHADOWSTACK_ENTRY_OFFSETOF_RETURN_ADDRESS - SHADOWSTACK_ENTRY_SIZE)(%ebx), %ebx
    
    /* Compare target to return address */
    cmpl 8(%esp), %ebx
    
    jne authorization_mismatch

    /* Retrieve translated return address and overwrite indirection target */
    movl ${&tld->top_of_shadowstack}, %ebx
    movl (SHADOWSTACK_ENTRY_OFFSETOF_TRANSLATED_RETURN_ADDRESS - SHADOWSTACK_ENTRY_SIZE)(%ebx), %ebx
    movl %ebx, {&tld->ind_target}
    subl $ SHADOWSTACK_ENTRY_SIZE, {&tld->top_of_shadowstack}
    
    popl %ebx
    
    addl (%esp), %esp // remove additional bytes from stack
    leal 8(%esp), %esp
    
    jmp_abs {&tld->ind_target}
    
  authorization_mismatch:  
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  BEGIN_ASM(transl_instr)
    movl 0x8(%esp), %ebx // load target addr
    movl %ebx, {tld->stack-1}
    
    // Store number of bytes removed
    movl 4(%esp), %ebx
    movl %ebx, {tld->stack-3}

    popl %ebx

    // now left on stack: nr_bytes and target
    addl (%esp), %esp
    leal 8(%esp), %esp
    
    // Store %esp
    movl %esp, {tld->stack-2}
    
    // Switch to secured stack
    movl %esp, {tld->stack-4}
    movl ${tld->stack-4}, %esp    
    
    pusha 
    pushl {tld->stack-3} // number of bytes removed
    pushl {tld->stack-2} // %esp
    pushl {tld->stack-1} // target        
    pushl ${tld}    

    call_abs {&shadowstack_reauthenticate}    

    movl %eax, {&tld->ind_target}    
    
    leal 16(%esp), %esp
    popa
    popl %esp
    
    jmp *{&tld->ind_target}    
  END_ASM  

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

#endif  /* SHADOWSTACK */

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

#if defined(SHADOWSTACK)
static void fbt_shadowstack_print(struct thread_local_data *tld) {
  struct shadowstack_entry *current = tld->top_of_shadowstack - 1;

  llprintf("orig | transl | esp | ebp\n");
  while (current >= tld->shadowstack) {
    llprintf("%d %x | %x | %x | %x | %s | ", (tld->top_of_shadowstack - current), current->return_address, current->translated_return_address, current->stack_pointer, current->call_target, "");

    llprintf("\n");
    current--;
  }
}

#if defined(SHADOWSTACK_IGNORE_LOADER)
static int shadowstack_callback(
  struct dl_phdr_info *info,
  size_t size,
  void *data) {
  
  struct thread_local_data *tld = (struct thread_local_data *)data;

  if (fbt_strncmp(SHADOWSTACK_LOADER_NAME, info->dlpi_name, fbt_strnlen(SHADOWSTACK_LOADER_NAME, 1024)) != 0) {
    return 0;
  }

  size_t lowest = 0xFFFFFFFF;
  size_t highest = 0;

  size_t i;
  for (i = 0; i < info->dlpi_phnum; ++i) {
    size_t low = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
    size_t high = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr + info->dlpi_phdr[i].p_memsz;

    if (high > highest) {
      highest = high;
    }

    if (low < lowest) {
      lowest = low;
    }

  }

  tld->shadowstack_loader_begin = lowest;
  tld->shadowstack_loader_end = highest;

  return 0;
}
#endif /* SHADOWSTACK_IGNORE_LOADER */

static void* shadowstack_reauthenticate(struct thread_local_data *tld,
                                        void *new_target,
                                        void *stack_pointer,
                                        ulong_t bytes_removed) {

  #if defined(SHADOWSTACK_IGNORE_LOADER)
  /* If we don't know the location of the loader yet, we iterate over the
     DSOs to find it */
  if (tld->shadowstack_loader_end == 0) {
    /* Set loader address to an empty range, so if we don't find it, 
       we won't search for it again */
    tld->shadowstack_loader_end = 0x1;
    tld->shadowstack_loader_begin = 0x1;
    /* Iterate over DSOs to find loader */
    dl_iterate_phdr(&shadowstack_callback, tld);
  }
  #endif /* SHADOWSTACK_IGNORE_LOADER */

  /* Compute current height of the loader. Note that tld->top_of_shadowstack points
     to the next *free* entry */
  int height = (long)(tld->top_of_shadowstack - tld->shadowstack) - 1;
  
  /* We might have to take into account bytes removed when computing our actual esp */
  ulong_t esp = (ulong_t)stack_pointer;
  esp -= bytes_removed;

  /* If stack is empty, we cannot fix, so we fall back to looking up in the
  code cache or translating if there is no entry */
  if (height <= 0) {
    return fbt_translate_noexecute(tld, new_target);
  }

  #if defined(SHADOWSTACK_IGNORE_LOADER)
  #define IN_LOADER(x) (tld->shadowstack_loader_begin <= x && x < tld->shadowstack_loader_end)

  /* Are we jumping into the loader / out of the loader: ignore for now */
  if (IN_LOADER(tld->shadowstack_return_origin) || IN_LOADER((ulong_t)new_target)) {
    /* Clear all entries related to the loader from the stack */
    struct shadowstack_entry *se = tld->shadowstack + 1;
    struct shadowstack_entry *insert = tld->shadowstack + 1;
    while (se < tld->top_of_shadowstack) {
      if (IN_LOADER(se->return_address)) {
        // do not copy entry
        // TODO: if we had a trampoline on the stack then we might leak memory   
      } else if (se != insert) {
        *insert++ = *se;
        insert++;
      }
      se++;
    }
    tld->top_of_shadowstack = insert;
    return fbt_translate_noexecute(tld, new_target);  
  }

  #undef IN_LOADER
  #endif /* SHADOWSTACK_IGNORE_LOADER */

  /* We check whether there's an entry that perfectly matches and roll
     back the shadow stack to it*/
  struct shadowstack_entry *top = tld->top_of_shadowstack - 1;
  while (top - 1 > tld->shadowstack) {
    /* Perfect match! */
    if (top->return_address == (ulong_t)new_target && top->stack_pointer == esp) {
      tld->top_of_shadowstack = top;
      return (void *)top->translated_return_address;
    }    
    
    top -= 1;
    
  }

  top = tld->top_of_shadowstack - 1;
  if (esp == top->stack_pointer) {
    if (top->return_address != (ulong_t)new_target) {
      /*llprintf("shadowstack_reauthenticate(tld=%x, new_target=%x, stack_pointer=%x, removed=%x)\n",
               tld,
               new_target,
               esp,
               bytes_removed);        
               
      fbt_shadowstack_print(tld);*/
    
      /* There seems to be something wrong with the stack: terminate */
      llprintf(
        "** SHADOWSTACK FAILURE *** Return address should be %x, but is %x.\n", 
        (ulong_t)top->return_address,
        (ulong_t)new_target
      );

      PRINT_DEBUG("*** SHADOWSTACK FAILURE ***\n");
      
      #if defined(SHADOWSTACK_TERMINATE_ON_FAILURE)
      fbt_suicide_str("Shadowstack authorization error (unauthorized return"
                      " address)\n");
      #endif
    } else {
      /* After fixing the stack, we have a hit */
      // NOTE: this code is not currently needed, as it is only executed
      // if we try to fix the stack in other ways (e.g. unwind it until the
      // stack address is not larger than the current one)
      tld->top_of_shadowstack -= 1;
      return (void *)top->translated_return_address;
    }
  }

  /* We did not find an entry for the current stack frame, so we can neither
  check the return address nor can we use an optimized return */  
  return fbt_translate_noexecute(tld, new_target);  
}

#ifdef SHADOWSTACK_DEBUG
void fbt_shadowstack_debug_call(struct thread_local_data *tld, void *from, void *to) {
  llprintf("call(tld=%x, from=%x, to=%x)\n", tld, from, to);  
  fbt_shadowstack_print(tld);  
  llprintf("--\n");  
}

void fbt_shadowstack_debug_ret(struct thread_local_data *tld, void *from, void *to) {
  llprintf("ret(tld=%x, from=%x, to=%x)\n", tld, from, to);
  fbt_shadowstack_print(tld);
  llprintf("--\n");    
}
#endif

#endif

static void initialize_sysenter_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->sysenter_trampoline = (void*)transl_instr;
  PRINT_DEBUG("sysenter trampoline is at %p\n", transl_instr);

  /* Stack before trampoline:
   * eip (from call to special guard page)
   * ecx, edx, ebp
   * The kernel would pop ebp, edx, and ecx when it returns from the sysenter
   * instructions, but we must hijack the control flow so that we return to
   * translated code.
   * The idea is that we switch to our secure stack and reproduce the setting
   * that is expected by the kernel. When the kernel returns we only leave the
   * eip on the stack and redirect control flow to an indirect jump.

   * Trampoline looks as follows:
   *   pop %ebp               # restore ebp
   *   leap 8(%esp), %esp     # remove edx, ecx
   *   movl %esp, (tld->stack-1) # change stack
   *   movl tld->stack-1, %esp
   *   pushl AFTER_SYSENTER
   *   pushl %ecx
   *   pushl %edx
   *   pushl %ebp
   *   movl  %esp, %ebp
   *   sysenter
   * AFTER_SYSENTER:
   *   popl %esp
   *   jmp opt_ijump_trampoline
       
   */

  BEGIN_ASM(transl_instr)
    popl %ebp
    leal 8(%esp), %esp
    SWITCH_TO_SECURED_STACK
    
#if defined(AUTHORIZE_SYSCALLS)
    pushl $-1
    pushl %esp
    pushl $1 // called by sysenter
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    pushl ${tld}
    
    // ensure that eax is in range
    andl ${MAX_SYSCALLS_TABLE-1}, %eax
#endif    

  END_ASM

#if defined(AUTHORIZE_SYSCALLS)

  /* jump through the syscall table */
  CALL_IND_MODRM_SIB_IMM32(transl_instr, 0x14, 0x85, tld->syscall_table);

  BEGIN_ASM(transl_instr)
    cmpl ${(char)(SYSCALL_AUTH_GRANTED)},  %eax
    leal 4(%esp), %esp
    
    // Restore context
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi

    je auth_granted

    // Authorization not granted, we return a fake value
    leal 12(%esp), %esp
    // Pop fake value into eax
    popl %eax
    // Dispatch back
    popl %esp
    jmp_abs {tld->opt_ijump_trampoline}

  auth_granted:  
    leal 16(%esp), %esp
    
  END_ASM
#endif

  PUSHL_IMM32(transl_instr, 0x0);
  ulong_t *patchloc = (ulong_t*)(((char*)transl_instr)-sizeof(void*));
  
  BEGIN_ASM(transl_instr) 
    pushl %ecx
    pushl %edx
    pushl %ebp
    
    movl %esp, %ebp
  END_ASM

  SYSENTER(transl_instr);
  
  /* AFTER_SYSENTER */
  *patchloc = (ulong_t)transl_instr;
  
  
  BEGIN_ASM(transl_instr)
    popl %esp
    jmp_abs {tld->opt_ijump_trampoline}
  END_ASM
    
  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

#if defined(ICF_PREDICT)
static void initialize_ijump_predict_fixup(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_ijump_predict_fixup = (void*)transl_instr;
  PRINT_DEBUG("ijump prediction fixup (for ICF) is at %p\n", transl_instr);

  /* Stack before trampoline:
   * target
   * flags
   * pointer to icf_predict struct that must be updated is stored to tld->stack-11
   */
  
  BEGIN_ASM(transl_instr)
    SWITCH_TO_SECURED_STACK
    pusha
    // load and push target
    movl {tld->stack-1}, %ebx
    movl 4(%ebx), %ebx
    pushl %ebx
    // jump over icf_target pushed before (in trampoline)
    leal -4(%esp), %esp
    pushl ${tld}
    
    call_abs {&icf_predict_fixup}
    movl %eax, {&tld->ind_target}
    leal 12(%esp), %esp
    
    popa
    popl %esp
    popfl
    leal 4(%esp), %esp
    jmp *{&tld->ind_target}
    
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */  

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_icall_predict_fixup(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_icall_predict_fixup = (void*)transl_instr;
  PRINT_DEBUG("icall prediction fixup (for ICF) is at %p\n", transl_instr);

  /* Stack before trampoline:
   * target
   * flags
   * pointer to icf_predict struct that must be updated is stored to tld->stack-11
   */

  BEGIN_ASM(transl_instr)
    SWITCH_TO_SECURED_STACK
    pusha
    // load and push target
    movl {tld->stack-1}, %ebx
    movl 4(%ebx), %ebx
    pushl %ebx
    // jump over icf_target pushed before (in trampoline)
    leal -4(%esp), %esp
    pushl ${tld}
    
    call_abs {&icf_predict_fixup}
    movl %eax, {&tld->ind_target}
    leal 12(%esp), %esp
 
    popa
    popl %esp
    popfl
    leal 4(%esp), %esp
    jmp *{&tld->ind_target}
  END_ASM

  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void *icf_predict_fixup(struct thread_local_data *tld,
                               struct icf_prediction *icf_predict,
                               void *target) {
  PRINT_DEBUG("Fixing prediction (for ICF) to %p (info at %p), \n",
              target, icf_predict);
  //llprintf("Fixing prediction (for ICF) to %p (info at %p), \n",
  //            target, icf_predict);
  void *transl = fbt_translate_noexecute(tld, target);
  /* origin is a pointer into the code cache */
  *(icf_predict->origin1) = ((ulong_t)target);
  /* dst is a pointer into the code cache */
  *(icf_predict->dst1) = ((ulong_t)transl - (ulong_t)(icf_predict->dst1) - 4);
  
  icf_predict->nrmispredict++;

  if (icf_predict->nrmispredict >= ICF_PREDICT_MAX_MISPREDICTIONS) {
    unsigned char* transl_addr = (unsigned char*)icf_predict->pred.src;
    BEGIN_ASM(transl_addr)
      jmp_abs {tld->opt_ijump_trampoline}
    END_ASM
    
    fbt_icf_predictor_free(tld, icf_predict);
  }
  return transl;
}
#endif  /* ICF_PREDICT */

#if defined(AUTHORIZE_SYSCALLS)
static void initialize_int80_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->int80_trampoline = (void*)transl_instr;
  PRINT_DEBUG("int80 trampoline is at %p\n", transl_instr);

  BEGIN_ASM(transl_instr)
    SWITCH_TO_SECURED_STACK
    
    // Default authorization return value
    pushl $-1
    
    pushl %esp

    // Called by int80
    pushl $0
    
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    pushl ${tld}

    // Ensure that eax is in range
    andl ${MAX_SYSCALLS_TABLE-1}, %eax
    
    
  END_ASM
  
  /* jump through the syscall table */
  CALL_IND_MODRM_SIB_IMM32(transl_instr, 0x14, 0x85, tld->syscall_table);
  
  BEGIN_ASM(transl_instr)
    cmpl ${(char)SYSCALL_AUTH_GRANTED}, %eax
    leal 4(%esp), %esp
    
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi    
    
    je auth_granted

    // Authorization not granted, we return a fake value
    leal 12(%esp), %esp
    
    // Pop fake value into %eax
    popl %eax
    
    // Restore original stack
    popl %esp
    
    jmp *{&tld->ind_target}
  
  auth_granted:
  
    leal 16(%esp), %esp
    popl %esp
    
    int $0x80
    jmp *{&tld->ind_target}    

    
  END_ASM
  
  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

#endif  /* AUTHORIZE_SYSCALLS */

#if defined(SEL_DEBUG)
static void initialize_ccache_flush_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->sdbg->ccache_flush_trampoline = (void *)transl_instr;

  PRINT_DEBUG("flush trampoline is at %p\n", transl_instr);

  /* Generate trampoline.
   * The stack looks as follows:
   *   [ esp           ]
   *   [ 32b regs      ]
   *   [ flags         ]
   * Prerequisites:
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   pusha
   *   pushfl
   *   jmp tld->sdbg->ccache_flush_trampoline
   */
  BEGIN_ASM(transl_instr)
    pushl {&tld->ind_target}
    pushl ${tld}
    call_abs {&fbt_translate_noexecute}

    movl %eax, {&tld->ind_target}
    leal 8(%esp), %esp

    popfl
    popa
    popl %esp
    jmp *{&tld->ind_target}
  END_ASM

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_fast_hashmap_search_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->sdbg->fast_hashmap_search_trampoline = (void *)transl_instr;

  PRINT_DEBUG("fast hashmap search trampoline is at %p\n", transl_instr);

  /* Generate trampoline.
   * The stack looks as follows:
   *   [ esp           ]
   *   [ 32b regs      ]
   *   [ flags         ]
   * Prerequisites:
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   pusha                                      WARNING:
   *   pushfl                                     WARNING: This comment looks like it has been copy-pasted
   *   jmp tld->sdbg->ccache_flush_trampoline     WARNING:
   */
  BEGIN_ASM(transl_instr)
    pushl %ebx
    pushl %ecx
    // skip null pointer this would be a false hit in hash map
    testl %eax, %eax
    je hit
    movl %eax, %ebx
    // hashmap size has to be a power of 2 (original comment said multiple)
    andl ${SDBG_HASHMAP_SIZE - 1}, %eax
    // shift with immediate is not supported by DSL!?
    shll %eax
    // value_type should have size 16 (32-bit)
    leal {tld->sdbg->watchpoints.map}(, %eax, 8), %eax
   test:
    cmpl (%eax), %ebx
    je hit
    movl 12(%eax), %ecx
    testl %ecx, %ecx
    je no_hit
    // times 16
    shll %ecx
    leal (, %ecx, 8), %ecx
    addl %ecx, %eax
    jmp test
   no_hit:
    xorl %eax, %eax
   hit:
    popl %ecx
    popl %ebx
    ret
  END_ASM

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

#endif

#if defined(TRACK_CFTX)
static void initialize_interrupt_cft_trampoline(struct thread_local_data *tld) {
  tld->interrupt_cft_trampoline = tld->trans.transl_instr;

  unsigned char *transl_instr = tld->trans.transl_instr;
  BEGIN_ASM(transl_instr)
    movl ${0}, {&tld->interrupted_cft}
  END_ASM

  tld->interrupt_cft_from_user_code_trampoline = transl_instr;
  BEGIN_ASM(transl_instr)
    // Switch to secured stack
    movl %esp, {tld->stack-1}
    movl ${tld->stack-1}, %esp

    pusha
    pushfl

    // Execute callback
    pushl ${tld}
    call *{&tld->interruption_target}
    leal 4(%esp), %esp

    popfl
    popa

    // Back to user stack
    popl %esp

    // Reset ret2app pointer
    //movl ${&tld->ind_target}, {tld->ret2app_trampoline_ind_target}

    jmp *{&tld->ind_target}
  END_ASM

  tld->trans.transl_instr = transl_instr;
}

static void initialize_interrupt_cft_from_ijump_trampoline(struct thread_local_data *tld) {
  tld->interrupt_cft_from_ijump_trampoline = tld->trans.transl_instr;
  
  unsigned char *transl_instr = tld->trans.transl_instr;
  BEGIN_ASM(transl_instr)
    // Pop target from stack so we can use other trampoline later
    movl %eax, {tld->stack-1}
    popl %eax
    movl %eax, {&tld->ind_target}
    movl {tld->stack-1}, %eax

    jmp_abs {tld->interrupt_cft_trampoline}
  END_ASM

  tld->trans.transl_instr = transl_instr;
}
#endif /* TRACK_CFTX */

#if defined(HANDLE_SIGNALS)
static void initialize_signal_trampoline(struct thread_local_data *tld) {
  tld->signal_trampoline = tld->trans.transl_instr;

  /*
  Now left on stack:
    ret_addr: long
    signal:   long
    siginfo:  fbt_siginfo_t
    ucontext: void* 
  */

  unsigned char *transl_instr = tld->trans.transl_instr;
  BEGIN_ASM(transl_instr)
    pushl %eax

    // Load siginfo 
    movl 12(%esp), %eax

    // Debug
    movl ${tld}, {offsetof(fbt_siginfo_t, value)}(%eax)

    // Load si_ptr into %eax
    movl {offsetof(fbt_siginfo_t, value)}(%eax), %eax

    cmpl ${tld}, %eax

    popl %eax



    // si_ptr is set to fbt: this is an internal signal
    je internal_sighandler

    // This seems to be a user signal, just use the standard handler
    jmp_abs {&sighandler}

    internal_sighandler:
      jmp_abs {&internal_sighandler}
  END_ASM

  tld->trans.transl_instr = transl_instr;

}

static void initialize_bootstrap_thread_trampoline(struct thread_local_data *tld) {
  tld->bootstrap_thread_trampoline = tld->trans.transl_instr;

  unsigned char *transl_instr = tld->trans.transl_instr;

  BEGIN_ASM(transl_instr)

    // Switch to secured stack
    movl %esp, {tld->stack-1}
    movl ${tld->stack-1}, %esp

    pusha
    pushfl

    // Execute thread bootstrapping
    pushl ${tld}
    call_abs {fbt_bootstrap_thread}
    leal 4(%esp), %esp

    popfl
    popa

    popl %esp

    jmp *{&tld->ind_target}

  END_ASM
  
  #if defined(TRACK_CFTX)
  (tld->trampoline_cfts_end++)->addr = transl_instr - 4;
  #endif /* TRACK_CFTX */
    
  tld->trans.transl_instr = transl_instr;
}
#endif /* HANDLE_SIGNALS */
