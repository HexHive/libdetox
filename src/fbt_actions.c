/**
 * @file fbt_actions.c
 * This module defines some generic default actions that are used to translate
 * specific machine codes.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-03-23 10:26:53 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedDate: 2011-03-23 10:26:53 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 443 $
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

#include "fbt_actions.h"

#include <assert.h>
#if defined(SLEEP_ON_FAIL)
#include <unistd.h>
#endif

#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "fbt_debug.h"
#include "fbt_datatypes.h"
#include "fbt_dso.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_translate.h"
#include "fbt_mem_mgmt.h"
#include "fbt_x86_opcode.h"

enum translation_state action_none(struct translate *ts) {
  PRINT_DEBUG_FUNCTION_START("action_none(*ts=%p)", ts);
  /* do nothing */
  PRINT_DEBUG_FUNCTION_END("-> neutral");
  return NEUTRAL;
}

enum translation_state action_copy(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  long length = ts->next_instr - ts->cur_instr;

  PRINT_DEBUG_FUNCTION_START("action_copy(*addr=%p, *transl_addr=%p, "
                             "length=%i)", addr, transl_addr, length);
  /* copy instruction verbatim to translated version */
  fbt_memcpy(transl_addr, addr, length);

  PRINT_DEBUG_FUNCTION_END("-> neutral, transl_length=%i", length);
  ts->transl_instr += length;
  if (*(ts->cur_instr) == 0xcc || *(ts->cur_instr) == 0xcd ||
      *(ts->cur_instr) == 0xce) {
#if defined(AUTHORIZE_SYSCALLS)
    if (*(ts->cur_instr) == 0xcd) {
      if (*(ts->first_byte_after_opcode) != 0x80) {
        fbt_suicide_str("Illegal interrupt encountered (fbt_actions.c)\n");
      }
      /* undo copy of the interrupt */
      ts->transl_instr -= length;
      transl_addr = ts->transl_instr;
      /* store location of this syscall */
      MOVL_IMM32_MEM32(transl_addr, 0x05, addr, &(ts->tld->syscall_location));
      MOVL_IMM32_MEM32(transl_addr, 0x05, 0x00c0ff33, &(ts->tld->ind_target));
      /* pointer to after int80 */
      ulong_t *ptr = (ulong_t*)(transl_addr-sizeof(void*));
      /* write: jump instruction to trampoline */
      JMP_REL32(transl_addr, (ulong_t)(ts->tld->int80_trampoline));
      *ptr = (ulong_t)(transl_addr);
      ts->transl_instr = transl_addr;
    }
#endif
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
    PRINT_DEBUG("Encountered an interrupt - closing TU with some glue code\n");
    return CLOSE_GLUE;
  }
  return NEUTRAL;
}

enum translation_state action_warn(struct translate *ts) {
  PRINT_DEBUG_FUNCTION_START("action_warn(*ts=%p)", ts);
  PRINT_DEBUG("unhandled opcode encountered in TU at %p: %s", ts->cur_instr,
              MEMDUMP(ts->cur_instr, ts->next_instr - ts->cur_instr));
  PRINT_DEBUG("Will try if it works to simply copy the instruction into the "
              "code cache, but something bad could happen now...");
  PRINT_DEBUG_FUNCTION_END("-> ???");
  return action_copy(ts);
}

enum translation_state action_fail(struct translate *ts) {
  PRINT_DEBUG_FUNCTION_START("action_fail(*ts=%p)", ts);
  PRINT_DEBUG("unhandled opcode encountered in TU at %p: %s", ts->cur_instr,
              MEMDUMP(ts->cur_instr, ts->next_instr - ts->cur_instr));
  PRINT_DEBUG("giving up!!!");
  PRINT_DEBUG_FUNCTION_END("-> FAIL");

  PRINT_DEBUG("\n\n\t\tERROR: unhandled opcode encountered in TU at %p: %s\n\n",
              ts->cur_instr, MEMDUMP(ts->cur_instr,
              ts->next_instr - ts->cur_instr));
  llprintf("\nERROR: unhandled opcode encountered in TU at %p\nBtOX will "
           "exit now!\n", ts->cur_instr);
  ffflush();
#if defined(SLEEP_ON_FAIL)
  llprintf("Something bad happened (action_fail). Attach a debugger!\n");
  ffflush();
  sleep(5);
#endif
  fbt_suicide(255);
  return CLOSE;
}

enum translation_state action_jmp(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  int length = ts->next_instr - ts->cur_instr;

  PRINT_DEBUG_FUNCTION_START("action_jmp(*addr=%p, *transl_addr=%p, length=%i)",
                             addr, transl_addr, length);
#if defined(FBT_STATISTIC)
  fbt_nr_translated_jmp++;
#endif

  /* read call argument (either 8bit or 32bit offset) and add EIP (EIP = addr +
     length) to argument --> absolute target address = addr + length + offset */
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
#if defined(VERIFY_CFTX)
  fbt_check_transfer(ts->tld, addr, (unsigned char*)jump_target, CFTX_JMP);
#endif  /* VERIFY_CFTX */

  /* check if the target is already translated; if it is not, do so now */
  void *transl_target = fbt_ccache_find(ts->tld, (void*)jump_target);
  if (transl_target == NULL) {
    /* we still have to translate the call target */
    PRINT_DEBUG_FUNCTION_END("-> open, transl_length=0");
    /* no need to actually jump
      simply change the next instr pointer to the first instr of the function
      this will put the body of the function right as the next instr in the
      translated code */
    ts->next_instr = (unsigned char*)jump_target;
    /* put the target into the tcache so later jumps can use the translated
       code */
    fbt_ccache_add_entry(ts->tld, (void*)jump_target, ts->transl_instr);
    return OPEN;
  }

  PRINT_DEBUG("translated jmp_target: %p", transl_target);

  /* write: jmp */
  JMP_REL32(transl_addr, (int32_t)transl_target);

  PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i",
                           transl_addr - ts->transl_instr);
  ts->transl_instr = transl_addr;
  return CLOSE;
}

enum translation_state action_jmp_indirect(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
  int length = ts->next_instr - ts->cur_instr;

  PRINT_DEBUG_FUNCTION_START("action_jmp_indirect(*addr=%p, *transl_addr=%p, " \
                             "length=%i)", addr, transl_addr, length);

#if defined(FBT_STATISTIC)
  fbt_nr_translated_jmp_ind++;
#endif

  if (ts->num_prefixes != 0) {
    /* no prefixes allowed */
    fbt_suicide_str("No prefixes handled in action_jmp_indirect! " \
                    "(fbt_actions.c)\n");
  }

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
    fbt_memcpy(transl_addr, (addr + 2), length - 2);
    transl_addr += length - 2;
  }

#if defined(VERIFY_CFTX)
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_addr, (ts->tld->stack-2));
  MOV_IMM32_ESP(transl_addr, (ts->tld->stack-2));
  PUSHAD(transl_addr);
  PUSHL_IMM32(transl_addr, CFTX_JMP_IND);
  PUSHL_RM32_SIB_IMM8(transl_addr, 0x74, 0x24, 36); /* pushl 40(%esp) target */ 
  PUSHL_IMM32(transl_addr, (int32_t)addr);
  PUSHL_IMM32(transl_addr, (int32_t)ts->tld); /* pushl tld */
  CALL_REL32(transl_addr, (int32_t)&fbt_check_transfer);
  LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x10); /* leal 16(%esp), %esp */
  POPAD(transl_addr);
  POPL_ESP(transl_addr);
#endif

#if !defined(ICF_PREDICT)
  JMP_REL32(transl_addr, (int32_t)(ts->tld->opt_ijump_trampoline));
#else  /* ICF_PREDICT */
  if (ts->tld->icf_predict == NULL)
    fbt_allocate_new_icf_predictors(ts->tld);
  struct icf_prediction *pred = ts->tld->icf_predict;
  ts->tld->icf_predict = pred->pred.next;

  pred->pred.src = transl_addr;
  pred->origin1 = NULL;
  pred->dst1 = NULL;
  
  PUSHFL(transl_addr);
  PUSHL_EBX(transl_addr);
  /* movl 0x8(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x08);
  /* cmpl (pred->origin1), %ebx */
  CMPL_R32_M32(transl_addr, 0x1d, (int32_t)&(pred->origin1));
  JNE_I8(transl_addr, 0x0); /* offset  */
  char *patchloc = ((char*)transl_addr)-1;
  POPL_EBX(transl_addr);
  POPFL(transl_addr);
  LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_addr, (int32_t)&(pred->dst1));
  /* NO HIT, we need to fix the prediction */
  *patchloc = (char)(((int32_t)transl_addr)-(((int32_t)patchloc)+1));
  MOVL_IMM32_MEM32(transl_addr, 0x05, pred, (int32_t)(ts->tld->stack-11));
  JMP_REL32(transl_addr, (int32_t)(ts->tld->opt_ijump_predict_fixup));
#endif  /* ICF_PREDICT */
  
  /* length bytes push target + 5 bytes push tld + 5 bytes call */
  PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i",
                           transl_addr - ts->transl_instr);
  ts->transl_instr = transl_addr;
  
  return CLOSE;
}

enum translation_state action_jcc(struct translate *ts) {
  
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  int length = ts->next_instr - ts->cur_instr;


  PRINT_DEBUG_FUNCTION_START("action_jcc(*addr=%p, *transl_addr=%p, length=%i)",
                             addr, transl_addr, length);
#if defined(FBT_STATISTIC)
  fbt_nr_translated_jcc++;
#endif
  ulong_t jump_target;
  ulong_t fallthru_target;
  void *transl_target;

  if (ts->num_prefixes != 0) {
    fbt_suicide_str("No prefixes handled in action_jcc! (fbt_actions.c)\n");
  }  
  assert(((*addr == 0x0F && length == 6) || (length == 2)));
  /* no prefixes handled */

  /* check if we have jecxz (jump if ecx register is zero) */
  if (*addr == 0xE3) {
    PRINT_DEBUG("processing jecxz");

    /* This is a little tricky. because JECXZ has only a 8bit offset we can not
       jmp directly to the trampoline therefore two unconditional jumps are
       inserted. first the unconditional jmp for the fall through target and
       then the jmp for the original jump target. A JECXZ inst is then used to
       jump over the jump of the fall through target if the RCX register is
       zero */

    /* calculate the jump targets */
    fallthru_target = ((ulong_t)addr) + length;
    jump_target = *((char*) (addr + 1)) + fallthru_target;

    /* insert a jecxz to jump over the fall through jump if CX is 0 */
    JECXZ_I8(transl_addr, 0x05);

    /* write: jump to trampoline for fallthrough address */
    /* create trampoline if one is needed, otherwise lookup and go */
    transl_target = fbt_ccache_find(ts->tld, (void*)fallthru_target);
    if ( transl_target != NULL ) {
      JMP_REL32(transl_addr, (ulong_t)transl_target);
    } else { 
      struct trampoline *trampo =
        fbt_create_trampoline(ts->tld, (void*)fallthru_target,
                              (void*)((ulong_t)(transl_addr)+1), ORIGIN_RELATIVE);
      JMP_REL32(transl_addr, (ulong_t)(trampo->code));
    }

    /* if we have an jecxz then the jump target comes second (switchted)
       but we can use the code from the normal jcc, so we don't need to copy
       the jmp_rel32 and all */
    fallthru_target = jump_target;
  } else {
    int16_t jcc_type;
    if (*(unsigned char*) addr != 0x0F) {
      /* find out if we have a one-byte or a two-byte opcode */
      PRINT_DEBUG("processing one-byte jcc");
      /* write two-byte jcc equivalent to the one-byte jcc */
      /* the trick is: opcode + 0x10 = second byte of twobyte Jcc instruction */
      jcc_type = 0x0F + ((*addr+0x10)<<8);

      fallthru_target = ((ulong_t)addr) + length;
      jump_target = *((char*) (addr + 1)) + fallthru_target;

    } else {
      PRINT_DEBUG("processing two-byte jcc");
      /* write: copy of two-byte jcc */
      jcc_type = *((int16_t*)addr);

      fallthru_target = ((ulong_t)addr) + length;
      jump_target = *((ulong_t*) (addr + 2)) + fallthru_target;

    }
  
    /* write: jump address to trampoline; create trampoline if one is needed,
       otherwise lookup and go */
    transl_target = fbt_ccache_find(ts->tld, (void*)jump_target);
    if ( transl_target != NULL ) {
      JCC_2B(transl_addr, jcc_type, (ulong_t)transl_target);
    } else {
      struct trampoline *trampo =
        fbt_create_trampoline(ts->tld, (void*)jump_target,
                              (void*)(((ulong_t)transl_addr)+2), ORIGIN_RELATIVE);
      JCC_2B(transl_addr, jcc_type, (ulong_t)(trampo->code));
    }

  }
#if defined(VERIFY_CFTX)
  fbt_check_transfer(ts->tld, addr, (unsigned char*)jump_target, CFTX_JMP);
#endif  /* VERIFY_CFTX */
  
  /* write: jump to trampoline for fallthrough address */
  transl_target = fbt_ccache_find(ts->tld, (void*)fallthru_target);
  if ( transl_target != NULL ) {
    JMP_REL32(transl_addr, (ulong_t)transl_target);
  } else {
    struct trampoline *trampo =
      fbt_create_trampoline(ts->tld, (void*)fallthru_target,
                            (void*)((ulong_t)(transl_addr)+1), ORIGIN_RELATIVE);
    JMP_REL32(transl_addr, (ulong_t)(trampo->code));
  }

  PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i",
                           transl_addr - ts->transl_instr);
  ts->transl_instr = transl_addr;
  return CLOSE;
}

enum translation_state action_call(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  int length = ts->next_instr - ts->cur_instr;

  PRINT_DEBUG_FUNCTION_START("action_call(*addr=%p, *transl_addr=%p," \
                             " length=%i)", addr, transl_addr, length);
#if defined(FBT_STATISTIC)
  fbt_nr_translated_call++;
#endif

  /* total length of a call we handle must be 5, otherwise we have prefixes and
     such in our stream */
  assert(length == 5 && !HAS_PREFIX(*addr));

  /* our opcode should be 0xE8, a near relative call */

  /* read call argument (32bit immediate) and add EIP (EIP = addr + length) to
     argument --> absolute target address */
  ulong_t call_target = *((ulong_t*) (addr + 1)) + (ulong_t) addr + length;
  PRINT_DEBUG("original call_target: %p", (void*)call_target);

#if defined(VERIFY_CFTX)
  fbt_check_transfer(ts->tld, addr, (unsigned char*)call_target, CFTX_CALL);
#endif  /* VERIFY_CFTX */
  
  /* write: push original EIP (we have to do this either way) */
  // TODO: 64bit needs a 64bit push!
  PUSHL_IMM32(transl_addr, ((int32_t)addr) + length);

#if defined(FBT_STATISTIC)
  INCL_M64(transl_addr, (int32_t)&fbt_nr_call);
#endif

#if defined(INLINE_CALLS)
  if (ts->inline_call_RIP != NULL) {
    /* are we inlining the current function? if so, then we bail out and
        * let the translate function handle the recursion */
    PRINT_DEBUG_FUNCTION_END("-> open, inlined transl_length=%i", transl_addr -
                             ts->transl_instr);
    ts->transl_instr = transl_addr;
    /* No need to actually call the function.
       Simply change the next instr pointer to the first instr of the function.
       This will put the body of the function right as the next instr in the
       translated code */
    ts->next_instr = (unsigned char*)call_target;
    return OPEN;
  }
#endif

#if defined(SHADOWSTACK)
  /*		(pushl eip)				# see before #if
   *		pushl %ebx
   *		movl (&tld->tos), %ebx
   *		movl "trampoline_put", (%ebx) # addr backpatched later on
   * 		movl orig_return_addr, 4(%ebx) 
   *		addl $8, (&tld->tos)
   *		popl %ebx
   * 		([jmp ...])				# see after #endif
   */
  PUSHL_EBX(transl_addr);
  MOVL_IMM32RM32_R32(transl_addr, 0x1d, (int32_t)&(ts->tld->top_of_shadowstack));
  /* next_instr + 2 as movl + modrm is 2 bytes long */
  void *transl_next = fbt_ccache_find(ts->tld, ts->next_instr);
  if (transl_next == NULL) {
    struct trampoline *trampo =
      fbt_create_trampoline(ts->tld, (void*)(ts->next_instr),
                            (void*)((ulong_t)(transl_addr)+2),
                            ORIGIN_SHADOWSTACK);
    MOVL_IMM32_RM32(transl_addr, 0x03, trampo->code);
  } else {
    MOVL_IMM32_RM32(transl_addr, 0x03, transl_next);
  }
  MOVL_IMM32_IMM8RM32(transl_addr, 0x43, 0x04, ts->next_instr);
  ADDL_RM32_IMM32IMM8(transl_addr, 0x05,
                      (int32_t)&(ts->tld->top_of_shadowstack), 0x08);
  POPL_EBX(transl_addr);
  
#endif /* FBT_RET_STACK */

  /* check if target is already translated; if not, do so now */
  void *transl_target = fbt_ccache_find(ts->tld, (void*)call_target);
  
  if (transl_target == NULL) {
    /* we still have to translate the call target */
    PRINT_DEBUG_FUNCTION_END("-> open, transl_length=%i",
                             transl_addr - ts->transl_instr);
    ts->transl_instr = transl_addr;
    /* No need to actually call the function.
       Simply change the next instr pointer to the first instr of the function.
       This will put the body of the function right as the next instr in the
       translated code */
    ts->next_instr = (unsigned char*)call_target;
    
    /* put the target into the tcache so later calls can use the translated
       code */
    fbt_ccache_add_entry(ts->tld, (void*)call_target, ts->transl_instr);
    
    return OPEN;
  }

  /* target is already translated */
  PRINT_DEBUG("translated call_target: %p", transl_target);

  /* write: jump instruction to translated target */
  JMP_REL32(transl_addr, (int32_t)transl_target);
  
  PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i",
                           transl_addr - ts->transl_instr);
  ts->transl_instr = transl_addr;
  return CLOSE;
}

enum translation_state action_call_indirect(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
  unsigned char* transl_addr = ts->transl_instr;
  int length = ts->next_instr - ts->cur_instr;
  int prefix_length = 0;

  PRINT_DEBUG_FUNCTION_START("action_call_indirect(*addr=%p, *transl_addr=%p," \
                             " length=%i)", ts->cur_instr, ts->transl_instr,
                             ts->next_instr - ts->cur_instr);

#if defined(FBT_STATISTIC)
  fbt_nr_translated_call_ind++;
  /* WARNING: this statement destroys the FLAGS */
  INCL_M64(ts->transl_addr, (int32_t)&fbt_nr_ind_calls);
#endif

  /* write: push original EIP */
  PUSHL_IMM32(transl_addr, (ulong_t)ts->next_instr);
  PRINT_DEBUG("original eip: %p", addr + length);

  /*
   * check for prefixes:
   * we handle only segment override prefixes, all others produce an error
   */
  while (HAS_PREFIX(*addr)) {
    /* we don't handle 16bit calls and strange prefixes */
    assert(*addr!=PREFIX_LOCK && *addr!=PREFIX_REPNEZ && *addr!=PREFIX_MISC &&
           *addr!=PREFIX_ADDR_SZ_OVR && *addr!=PREFIX_OP_SZ_OVR &&
           *addr!=PREFIX_SIMD_EXT);
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
    fbt_memcpy(transl_addr, (first_byte_after_opcode + 1), length - 2 -
               prefix_length);
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
      PRINT_DEBUG("We have a ModR/M and maybe SIB that uses the ESP register" \
                  "- we need to fix this mess! ");
      /* we use ESP but not with an 8bit displacement!
         either the SIB is scaled or we have no/wrong
         displacement in the modrm
         Maybe we can latter add 4 special handling for
         4b displacement (change offset for displacement)
         and no displacement (change to 1b displacement).
         Currently ONLY 1b displacement is handled!
         TODO: remove assertion and add code for 4b displacement
      */
      assert(((modrm>>6)&0x3)==1);
      /* just change the following displacement and add 4bytes to it! */
      *(transl_addr-1) = (*(transl_addr-1))+4;
    }
  }

#if defined(VERIFY_CFTX)
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_addr, (ts->tld->stack-2));
  MOV_IMM32_ESP(transl_addr, (ts->tld->stack-2));
  PUSHAD(transl_addr);
  PUSHL_IMM32(transl_addr, CFTX_CALL_IND);
  PUSHL_RM32_SIB_IMM8(transl_addr, 0x74, 0x24, 36); /* pushl 40(%esp) target */ 
  PUSHL_IMM32(transl_addr, (int32_t)addr);
  PUSHL_IMM32(transl_addr, (int32_t)ts->tld); /* pushl tld */
  CALL_REL32(transl_addr, (int32_t)&fbt_check_transfer);
  LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x10); /* leal 16(%esp), %esp */
  POPAD(transl_addr);
  POPL_ESP(transl_addr);
#endif

#if !defined(ICF_PREDICT)
  /* write: jump instruction to trampoline */
  JMP_REL32(transl_addr, (ulong_t)(ts->tld->opt_icall_trampoline));
#else  /* ICF_PREDICT */
  if (ts->tld->icf_predict == NULL)
    fbt_allocate_new_icf_predictors(ts->tld);
  struct icf_prediction *pred = ts->tld->icf_predict;
  ts->tld->icf_predict = pred->pred.next;

  pred->pred.src = transl_addr;
  pred->origin1 = NULL;
  pred->dst1 = NULL;
  
  PUSHL_EBX(transl_addr);
  /* movl 0x4(%esp), %ebx - load target addr */
  MOVL_IMM8RM32SIB_R32(transl_addr, 0x5c, 0x24, 0x04);
  /* cmpl (pred->origin1), %ebx */
  CMPL_R32_M32(transl_addr, 0x1d, (int32_t)&(pred->origin1));
  JNE_I8(transl_addr, 0x0); /* offset  */
  char *patchloc = ((char*)transl_addr)-1;
  POPL_EBX(transl_addr);
  LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
  JMP_IND_M32(transl_addr, (int32_t)&(pred->dst1));
  /* NO HIT, we need to fix the prediction */
  *patchloc = (char)(((int32_t)transl_addr)-(((int32_t)patchloc)+1));
  MOVL_IMM32_MEM32(transl_addr, 0x05, pred, (int32_t)(ts->tld->stack-11));
  JMP_REL32(transl_addr, (int32_t)(ts->tld->opt_icall_predict_fixup));
#endif  /* ICF_PREDICT */
  
  ts->transl_instr = transl_addr;
  PRINT_DEBUG_FUNCTION_END("-> close");
  return CLOSE;
}

enum translation_state action_sysenter(struct translate *ts) {
  unsigned char* transl_addr = ts->transl_instr;
#ifdef DEBUG
  unsigned char *addr = ts->cur_instr;
  int length = ts->next_instr - ts->cur_instr;
  PRINT_DEBUG_FUNCTION_START("action_sysenter(*addr=%p, *transl_addr=%p, " \
                             "length=%i)", addr, transl_addr, length);
#endif
#if defined(AUTHORIZE_SYSCALLS)
  /* store location of this syscall */
  MOVL_IMM32_MEM32(transl_addr, 0x05, ts->cur_instr,
                   &(ts->tld->syscall_location));
#endif
  /* write: jump instruction to trampoline */
  JMP_REL32(transl_addr, (ulong_t)(ts->tld->sysenter_trampoline));
  
  ts->transl_instr = transl_addr;
  PRINT_DEBUG_FUNCTION_END("-> close");
  return CLOSE;
}

#if !defined(STACKPROTECTION)
enum translation_state action_ret(struct translate *ts) {
  unsigned char *addr = ts->cur_instr;
  unsigned char* transl_addr = ts->transl_instr;
  unsigned char *first_byte_after_opcode = ts->first_byte_after_opcode;
#ifdef DEBUG
  int length = ts->next_instr - ts->cur_instr;
  PRINT_DEBUG_FUNCTION_START("action_ret(*addr=%p, *transl_addr=%p, length=%i)",
                             addr, transl_addr, length);

  /* ret plus evtl imm16 - no opcode prefixes */
  assert((!HAS_PREFIX(*addr) && ((*addr==0xC2 && length==3) ||          \
                                 (*addr==0xC3 && length==1))) ||        \
         (*addr==PREFIX_MISC &&  ((*(addr+1)==0xC2 && length==4) ||     \
                                  (*(addr+1)==0xC3 && length==2))));

  /* see: http://readlist.com/lists/gcc.gnu.org/gcc-help/1/8765.html */
  if (!(!HAS_PREFIX(*addr) && ((*addr==0xC2 && length==3) || \
                               (*addr==0xC3 && length==1))) \
      && *addr==PREFIX_MISC) {
    PRINT_DEBUG("Useless REPZ Prefix found (and removed) in RET (ugly gcc " \
                "hack for Athlon and K8)");
  }
#endif
  // useless prefix found, skip to the real ret instr
  if (*addr == PREFIX_MISC) {
    addr++;
  }

#if defined(INLINE_CALLS)
  /* are we currently inlining a function
   * and do we need to unwrap the current stack frame
   * (e.g. fast return to the callee instead of going
   * through the ind_jump feature)
   */
  if (ts->inline_call_RIP != NULL) {
    /* restore the stack to the point before the call */
    if (*addr == 0xC2) {
      /* leal 4+$imm(%esp), %esp */
      int16_t rem_bytes = *((int16_t*)first_byte_after_opcode);
      PRINT_DEBUG("we must remove additional bytes: %d\n", rem_bytes);
      if (rem_bytes < 0) {
        fbt_suicide_str("Ret removes a negative amount of bytes, this is "
                        "illegal! (fbt_actions.c)\n");
      }
      LEAL_IMM32RM32_R(transl_addr, 0xa4, 0x24, 0x04+(int32_t)(rem_bytes));
    } else {
      /* leal 4(%esp), %esp */
      LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x04);  /* leal 4(%esp), %esp */
    }

    ts->next_instr = ts->inline_call_RIP;
    ts->inline_call_RIP = NULL;

#if defined(FBT_STATISTIC)
    fbt_nr_trans_inlined_calls++;
    INCL_M64(transl_addr, (int32_t)&fbt_nr_inlined_calls);
#endif
    PRINT_DEBUG_FUNCTION_END("-> open, inlined, transl_length=%i",
                             transl_addr - ts->transl_instr);
    
    ts->transl_instr = transl_addr;
    return OPEN;
  }
#endif

#if defined(VERIFY_CFTX)
  /* switch to secured stack */
  MOV_ESP_MEM32(transl_addr, (ts->tld->stack-2));
  MOV_IMM32_ESP(transl_addr, (ts->tld->stack-2));
  PUSHAD(transl_addr);
  PUSHL_IMM32(transl_addr, CFTX_RET);
  PUSHL_RM32_SIB_IMM8(transl_addr, 0x74, 0x24, 36); /* pushl 40(%esp) target */ 
  PUSHL_IMM32(transl_addr, (int32_t)addr);
  PUSHL_IMM32(transl_addr, (int32_t)ts->tld); /* pushl tld */
  CALL_REL32(transl_addr, (int32_t)&fbt_check_transfer);
  LEAL_IMM8RM32_R(transl_addr, 0x64, 0x24, 0x10); /* leal 16(%esp), %esp */
  POPAD(transl_addr);
  POPL_ESP(transl_addr);
#endif

  /*
   * This is the simple case, we just replace the ret
   * with an indirect jump that translates the control flow
   * back to the callee (no optimization)
   */
  int32_t jmp_target = 0;
  if (*addr == 0xc2) {
    /* this ret wants to pop some bytes of the stack */
    int16_t rem_bytes = *((int16_t*)first_byte_after_opcode);
    PRINT_DEBUG("we must remove additional bytes: %d\n", rem_bytes);
    if (rem_bytes < 0) {
      fbt_suicide_str("Ret removes a negative amount of bytes, this is "
                      "illegal! (fbt_actions.c)\n");
    }
    PUSHL_IMM32(transl_addr, (int32_t)rem_bytes);
    jmp_target = (int32_t)(ts->tld->opt_ret_remove_trampoline);
  }

  if (*addr == 0xc3) {
    jmp_target = (int32_t)(ts->tld->opt_ret_trampoline);
  }
  
  /* write: jump instruction to trampoline */
  JMP_REL32(transl_addr, jmp_target);

  PRINT_DEBUG_FUNCTION_END("-> close, transl_length=%i",
                           transl_addr - ts->transl_instr);
  ts->transl_instr = transl_addr;
  return CLOSE;
}
#endif
