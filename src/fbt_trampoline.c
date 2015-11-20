/**
 * @file fbt_trampoline.c
 * Implementation of thread local trampolines.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-03-23 09:15:08 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedDate: 2011-03-23 09:15:08 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 442 $
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

/** max number of mispredictions before we rewrite the prediciton into a fast
    lookup. */
#define ICF_PREDICT_MAX_MISPREDICTIONS 10

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
#endif  /* ICR_PREDICT */

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
 * This method is called from inside the shadowstack check if there is an
 * error/mismatch between the shadow stack and the original stack.
 */
static void shadowstack_authorization_error();

/**
 * This function tries to reauthenticate the original thread stack if there is a
 * (fixable) mismatch between the shadow stack and the thread stack.
 * @param tld thread local data.
 * @param new_target pointer to mismatched target.
 * @return Function pointer where control is redirected next. Either the fixed
 * location or an error function that terminates the process.
 */
static void* shadowstack_reauthenticate(struct thread_local_data *tld,
                                        void *new_target );
#endif  /* SHADOWSTACK */

#if defined(AUTHORIZE_SYSCALLS)
/**
 * Initializes the int80 trampoline that handles a sysenter system call.
 * @param tld thread local data.
 */
static void initialize_int80_trampoline(struct thread_local_data *tld);
#endif  /* AUTHORIZE_SYSCALLS */

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
   * Contents of the trampoline:
   *   pushfl     ; save flags
   *   pushad     ; push all registers
   *   pushl trampo ; target trampoline
   *   pushl tld  ; pointer to thread local data
   *   call translate_execute ; needs to figure out and free trampoline
   *   leal 8(%esp), %esp ; adjust %esp
   *   popad      ; restore registers
   *   popfl      ; pop flags
   *   leal 4(%esp), %esp ; remove rip from trampoline
   *   popl %esp  ; restore esp (to original stack frame)
   *   jmp *(tld->ind_target)
   */
  PUSHFL(transl_instr);
  PUSHAD(transl_instr);
  PUSHL_RM32_SIB_IMM8(transl_instr, 0x74, 0x24, 36); /* pushl 36(%esp) */
  ADDL_RM32_SIB_IMM8(transl_instr, 0x04, 0x24, 0xf0); /* addl $-16, (%esp) */
  PUSHL_IMM32(transl_instr, (ulong_t)tld);
  CALL_REL32(transl_instr, translate_execute);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08); /* leal 8(%esp), %esp */
  POPAD(transl_instr);
  POPFL(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04); /* leal 4(%esp), %esp */
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (ulong_t)(&(tld->ind_target)));

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_ret2app_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->ret2app_trampoline = (void*)transl_instr;
  PRINT_DEBUG("ret2app trampoline (for fbt_start_transaction) is at %p\n",
              transl_instr);

  /* Generate trampoline:
   *   pushl 0x0  ; overwrite address of the trampoline
   *   leal 4(%esp), %esp ; adjust %esp
   *   jmp *(tld->ind_target)
   */
  PUSHL_IMM32(transl_instr, 0x0);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);
  JMP_IND_M32(transl_instr, (ulong_t)&(tld->ind_target));

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
      ulong_t *shadowstack = tld->shadowstack;
      /* backpatch all occurances on the shadowstack */
      while (shadowstack <= tld->top_of_shadowstack) {
        if ((*shadowstack == (ulong_t)(trampo->code)) &&
            (*(shadowstack+1) == (ulong_t)(trampo->target))) {
          *shadowstack = (ulong_t)transl_addr;
        }
        shadowstack += 2; /* one shadow stack instance */
      }
      /* backpatch original call instruction */
      *origin = (ulong_t)transl_addr;
    }
      break;
#endif
    default:
      fbt_suicide_str("Illegal origin in trampoline (fbt_trampoline.c).\n");
    }
  }
  
  /* free trampoline */
  fbt_trampoline_free(tld, trampo);
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
  PUSHFL(transl_instr);
  PUSHL_EBX(transl_instr);
  PUSHL_ECX(transl_instr);
  /* movl 0xc(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_instr, 0x5c, 0x24, 0x0c);
  MOVL_R32_RM32(transl_instr, 0xd9); /* movl %ebx, %ecx  # duplicate RIP */

#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_jump);
#endif

  /* use special hash_pattern (unshifted) to get offset right */
  ANDL_IMM32_RM32(transl_instr, 0xe3, (int32_t)(MAPPING_PATTERN>>3));
  /* cmpl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
  CMPL_R32_IMM32RM32SIB(transl_instr, 0x0c, 0xdd, (int32_t)(tld->mappingtable));
  /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
  MOVL_IMM32RM32SIB_R32(transl_instr, 0x1c, 0xdd,
                        (int32_t)(tld->mappingtable)+4);

  /* Hit or no hit? jump if ecx is 0 */
  /***********************************/
  JNE_I8(transl_instr, 0x0); /* offset  */
  char *hitloc = ((char*)transl_instr)-1;

  /*  we hit it - lets jump to the correct location */
  /**************************************************/
  /* movl %ebx, &(tld->ind_target)   # set ind_target */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t) &(tld->ind_target));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  POPFL(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04); /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

  /* recover mode - there was no hit! */
  /************************************/
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_jump_miss);
#endif

  /* movl %ecx, (tld->stack-1) ($target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x0d, (int32_t)(tld->stack-1));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  /* now left on stack: FLAGS and target */
  
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-2));
  MOV_IMM32_ESP(transl_instr, (tld->stack-2));
  PUSHAD(transl_instr);
  PUSHL_RM32_SIB_IMM8(transl_instr, 0x74, 0x24, 36); /* pushl 36(%esp) target */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&fbt_translate_noexecute);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  
  POPAD(transl_instr);
  POPL_ESP(transl_instr);  
  POPFL(transl_instr);
  /* now left on stack: target */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  
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
   *   pushl  %ebx
   *   pushl  %ecx
   *   movl   8(%esp), %ebx      # load target
   *   movl   %ebx, %ecx          # duplicate rip
   *   andl   MAPPING_PATTERN, %ebx  # hash function
   *   cmpl   mappingtable_start(0, %ebx, 8), %ecx
   *   jne    nohit
   *
   *   movl   mappingtable_start+4(0, %ebx, 8), %ebx # load target
   *   movl   %ebx, (tld->ind_target)       # set ind_target
   *   popl   %ecx
   *   popl   %ebx
   *   leal 4(%esp), %esp
   *   jmp *(tld->ind_target)
   *
   * nohit:
   *   movl $target, (tld->stack-1)  # target is still in %ecx
   *   popl   %ecx
   *   popl   %ebx
   *   leal 4(%esp), %esp
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
   *   jmp *(tld->ind_target)
   */
  
  PUSHL_EBX(transl_instr);
  PUSHL_ECX(transl_instr);
  /* movl 0x8(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_instr, 0x5c, 0x24, 0x08);
  MOVL_R32_RM32(transl_instr, 0xd9);  /* movl %ebx, %ecx - duplicate rip */

#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_call);
#endif

  /* use special hash_pattern (unshifted) to get offset right */
  ANDL_IMM32_RM32(transl_instr, 0xe3, (int32_t)(MAPPING_PATTERN>>3));
  /* cmpl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
  CMPL_R32_IMM32RM32SIB(transl_instr, 0x0c, 0xdd, (int32_t)(tld->mappingtable));
  /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
  MOVL_IMM32RM32SIB_R32(transl_instr, 0x1c, 0xdd,
                        (int32_t)(tld->mappingtable)+4);

  /* Hit or no hit? jump if ecx is 0 */
  /***********************************/
  JNE_I8(transl_instr, 0x0); /* offset  */
  char *hitloc = ((char*)transl_instr)-1;

  /*  we hit it - lets jump to the correct location */
  /**************************************************/
  /* movl %ebx, &(tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t) &(tld->ind_target));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  /* left on stack: target */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

  /* recover mode - there was no hit! */
  /************************************/
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ind_call_miss);
#endif

  /* movl %ecx, (tld->stack-1) ($target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x0d, (int32_t)(tld->stack-1));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  /* now left on stack: target */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-2));
  MOV_IMM32_ESP(transl_instr, (tld->stack-2));
  PUSHAD(transl_instr);
  PUSHL_RM32_SIB_IMM8(transl_instr, 0x74, 0x24, 36); /* pushl 36(%esp) target */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&fbt_translate_noexecute);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08); /* leal 8(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  
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
  PUSHL_EBX(transl_instr);
  PUSHL_ECX(transl_instr);
  /* movl 0xc(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_instr, 0x5c, 0x24, 0x0c);
  MOVL_R32_RM32(transl_instr, 0xd9);  /* movl %ebx, %ecx - duplicate rip */

#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ret_remove);
#endif

  /* use special hash_pattern (unshifted) to get offset right */
  ANDL_IMM32_RM32(transl_instr, 0xe3, (int32_t)(MAPPING_PATTERN>>3));
  /* cmpl hashtable_start(0, %ebx, 8), %ecx - load hashline (eip element) */
  CMPL_R32_IMM32RM32SIB(transl_instr, 0x0c, 0xdd, (int32_t)(tld->mappingtable));
  /* movl hashtable_start+4(0, %ebx, 8), %ebx - load target */
  MOVL_IMM32RM32SIB_R32(transl_instr, 0x1c, 0xdd,
                        (int32_t)(tld->mappingtable)+4);

  /* Hit or no hit? jump if ecx is 0 */
  /***********************************/
  JNE_I8(transl_instr, 0x0); /* offset  */
  char *hitloc = ((char*)transl_instr)-1;

  /*  we hit it - lets jump to the correct location */
  /**************************************************/
  /* movl %ebx, &(tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t) &(tld->ind_target));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  ADDL_RM32SIB_R32(transl_instr, 0x24, 0x24);  /* addl (%esp), %esp */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

  /* recover mode - there was no hit! */
  /************************************/
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));
#if defined(FBT_STATISTIC)
  INCL_M64(transl_instr, (int32_t)&fbt_nr_ret_remove_miss);
#endif

  /* movl %ecx, (tld->stack-10)  # $target is still in %ecx */
  MOVL_R32_IMM32RM32(transl_instr, 0x0d, (int32_t)(tld->stack-10));
  POPL_ECX(transl_instr);
  POPL_EBX(transl_instr);
  /* now left on stack: nr_bytes and target */
  ADDL_RM32SIB_R32(transl_instr, 0x24, 0x24);  /* addl (%esp), %esp */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */

  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-1));
  MOV_IMM32_ESP(transl_instr, (tld->stack-1));
  PUSHAD(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0xfc);  /* leal -4(%esp), %esp */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&fbt_translate_noexecute);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  
#else  /* else: we use the shadow stack */
  
  /* we use the same trampoline as for indirect calls */
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_ret_trampoline = (void*)transl_instr;
  PRINT_DEBUG("shadowstack ret trampoline is at %p\n", transl_instr);
  /* pushl %ebx
   * movl (&tld->tos), %ebx
   * movl -4(%ebx), %ebx
   * cmpl 4(%esp), %ebx
   * jne end
   * movl (&tld->tos), %ebx
   * movl -8(%ebx), %ebx
   * movl %ebx, ind_target
   * subl $8, &tld->tos
   * popl %ebx
   * leal 4(%esp), %esp
   * jmp *tld->ind_target
   * call crash_and_burn
   */
  PUSHL_EBX(transl_instr);
  /* movl &tld->tos, %ebx */
  MOVL_IMM32RM32_R32(transl_instr, 0x1d, (int32_t)&(tld->top_of_shadowstack));
  MOVL_IMM8RM32_R32(transl_instr, 0x5b, 0xfc);  /* movl -4(%ebx), %ebx */
  /* cmpl 4(%esp), %ebx */
  CMPL_R32_IMM8RM32SIB(transl_instr, 0x5c, 0x24, 0x04);
  JNE_I8(transl_instr, 0x0);
  char *hitloc = ((char*)transl_instr)-1;
  /* movl &tld->tos, %ebx */
  MOVL_IMM32RM32_R32(transl_instr, 0x1d, (int32_t)&(tld->top_of_shadowstack));
  MOVL_IMM8RM32_R32(transl_instr, 0x5b, 0xf8);  /* movl -8(%ebx), %ebx */
  /* movl %ebx, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (uint32_t)(&(tld->ind_target)));
  /* subl $0x8, &tld->tos */
  ADDL_RM32_IMM32IMM8(transl_instr, 0x2d, (int32_t)&(tld->top_of_shadowstack),
                      0x08);
  POPL_EBX(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  /* authorization error: */
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));

  /* movl 0x4(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_instr, 0x5c, 0x24, 0x04);
  /* movl %ebx, (tld->stack-1)  # $target is still in %ebx */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t)(tld->stack-1));
  POPL_EBX(transl_instr);
  /* now left on stack: nr_bytes and target */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 8(%esp), %esp */

  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-2));
  MOV_IMM32_ESP(transl_instr, (tld->stack-2));
  PUSHAD(transl_instr);
  PUSHL_RM32_SIB_IMM8(transl_instr, 0x74, 0x24, 36); /* pushl 36(%esp) target */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&shadowstack_reauthenticate);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

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
   * pushl %ebx
   * movl (&tld->tos), %ebx
   * movl -4(%ebx), %ebx
   * cmpl 8(%esp), %ebx
   * jne end
   * movl (&tld->tos), %ebx
   * movl -8(%ebx), %ebx
   * movl %ebx, ind_target
   * subl $8, &tld->tos
   * popl %ebx
   * addl (%esp), %esp          # remove additional bytes from stack
   * leal 8(%esp), %esp
   * jmp *tld->ind_target
   * end:
   * call crash_and_burn
   */
  PUSHL_EBX(transl_instr);
  /* movl &tld->tos, %ebx */
  MOVL_IMM32RM32_R32(transl_instr, 0x1d, (int32_t)&(tld->top_of_shadowstack));
  MOVL_IMM8RM32_R32(transl_instr, 0x5b, 0xfc);  /* movl -4(%ebx), %ebx */
  /* cmpl 8(%esp), %ebx */
  CMPL_R32_IMM8RM32SIB(transl_instr, 0x5c, 0x24, 0x08);
  JNE_I8(transl_instr, 0x0);
  hitloc = ((char*)transl_instr)-1;
  /* movl &tld->tos, %ebx */
  MOVL_IMM32RM32_R32(transl_instr, 0x1d, (int32_t)&(tld->top_of_shadowstack));
  MOVL_IMM8RM32_R32(transl_instr, 0x5b, 0xf8);  /* movl -8(%ebx), %ebx */
  /* movl %ebx, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (uint32_t)(&(tld->ind_target)));
  /* subl $0x8, &tld->tos */
  ADDL_RM32_IMM32IMM8(transl_instr, 0x2d, (int32_t)&(tld->top_of_shadowstack),
                      0x08);
  POPL_EBX(transl_instr);
  ADDL_RM32SIB_R32(transl_instr, 0x24, 0x24);  /* addl (%esp), %esp */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  /* authorization missmatch: */
  *hitloc = (char)(((int32_t)transl_instr)-(((int32_t)hitloc)+1));

  /* movl 0x8(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_instr, 0x5c, 0x24, 0x08);
  /* movl %ebx, (tld->stack-1)  # $target is still in %ebx */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t)(tld->stack-1));
  POPL_EBX(transl_instr);
  /* now left on stack: nr_bytes and target */
  ADDL_RM32SIB_R32(transl_instr, 0x24, 0x24);  /* addl (%esp), %esp */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */

  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-2));
  MOV_IMM32_ESP(transl_instr, (tld->stack-2));
  PUSHAD(transl_instr);
  PUSHL_RM32_SIB_IMM8(transl_instr, 0x74, 0x24, 36); /* pushl 36(%esp) target */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&shadowstack_reauthenticate);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));
  
#endif  /* SHADOWSTACK */

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

#if defined(SHADOWSTACK)
static void* shadowstack_reauthenticate(struct thread_local_data *tld,
                                        void *new_target ) {
  llprintf("Reauthenticating: %p, caller: 0x%x\n", new_target,
           *(tld->top_of_shadowstack-1));
  ffflush();
  /* TODO: if we had a trampoline on the stack then we leak memory */
  tld->top_of_shadowstack -= 2; /* adjust shadowstack */
  //shadowstack_authorization_error();
  return fbt_translate_noexecute(tld, new_target);
}

static void shadowstack_authorization_error()
{
  fbt_suicide_str("Shadowstack authorization error (unauthorized return"
                  " address)\n");
}
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

  POPL_EBP(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x08);  /* leal 8(%esp), %esp */
  MOV_ESP_MEM32(transl_instr, (tld->stack-1));
  MOV_IMM32_ESP(transl_instr, (tld->stack-1));
#if defined(AUTHORIZE_SYSCALLS)
  PUSHL_IMM32(transl_instr, -1);
  PUSHL_ESP(transl_instr);
  PUSHL_IMM32(transl_instr, 0x1);  /* called by sysenter */
  PUSHL_EBP(transl_instr);
  PUSHL_EDI(transl_instr);
  PUSHL_ESI(transl_instr);
  PUSHL_EDX(transl_instr);
  PUSHL_ECX(transl_instr);
  PUSHL_EBX(transl_instr);
  PUSHL_EAX(transl_instr);
  PUSHL_IMM32(transl_instr, (ulong_t)tld);
  /* ensure that eax is in range */
  ANDL_IMM32_RM32(transl_instr, 0xe0, MAX_SYSCALLS_TABLE-1);
  /* jump through the syscall table */
  CALL_IND_MODRM_SIB_IMM32(transl_instr, 0x14, 0x85, tld->syscall_table);
  /* cmpl %eax, $SYSCALL_AUTH_GRANTED */
  CMPL_R32_IMM8(transl_instr, 0xf8, (char)SYSCALL_AUTH_GRANTED);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x4); /* leal 4(%esp), %esp */
  /* restore context */
  POPL_EAX(transl_instr);
  POPL_EBX(transl_instr);
  POPL_ECX(transl_instr);
  POPL_EDX(transl_instr);
  POPL_ESI(transl_instr);
  POPL_EDI(transl_instr);  
  JE_I8(transl_instr, 0x0);
  char *checkloc = (char*)(transl_instr)-1;
  /* auth not granted, we return a fake value */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0xc); /* leal 12(%esp), %esp */
  /* pop fake value into eax */
  POPL_EAX(transl_instr);
  /* dispatch back */
  POPL_ESP(transl_instr);
  JMP_REL32(transl_instr, tld->opt_ijump_trampoline);
  
  /* auth granted */
  *checkloc = (char)(((int32_t)transl_instr)-(((int32_t)checkloc)+1));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x10); /* leal 16(%esp), %esp */
#endif
  PUSHL_IMM32(transl_instr, 0x0);
  ulong_t *patchloc = (ulong_t*)(((char*)transl_instr)-sizeof(void*));
  PUSHL_ECX(transl_instr);
  PUSHL_EDX(transl_instr);
  PUSHL_EBP(transl_instr);
  MOVL_R32_RM32(transl_instr, 0xe5);  /* movl %esp, %ebp */
  SYSENTER(transl_instr);
  /* AFTER_SYSENTER */
  *patchloc = (ulong_t)transl_instr;
  
  POPL_ESP(transl_instr);
  JMP_REL32(transl_instr, tld->opt_ijump_trampoline);
  
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
   * ebx
   * pointer to icf_predict struct that must be updated is stored to tld->stack-11

   * Trampoline looks as follows:
   *   movl %ebx, (tld->stack-10)
   *   popl %ebx               # restore ebx
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   pushad
   *   leal -8(%esp), %esp # jump over target, icf_target pushed before
   *   pushl tld
   *   call icf_predict_fixup
   *   movl %eax, (tld->ind_target)
   *   leal 12(%esp), %esp
   *   popad
   *   popl %esp
   *   popfl
   *   leal 4(%esp), %esp
   *   jmpl *(tld->ind_target)
   */
  
  /* movl %ebx, (tld->stack-10)  # $target is still in %ecx */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t)(tld->stack-10));
  POPL_EBX(transl_instr);
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-1));
  MOV_IMM32_ESP(transl_instr, (tld->stack-1));
  PUSHAD(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0xf8);  /* leal -8(%esp), %esp */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&icf_predict_fixup);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x0c);  /* leal 12(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  POPFL(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void initialize_icall_predict_fixup(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->opt_icall_predict_fixup = (void*)transl_instr;
  PRINT_DEBUG("icall prediction fixup (for ICF) is at %p\n", transl_instr);

  /* Stack before trampoline:
   * target
   * ebx
   * pointer to icf_predict struct that must be updated is stored to tld->stack-11

   * Trampoline looks as follows:
   *   movl %ebx, (tld->stack-10)
   *   popl %ebx               # restore ebx
   *   movl %esp, (tld->stack-1)
   *   movl tld->stack-1, %esp
   *   pushad
   *   leal -8(%esp), %esp # jump over target, icf_target pushed before
   *   pushl tld
   *   call icf_predict_fixup
   *   movl %eax, (tld->ind_target)
   *   leal 12(%esp), %esp
   *   popad
   *   popl %esp
   *   leal 4(%esp), %esp
   *   jmpl *(tld->ind_target)
   */
  
  /* movl %ebx, (tld->stack-10)  # $target is still in %ecx */
  MOVL_R32_IMM32RM32(transl_instr, 0x1d, (int32_t)(tld->stack-10));
  POPL_EBX(transl_instr);
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-1));
  MOV_IMM32_ESP(transl_instr, (tld->stack-1));
  PUSHAD(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0xf8);  /* leal -8(%esp), %esp */
  PUSHL_IMM32(transl_instr, (int32_t)tld); /* pushl tld */
  CALL_REL32(transl_instr, (int32_t)&icf_predict_fixup);
  /* movl %eax, (tld->ind_target) */
  MOVL_R32_IMM32RM32(transl_instr, 0x05, (uint32_t)(&(tld->ind_target)));
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x0c);  /* leal 12(%esp), %esp */
  POPAD(transl_instr);
  POPL_ESP(transl_instr);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_instr, (int32_t) &(tld->ind_target));

  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}

static void *icf_predict_fixup(struct thread_local_data *tld,
                               struct icf_prediction *icf_predict,
                               void *target) {
  PRINT_DEBUG("Fixing prediction (for ICF) to %p (info at %p), \n",
              target, icf_predict);
  ffflush();
  void *transl = fbt_translate_noexecute(tld, target);
  icf_predict->origin1 = target;
  icf_predict->dst1 = transl;
  icf_predict->nrmispredict++;

  if (icf_predict->nrmispredict >= ICF_PREDICT_MAX_MISPREDICTIONS) {
    unsigned char* transl_addr = (unsigned char*)icf_predict->pred.src;
    JMP_REL32(transl_addr, (int32_t)(tld->opt_ijump_trampoline));
    fbt_icf_predictor_free(tld, icf_predict);
  }
  return transl;
}
#endif

#if defined(AUTHORIZE_SYSCALLS)
static void initialize_int80_trampoline(struct thread_local_data *tld) {
  unsigned char *transl_instr = tld->trans.transl_instr;
  tld->int80_trampoline = (void*)transl_instr;
  PRINT_DEBUG("int80 trampoline is at %p\n", transl_instr);

  /* switch to secure stack */
  MOV_ESP_MEM32(transl_instr, (tld->stack-1));
  MOV_IMM32_ESP(transl_instr, (tld->stack-1));
  /* (default) authorization return value */
  PUSHL_IMM32(transl_instr, -1);
  PUSHL_ESP(transl_instr);
  PUSHL_IMM32(transl_instr, 0x0);  /* called by int80 */
  PUSHL_EBP(transl_instr);
  PUSHL_EDI(transl_instr);
  PUSHL_ESI(transl_instr);
  PUSHL_EDX(transl_instr);
  PUSHL_ECX(transl_instr);
  PUSHL_EBX(transl_instr);
  PUSHL_EAX(transl_instr);
  PUSHL_IMM32(transl_instr, (ulong_t)tld);
  /* ensure that eax is in range */
  ANDL_IMM32_RM32(transl_instr, 0xe0, MAX_SYSCALLS_TABLE-1);
  /* jump through the syscall table */
  CALL_IND_MODRM_SIB_IMM32(transl_instr, 0x14, 0x85, tld->syscall_table);
  /* cmpl %eax, $SYSCALL_AUTH_GRANTED */
  CMPL_R32_IMM8(transl_instr, 0xf8, (char)SYSCALL_AUTH_GRANTED);
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x4); /* leal 4(%esp), %esp */
  /* restore context */
  POPL_EAX(transl_instr);
  POPL_EBX(transl_instr);
  POPL_ECX(transl_instr);
  POPL_EDX(transl_instr);
  POPL_ESI(transl_instr);
  POPL_EDI(transl_instr);  
  JE_I8(transl_instr, 0x0);
  char *checkloc = (char*)(transl_instr)-1;
  /* auth not granted, we return a fake value */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0xc); /* leal 12(%esp), %esp */
  /* pop fake value into eax */
  POPL_EAX(transl_instr);
  /* restore original stack */
  POPL_ESP(transl_instr);
  JMP_IND_M32(transl_instr, (ulong_t)(&(tld->ind_target)));
  
  /* auth granted */
  *checkloc = (char)(((int32_t)transl_instr)-(((int32_t)checkloc)+1));
  /* remove remaining arguments from checker function */
  LEAL_IMM8RM32_R(transl_instr, 0x64, 0x24, 0x10); /* leal 16(%esp), %esp */
  POPL_ESP(transl_instr);

  /* execute int80 on application stack */
  INT80(transl_instr);
  
  JMP_IND_M32(transl_instr, (ulong_t)(&(tld->ind_target)));
  
  /* forward pointer */
  tld->trans.transl_instr = transl_instr;
}
#endif  /* AUTHORIZE_SYSCALLS */

