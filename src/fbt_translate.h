/**
 * @file fbt_translate.h
 * This module translates one code region and puts the result into the code
 * cache.
 *
 * Copyright (c) 2010 ETH Zurich
 *
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
#ifndef FBT_TRANSLATE_H
#define FBT_TRANSLATE_H

/* forward declare struct */
struct trampoline;
struct translate;
struct thread_local_data;

/** Maximum size for a translated code block  */
#define MAX_BLOCK_SIZE 512

/** the translation can be in these states. */
enum translation_state {
  /** translation must not stop after this instruction but must continue */
  OPEN,
  /** translation may stop after this instruction */
  NEUTRAL,
  /** translation must stop after this instruction */
  CLOSE,
  /** translation must stop after this instruction and fixup code must be
     inserted as if the instruction limit is reached */
  CLOSE_GLUE
};
  
/** Function definition for action functions. */
typedef enum translation_state (*actionFunP_t)(struct translate *ts);

/**
 * Translate a given code region located at orig_address and put the
 * translated code fragment into the code cache.
 * Translates a translation unit without jumping to the translated code.
 * If the translation unit (TU) has already been translated and is in the
 * code cache, the function returns immediately. Otherwise, the TU is
 * translated first and stored in the code cache.
 * @param tld pointer to thread local data.
 * @param orig_address the address where the TU begins
 * @return pointer to the translated code that corresponds to this TU
 */
void *fbt_translate_noexecute(struct thread_local_data *tld,
                              void *orig_address);

/**
 * Disassembles one instruction and fills in all information into the
 * struct translate.
 * This function disassembles the current instruction at ts->next_instr and
 * retrieves the length of the instruction in bytes (including prefixes, opcode,
 * ptr, modR/M, SIB, and immediates) and the pointer to the function that shall
 * be used to handle this instruction.
 * @param ts pointer to translate struct. ts is changed to correspond to the
 * current instruction.
 */
void fbt_disasm_instr(struct translate *ts);

#endif /* FBT_TRANSLATE_H */
