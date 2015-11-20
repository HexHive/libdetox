/**
 * @file fbt_patching_translate.h
 * Implementation file for the online patching-specific translation
 * function
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 1134 $
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

#if defined(ONLINE_PATCHING)

#include <assert.h>

#include "../libfastbt.h"
#include "../fbt_asm_macros.h"
#include "../fbt_algorithms.h"
#include "../fbt_actions.h"
#include "../fbt_mem_mgmt.h"
#include "../fbt_translate.h"
#include "../fbt_debug.h"
#include "../fbt_libc.h"
#include "../fbt_translate.h"
#include "../fbt_code_cache.h"
#include "../fbt_datatypes.h"
#include "../fbt_x86_opcode.h"
#include "../fbt_mem_mgmt.h"
#include "../fbt_syscalls_impl.h"
#include "../fbt_trampoline.h"
#include "../fbt_llio.h"
#include "../fbt_restart_transaction.h"

#include "fbt_patching.h"

#include "fbt_patching_translate.h"

void *fbt_online_patching_translate_noexecute(
    struct thread_local_data *tld,
    void *orig_address)
{
  PRINT_DEBUG_FUNCTION_START("op:translate_noexecute(*tld=%p, *orig_address=%p)",
                             tld, orig_address);

  assert(tld != NULL);
  
  struct patching_information *pi = tld->patching_information;

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
        (orig_address <= (code_block->ptr + code_block->size))) {
      llprintf("Translating translated code: %p (%p len: 0x%x (%p))\n",
               orig_address, code_block->ptr, code_block->size, code_block);
      fbt_suicide(255);
    }
    code_block = code_block->next;
  }
  
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

  /* add entry to ccache index */
  fbt_ccache_add_entry(tld, orig_address, ts->transl_instr);

  /* look up address in translation cache index */
  void *transl_address = ts->transl_instr;

#ifdef TRACK_BASIC_BLOCKS
  struct basic_block_node *basic_block = fbt_smalloc(
    tld,
    sizeof(struct basic_block_node)
  );
  basic_block->start = ts->transl_instr;
  basic_block->length = 0;
#endif  

  /* we translate as long as we
     - stay in the limit (MAX_BLOCK_SIZE)
     - or if we have an open TU (could happen if we are translating a call or
       so)
     - or we are inlining, then we first need to finish it and spill all the
       frames! we need to check for inlining here because inlining could
       overrun the MAX_BLOCK _SIZE (this can happen if we translate short
       instructions in the inlined code to longer instructions in the
       translated code - but thats what the guard is there for!) */
  while (((bytes_translated < MAX_BLOCK_SIZE) && (tu_state == NEUTRAL)) ||
         (tu_state == OPEN)) {
    /* translate an instruction */
    /* Check if we are still within the boundaries of the current section.
       Otherwise, do a complete check of the address to translate and
       update the current section information. */

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

    ts->original_addr = ts->next_instr;

    struct change *current_change = pi != NULL ? fbt_online_patching_find_change(pi, ts->next_instr) : NULL;
    if (current_change != NULL) {   
      PRINT_DEBUG_FUNCTION_START("online-patching");
      
      int num_changed = current_change->original_instr_len;
      PRINT_DEBUG("num_changed = %d\n", num_changed);
      
      unsigned char *saved_next = ts->next_instr + num_changed;
      PRINT_DEBUG("saved_next = %x (%x + %x)\n", saved_next, ts->next_instr, num_changed);
      PRINT_DEBUG("*** FOUND A PATCH ***\n");
      ts->next_instr = (unsigned char *)current_change->machine_code;

      PRINT_DEBUG("Disassembling.\n");
      
      if (current_change->num_bytes > 0) {
        if (current_change->generation_addr != 0) {
            ts->original_addr = (unsigned char *)current_change->generation_addr;
        }
        ts->next_patch_instr = (unsigned char *)current_change->offset + current_change->original_instr_len;        
        PRINT_DEBUG("original_addr = %x (%x)\n", ts->original_addr, current_change->generation_addr);        
        PRINT_DEBUG("first byte = %x\n", *current_change->machine_code);
      
        fbt_disasm_instr(ts);

        PRINT_DEBUG("Calling handler\n");

        tu_state = ts->cur_instr_info->opcode.handler(ts);

        if (ts->cur_instr_info->opcode.handler == action_copy) {
          ts->next_instr = saved_next;
        }

        PRINT_DEBUG("ts->next_instr = %x\n", ts->next_instr);
      } else {
        ts->next_instr = saved_next;
      }
            
      bytes_translated += num_changed;

      PRINT_DEBUG_FUNCTION_END("");
      
    } else {
      fbt_disasm_instr(ts);
      ts->next_patch_instr = ts->next_instr;

      unsigned char *old_transl_instr = ts->transl_instr;
  #ifdef DEBUG
      unsigned char *old_cur_instr = ts->cur_instr;
      unsigned char *old_next_instr = ts->next_instr;
  #endif
  #ifdef DUMP_GENERATED_CODE
      unsigned char *old_transl_instr_dump = ts->transl_instr;
      unsigned char *old_next_instr_dump = ts->next_instr;
  #endif    
    
      tu_state = ts->cur_instr_info->opcode.handler(ts);
      bytes_translated += (ts->transl_instr - old_transl_instr);
      
      PRINT_DEBUG("orig_ins_addr: %p (%db)", ts->cur_instr, (old_next_instr -
                                                             old_cur_instr));
      PRINT_DEBUG("transl_insn_addr: %p (%db)", old_transl_instr,
                  (ts->transl_instr - old_transl_instr));
      PRINT_DEBUG("tu_state: %d\n", tu_state); /* debug */
      DUMP_CODE(ts, (old_next_instr_dump - ts->cur_instr),
                (ts->transl_instr - old_transl_instr_dump));      
    }

#if defined(FBT_STATISTIC)
    fbt_nr_translated_instr++;
#endif
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
#endif