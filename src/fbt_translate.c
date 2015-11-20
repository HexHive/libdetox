/**
 * @file fbt_translate.c
 * This modules translates one code region and puts the result into the code
 * cache.
 *
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-01-14 21:34:20 +0100 (Sat, 14 Jan 2012) $
 * $LastChangedDate: 2012-01-14 21:34:20 +0100 (Sat, 14 Jan 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1171 $
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
#include "fbt_translate.h"

#ifdef SECU_ENFORCE_NX
#endif

#include <assert.h>

#include "fbt_actions.h"
#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "fbt_x86_opcode.h"
#if defined(SEL_DEBUG)
#include "fbt_sdbg.h"
#endif

#include "fbt_opcode_tables.h"

#if defined(FBT_STATISTIC)
#include "fbt_statistic.h"
#endif

#if defined(ONLINE_PATCHING)
#include "patching/fbt_patching.h"
#endif

#if defined(INLINE_CALLS)
#define INLINE_MAX_LENGTH 64
#endif

/* bit test masks for ModR/M byte */
#define MASK_MOD 0xC0
#define MASK_REG 0x38
#define SHIFT_REG 3
#define MASK_RM  0x07

/**
 * Returns the operandSize for specific operandFlags and a given prefix.
 * @param operandFlags the operandFlags for this instruction
 * @param prefix the prefix for this instruction or 0x0
 * @return number of bytes for this operand flag combination
 */
static ulong_t fbt_operand_size(ulong_t operandFlags, unsigned char prefix);

#if defined(INLINE_CALLS)
/**
 * Checks if the current call is inlineable
 * @param ts translate struct. This translate struct must currently point to a
 * valid call instruction.
 * @return a boolean if this function is inlineable or not
 */
static ulong_t check_inline(struct translate *ts);
#endif
#if defined(SECU_ENFORCE_NX) || defined(INFO_OUTPUT)
/**
 * Checks if the translation of the given address is allowed.
 * If the translation is not allowed then fastBT will fail/quit.
 * @param orig_address instruction pointer that will be checked
 * @param info pointer to all mem_info structures that will be checked.
 */
static void check_transl_allowed(void* orig_address, struct mem_info *info);
#endif

#ifdef LMEM
void *fbt_lmem_translate_noexecute(struct thread_local_data *tld,
                              void *orig_address);
#endif

void *fbt_translate_noexecute(struct thread_local_data *tld,
                              void *orig_address) {
  #ifdef ONLINE_PATCHING
  return fbt_online_patching_translate_noexecute(tld, orig_address);
  #endif

#ifdef LMEM
  return fbt_lmem_translate_noexecute(tld, orig_address);
#endif
                              
  PRINT_DEBUG_FUNCTION_START("translate_noexecute(*tld=%p, *orig_address=%p)",
                             tld, orig_address);

  assert(tld != NULL);

  /* if the target is already translated then we return the cached version  */
  void *already_translated = fbt_ccache_find(tld, orig_address);
  if (already_translated != NULL) {
    PRINT_DEBUG_FUNCTION_END("already translated -> %p",
                             already_translated);
    return already_translated;
  }

  struct mem_info *code_block = tld->chunk;
  /* make sure that we don't translate translated code */
  while (code_block != NULL) {
    if ((orig_address >= code_block->ptr) &&
        (orig_address < (code_block->ptr + code_block->size))) {
      llprintf("Translating translated code: %p (%p len: 0x%x (%p) type: %d (syscall=%d))\n",
               orig_address, code_block->ptr, code_block->size, code_block, 
               code_block->type, MT_SYSCALL_TABLE);
      fbt_suicide(255);
    }
    code_block = code_block->next;
  }

#if defined(SECU_ENFORCE_NX)
  /* Check if the memory address to translate lies in an executable
     section of a loaded library or the executable itself. We only allow
     execution if this is the case. */
  struct mem_info curr_section;
  check_transl_allowed(orig_address, &curr_section);
#endif
  
  /* we need to translate TU, add to ccache index,
     jump to the translated code */
  enum translation_state tu_state = NEUTRAL;

  int bytes_translated = 0;
  struct translate *ts = &(tld->trans);
  ts->next_instr = (unsigned char*)orig_address;

  /* check if more memory needs to be allocated for tcache */
  if ((long)(ts->code_cache_end - ts->transl_instr) < MAX_BLOCK_SIZE) {
    PRINT_DEBUG("Not enough memory for new code block - allocating more!");
    unsigned char *prev_transl_instr = ts->transl_instr;
    
    fbt_allocate_new_code_cache(tld);

    /* add a jmp connect old and new tcache memory blocks */
    if (prev_transl_instr != NULL) {
      JMP_REL32(prev_transl_instr, ts->transl_instr);
    }
  }
  PRINT_DEBUG("tld->ts.transl_instr: %p", ts->transl_instr);

#if defined(SEL_DEBUG)
  if (tld->sdbg->command.operation != CCACHE_FLUSH) {
    /* add entry to ccache index */
    fbt_ccache_add_entry(tld, orig_address, ts->transl_instr);
  }
#else
  /* add entry to ccache index */
  fbt_ccache_add_entry(tld, orig_address, ts->transl_instr);
#endif


  /* look up address in translation cache index */
  void *transl_address = ts->transl_instr;

#ifdef TRACK_BASIC_BLOCKS
  struct basic_block_node *basic_block = fbt_smalloc(
    tld,
    sizeof(struct basic_block_node)
  );
  basic_block->start = ts->transl_instr;
  basic_block->original_start = orig_address;
  basic_block->length = 0;
  basic_block->original_length = 0;
#endif

  /*
#if defined(SEL_DEBUG)
  MOVB_IMM8_MEM8(ts->transl_instr, 0x05,  1, (int32_t)tld->sdbg->flag);
#endif
*/
  /* we translate as long as we
     - stay in the limit (MAX_BLOCK_SIZE)
     - or if we have an open TU (could happen if we are translating a call or
       so)
     - or we are inlining, then we first need to finish it and spill all the
       frames! we need to check for inlining here because inlining could
       overrun the MAX_BLOCK _SIZE (this can happen if we translate short
       instructions in the inlined code to longer instructions in the
       translated code - but thats what the guard is there for!) */
#ifdef INLINE_CALLS
  while (((bytes_translated < MAX_BLOCK_SIZE) && (tu_state == NEUTRAL)) ||
         (tu_state == OPEN) || (ts->inline_call_RIP!=NULL)) {
#else
  while (((bytes_translated < MAX_BLOCK_SIZE) && (tu_state == NEUTRAL)) ||
         (tu_state == OPEN)) {
#endif  /* INLINE_CALLS */
    /* translate an instruction */
    /* Check if we are still within the boundaries of the current section.
       Otherwise, do a complete check of the address to translate and
       update the current section information. */
#ifdef SECU_ENFORCE_NX
    if (((void*) ts->next_instr < curr_section.node.addr_begin)
        || ((void*) ts->next_instr >= curr_section.node.addr_end)) {
      check_transl_allowed(ts->next_instr, &curr_section);
    }
#endif /* SECU_ENFORCE_NX */

#if defined(FBT_CHECK_TRANSLATED)
    // current instr already translated?
    unsigned char *dst=NULL;
    if (((dst=tcache_find(tld, ts->next_instr))!=NULL) &&
        dst!=ts->transl_instr && ts->inlined_frames==NULL) {
      JMP_REL32(ts->transl_instr, (int32_t)dst);
      tu_state = CLOSE;
      break;
    }
#endif

    fbt_disasm_instr(ts);
    PRINT_DEBUG("translating a '%s'", ts->cur_instr_info->mnemonic);

    unsigned char *old_transl_instr = ts->transl_instr;
#ifdef DEBUG
    unsigned char *old_cur_instr = ts->cur_instr;
    unsigned char *old_next_instr = ts->next_instr;
#endif
#ifdef DUMP_GENERATED_CODE
    unsigned char *old_transl_instr_dump = ts->transl_instr;
    unsigned char *old_next_instr_dump = ts->next_instr;
#endif

#if defined(INLINE_CALLS)
    /* if the current instruction is a call, then we check if it is inlinable */
    if (ts->cur_instr_info->opcode.handler == action_call) {
      unsigned int function_length;
      // inlinable ?
      if ((function_length = check_inline(ts)) &&
          ((bytes_translated + function_length) < MAX_BLOCK_SIZE)) {
        // if yes, then we construct the static call frame (with return address)
        ts->inline_call_RIP = ts->next_instr;
      }
    }
#endif

#if defined(SEL_DEBUG)
    sdbg_handle_flush_and_breaks(tld);
#endif

#if defined(TRACK_BASIC_BLOCKS)
    basic_block->original_length += (ts->next_instr - ts->cur_instr);
#endif  

#if defined(TRACK_INSTRUCTIONS)
    fbt_track_instruction(tld, ts->transl_instr, ts->cur_instr);
#endif

    /* call the action specified for this instruction */
    tu_state = ts->cur_instr_info->opcode.handler(ts);

#if defined(SEL_DEBUG)
    sdbg_handle_watchpoints(tld);
#endif

    bytes_translated += (ts->transl_instr - old_transl_instr);

#if defined(FBT_STATISTIC)
    fbt_nr_translated_instr++;
#endif
    PRINT_DEBUG("orig_ins_addr: %p (%db)", ts->cur_instr, (old_next_instr -
                                                           old_cur_instr));
    PRINT_DEBUG("transl_insn_addr: %p (%db)", old_transl_instr,
                (ts->transl_instr - old_transl_instr));
    PRINT_DEBUG("tu_state: %d\n", tu_state); /* debug */
    DUMP_CODE(ts, (old_next_instr_dump - ts->cur_instr),
              (ts->transl_instr - old_transl_instr_dump));
  }

  /* if the TU was finished because the number of instructions hit the limit, we
     have to add glue code to the TU */
  if (tu_state != CLOSE) {
    if (tu_state != CLOSE_GLUE) {
      PRINT_DEBUG("finishing TU because instruction limit was hit, invoking"
                  "translation function on %p", ts->next_instr);
    } else {
      PRINT_DEBUG("closing TU upon request, invoking translation function on"
                  " %p", ts->next_instr);
    }
    /* takes care of the flag register! */
    /* write: jump to trampoline that does chaining if next block needs to be
       translated or does fallthrough otherwise */
    struct trampoline *trampo = fbt_create_trampoline(tld,
                                                      (void*)ts->next_instr,
                                                      ts->transl_instr+1,
                                                      ORIGIN_RELATIVE);
    JMP_REL32(ts->transl_instr, trampo->code);
  }

  /* make sure that we always stay in the limits, even if we overwrite the
     MAX_BLOCK_SIZE due to some optimizations */
  assert(bytes_translated < TRANSL_GUARD);
  assert((void*)(ts->transl_instr) < (void*)(ts->code_cache_end +
                                             TRANSL_GUARD));

  PRINT_DEBUG_FUNCTION_END("-> %p,   next_tu=%p (len: %d)", transl_address,
                           ts->next_instr, bytes_translated);
  
  /* Store the basic block */
#ifdef TRACK_BASIC_BLOCKS
  basic_block->next = tld->basic_blocks;
  basic_block->length = bytes_translated;
  tld->basic_blocks = basic_block;
#endif
  
  return transl_address;
}


void fbt_disasm_instr(struct translate *ts) {
  const struct ia32_opcode *opcode, *opcode_table = opcode_table_onebyte;
  int bytes_argument;

  unsigned char *cur = (ts->cur_instr = ts->next_instr);
  ts->num_prefixes = 0;

  /* 0x1 - operand overwrite */
  unsigned char prefix = 0;

#ifdef __LP64__
  unsigned char rex = 0;
#endif

  while (1) {
    opcode = opcode_table + (unsigned int)*cur;

    if ((*cur==PREFIX_OP_SZ_OVR || *cur==PREFIX_REPNEZ || *cur==PREFIX_MISC) &&
        (*(cur+1)==0x0F) && /* opcode esc */
        /* ensure that we can escape to a table and the destination is valid */
        (OPCODETABLE(opcode->tableFlags) &&
         ((opcode->opcode.table + *(cur+2))->opcode.handler)!=0)) {
      /* this is an opcode escape -> we have to perform a lookup in an
         additional table */
      PRINT_DEBUG("opcode escape found to 3byte opcode: %x", *cur);
      /* there are some 3byte opcodes which start with a prefix and then escape
         through 0x0f we take care of them _here_ otherwise, we just go back to
         the first table and continue our translation */
      opcode_table = opcode->opcode.table;
      cur+=2; /* skip the 0x0f escape byte */
      continue;  /* do one more iteration */
      /* check for OPCODE_ESC */
    } else if ((opcode->tableFlags & OPCODE_ESC) != 0) {
      /* this is an opcode escape -> we have to perform a lookup in an
         additional table */
      PRINT_DEBUG("opcode escape found: %x", *cur);
      opcode_table = opcode->opcode.table;
      cur++;
      continue;  /* do one more iteration */
    } else if ((opcode->tableFlags & OPCODE_PREFIX_ESC) != 0) {
      /* this is a prefix -> we have to perform a lookup in an additional
         table */
      PRINT_DEBUG("prefix found: %x", *cur);
      ts->num_prefixes++;
      /* for performance reasons we _now_ only support 1 out of ADDR or OP SZ
         OVR prefix. more can be added. */
      if (!(ts->num_prefixes<=1 || ((prefix!=PREFIX_ADDR_SZ_OVR) &&
                                    (prefix!=PREFIX_OP_SZ_OVR)))) {
        /* we only support one prefix atm */
        fbt_suicide_str("Only one prefix supported (disasm_instr: "
                        "fbt_disassemble.c)\n");
      }
      prefix = *cur;
      /* go back to the first table if it is not a special escape prefix! */
      opcode_table = opcode_table_onebyte;
      cur++;
      continue;  /* do one more iteration */

#ifdef __LP64__
    } else if (*cur>=0x40 && *cur<=0x4f && opcode_table==opcode_table_onebyte) {
      rex = *cur;
      cur++;
      continue;
#endif
    } else {
      /* this was the last byte of the opcode */
      ts->first_byte_after_opcode = cur + 1;

      /* check if this instruction has any kind of ModRM byte */
      if (anyModRMOp(opcode) || (opcode->tableFlags&OPCODE_MODRM_ESC) ||
          (opcode->tableFlags&OPCODE_FPU_ESC)) {
        cur++;
        bytes_argument = 0;

        /* check for OPCODE_RM */
        if (opcode->tableFlags&OPCODE_MODRM_ESC) {
          /* this opcode extends into the reg bits of the ModR/M byte */
          /* get the opcode from the group table */
          PRINT_DEBUG("opcode table: %p -> %p (%p)\n", opcode,
                      opcode->opcode.table,
                      opcode->opcode.table + ((*cur & MASK_REG) >> SHIFT_REG));
          opcode = opcode->opcode.table + ((*cur & MASK_REG) >> SHIFT_REG);
          /* check for FPU_ESC */
        } else if (opcode->tableFlags&OPCODE_FPU_ESC) {
          /* this opcode extends into a fpu table */
          /* get the opcode from the fpu table */
          opcode = opcode->opcode.table + (unsigned int)*cur;
        }
        if ((opcode->tableFlags&HAS_MODRM) && (*cur & MASK_MOD) != 0xC0) {
          /* this instruction has a modRM byte and it refers to a memory
             location */
          if (prefix!=PREFIX_ADDR_SZ_OVR) {
            if ((*cur & MASK_RM) == 0x04) {
              /* there follows a SIB byte (SIB bytes only follow in 32bit
                 mode! */
              if ((*cur & MASK_MOD) == 0x00 && (*(cur+1) & 0x07) == 0x05) {
                /* after the SIB there follows a 32bit displacement */
                bytes_argument += 5;
              } else {
                bytes_argument++;
              }
            }
            if ((*cur & MASK_MOD) == 0x00 && (*cur & MASK_RM) == 0x05) {
              /* mod bits are 00, R/M bits are 101 -> another case of 32bit
                 displacement */
              bytes_argument += 4;
            } else if ((*cur & MASK_MOD) == 0x40) {
              /* mod bits are 01 -> 8bit displacement */
              bytes_argument++;
            } else if ((*cur & MASK_MOD) == 0x80) {
              /* Mod bits are 10 -> 32bit displacement */
              bytes_argument += 4;
            }
          } else {
            /* we have 16bit addr mode (prefix override)!! */
            PRINT_DEBUG("Prefix found: ADDR_SZ_OVR!");
            if ((*cur & MASK_MOD) == 0x00 && (*cur & MASK_RM) == 0x06) {
              /* mod bits are 00, R/M bits are 110 -> another case of 16bit
                 displacement */
              bytes_argument += 2;
            } else if ((*cur & MASK_MOD) == 0x40) {
              /* mod bits are 01 -> 8bit displacement */
              bytes_argument++;
            } else if ((*cur & MASK_MOD) == 0x80) {
              /* Mod bits are 10 -> 16bit displacement */
              bytes_argument += 2;
            }
          }
          cur += bytes_argument;
        }
      }

      /* set the action */
      ts->cur_instr_info = opcode;
      assert(!OPCODETABLE(opcode->tableFlags));

      ts->dest_operand_size = 0;
      ts->src_operand_size = 0;
      ts->aux_operand_size = 0;
      
#ifdef __LP64__
      if (rex && (rex&REXW)) {
        /* rex.b size extenstion */
        /* if ((MASK_ARG & opcode->opcode_flags)==ARG_D) {
          cur += 8;
          } else { */
        llprintf("unhandled rexw prefix!\n");
        /* } */
      } else {
#endif
        /* read info regarding immediate arguments (imms, code offset, etc.) */
        if (hasImmOp(opcode->destFlags)) {
          ts->dest_operand_size = fbt_operand_size(opcode->destFlags, prefix);
          cur += ts->dest_operand_size;  
        }
        if (hasImmOp(opcode->srcFlags)) {
          ts->src_operand_size = fbt_operand_size(opcode->srcFlags, prefix);
          cur += ts->src_operand_size;  
        }
        if (hasImmOp(opcode->auxFlags)) {
          ts->aux_operand_size = fbt_operand_size(opcode->auxFlags, prefix);
          cur += ts->aux_operand_size;  
        }
#ifdef __LP64__
      }
#endif
    }

    /* if we reach this point we are done */
    break;
  }

  /* now we know how many bytes the current instructions takes */
  ts->next_instr = cur+1;
}

static ulong_t fbt_operand_size(ulong_t operandFlags, unsigned char prefix) {
  unsigned char rprefix = 0x0;
  if (prefix==PREFIX_OP_SZ_OVR && ((operandFlags&ADDRM_I)==ADDRM_I))
    rprefix = prefix;
  if (prefix==PREFIX_ADDR_SZ_OVR &&
      (((operandFlags&ADDRM_A)==ADDRM_A) || ((operandFlags&ADDRM_J)==ADDRM_J) ||
       ((operandFlags&ADDRM_O)==ADDRM_O)))
    rprefix = PREFIX_OP_SZ_OVR; // use simple override

  switch (operandFlags & OPT_MASK) {
  case OPT_b:
    return 1;
  case OPT_w:
    return 2;
  case OPT_d:
  case OPT_fs:
    return 4;
  case OPT_pi:
  case OPT_q:
  case OPT_fd:
    return 8;
  case OPT_s:
  case OPT_fe:
  case OPT_fp:
    return 10;
  case OPT_dq:
  case OPT_ps:
  case OPT_pd:
  case OPT_ss:
  case OPT_sd:
    return 16;
  case OPT_fv:
    return 28;
  case OPT_fst:
    return 108;
    /* special handling if OP_SZ is present */
  case OPT_c:
    if (rprefix==PREFIX_OP_SZ_OVR) return 1;
    return 2;
  case OPT_a:
    if (rprefix==PREFIX_OP_SZ_OVR) return 4;
    return 8;
  case OPT_z:
    if (rprefix==PREFIX_OP_SZ_OVR) return 2;
    return 4;
  case OPT_v:
    /* TODO: special handling for 64bit x86_64 ! */
    if (rprefix==PREFIX_OP_SZ_OVR) return 2;
    return 4;
  case OPT_p:
    /* 32bit ptr (16b addr mode), 48bit ptr (32b addr mode) or 80b ptr (64b addr
       mode) */
  case OPT_si:
    /* doubleword register - unhandled */
    fbt_suicide_str("Doubleword register, unhandled (fbt_operand_size: "
                    "fbt_translate.c)\n");
    return -1;
  }
  return 0;
}

#if defined(SECU_ENFORCE_NX)
static void check_transl_allowed(void* orig_address, struct mem_info *info) {
  if (fbt_memprotect_execquery(orig_address)) {
    if (!fbt_memprotect_info(orig_address, info)) {
      fbt_suicide_str("Unknown error in check_transl_allowed"
                      " (check_transl_allowed: fbt_translate.c).\n");
    }
  } else {
    if (fbt_memprotect_info(orig_address, info)) {
      llprintf("Tried to translate code at address %p in %s, section %s, "
               "which is not marked as executable.\n",
               orig_address, info->obj_name, info->sec_name);
    } else {
      llprintf("Tried to translate code at address %p, which was determined"
               " not to be in a section of any loaded shared library or the"
               " executable.\n", orig_address);
    }
    fbt_suicide_str("Exiting Program! If you believe this occurs in error,"
                    " disable the -DSECU_ENFORCE_NX CFLAG in the libdetox "
                    "Makefile (check_transl_allowed: fbt_translate.c).\n");
  }
}
#endif

#if defined(INLINE_CALLS)
static ulong_t check_inline(struct translate *ts) {
  /* static translate struct for internal use */
  struct translate myts;
  fbt_memcpy(&myts, ts, sizeof(struct translate));
  unsigned int function_length = 0;

  /* extract relative call target from call. */
  /* we assume that this call will not have any prefixes */
  assert(!HAS_PREFIX(*(myts.cur_instr)));
  myts.next_instr = (unsigned char*)(*((int32_t*)(myts.cur_instr + 1)) +
                                     (int32_t)myts.cur_instr + 5);

  while (function_length < INLINE_MAX_LENGTH) {
    fbt_disasm_instr(&myts);
    function_length += (myts.next_instr - myts.cur_instr);

    if (myts.cur_instr_info->opcode.handler != action_copy) {
      /* if we found a ret, then we are done, let's return the length of this
         function (in bytes) */
      if (myts.cur_instr_info->opcode.handler == action_ret)
        return function_length;
      
      /* if (myts.action == action_call) ...
         if we have a call, then we might go into recursion and inline this
         call as well. But if we want to support recursion then we need to add
         dynamic inline information structs (e.g., a linked list) and then we
         would also need to implement a free function for unused inline
         structs. As we only inline 64 bytes usually this is not worth it. */

      /* some action we don't know jack about - stop inlining and bail out */
      return 0;
    }
  }
  /* if the function is too long we might hit that */
  return 0;
}
#endif
 
