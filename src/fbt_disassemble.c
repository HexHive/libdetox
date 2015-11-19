/**
 * Defines the types and functions needed to disassemble one asm instruction
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
#include "fbt_translate.h"
#include "fbt_datatypes.h"
#include "fbt_actions.h"
#include "fbt_disassemble.h"
#include "fbt_opcode_tables.h"

#include "fbt_debug.h"

/* bit test masks for ModR/M byte */
#define MASK_MOD 0xC0
#define MASK_REG 0x38
#define SHIFT_REG 3
#define MASK_RM  0x07


/**
 * disassemles an instruction and retrieves information on how to handle it.
 * This function disassembles the instruction at insn_address, and retrieves the length of
 * the instruction in bytes (including prefixes, opcode, ptr, modR/M, SIB, and immediates) and
 * the pointer to the function that shall be used to handle this instruction.
 * @param insn_addr the address of the instruction that shall be disassembled
 * @param action pointer to the function pointer used to handle the disassembled instruction
 * @param opcode_table the table where the opcode lookup shall be performed
 * @return the length of the instruction in bytes
 */
void disasm_instr(translate_struct_t *ts)
{
	ia32_opcode_t *opcode, *opcode_table = opcode_table_onebyte;
	int bytes_argument;

	unsigned char *cur = (ts->cur_instr = ts->next_instr);
	ts->num_prefixes = 0;

	/* 0x1 - operand overwrite */
	unsigned char prefix = 0;

	while (1) {
		opcode = opcode_table + (unsigned int)*cur;

		/* check for OPCODE_ESC */
		if ((opcode->opcode_flags & OPCODE_ESC) != 0) {
			/* this is an opcode escape -> we have to perform a lookup in an additional table */
			PRINT_DEBUG("opcode escape found: %x", *cur);
			opcode_table = opcode->opcode_table;
			cur++;
			continue;  /* do one more iteration */

		} else if ((opcode->opcode_flags & PREFIX_ESC) != 0) {
			/* this is a prefix -> we have to perform a lookup in an additional table */
			PRINT_DEBUG("prefix found: %x", *cur);
			ts->num_prefixes++;
			/* for performance reasons we _now_ only support 1 prefix. more can be added. */
			assert(ts->num_prefixes<=1 || ((prefix!=PREFIX_ADDR_SZ_OVR) && (prefix!=PREFIX_OP_SZ_OVR)));
			prefix = *cur;
			opcode_table = opcode->opcode_table;
			cur++;
			continue;  /* do one more iteration */

		} else {
			/* this was the last byte of the opcode */
			ts->first_byte_after_opcode = cur + 1;

			/* check if this instruction has any kind of ModRM byte */
			if ((opcode->opcode_flags & ARG_RM) || (opcode->opcode_flags & OPCODE_RM) || (opcode->opcode_flags & FPU_ESC)) {
				cur++;
				bytes_argument = 0;

				/* check for OPCODE_RM */
				if ((opcode->opcode_flags & OPCODE_RM)) {
					/* this opcode extends into the reg bits of the ModR/M byte */
					/* get the opcode from the group table */
					opcode = opcode->opcode_table + ((*cur & MASK_REG) >> SHIFT_REG);

				/* check for FPU_ESC */
				} else if ((opcode->opcode_flags & FPU_ESC )) {
					/* this opcode extends into a fpu table */
					/* get the opcode from the fpu table */
					opcode = opcode->opcode_table + (unsigned int)*cur;
				}

				if((opcode->opcode_flags & ARG_RM) && (*cur & MASK_MOD) != 0xC0) {
					/* this instruction has a modRM byte and it refers to a memory location */

					if (prefix!=PREFIX_ADDR_SZ_OVR) {
					  if ((*cur & MASK_RM) == 0x04) {
					    /* there follows a SIB byte (SIB bytes only follow in 32bit mode! */
					    if ((*cur & MASK_MOD) == 0x00 && (*(cur+1) & 0x07) == 0x05) {
					      /* after the SIB there follows a 32bit displacement */
					      bytes_argument += 5;
					    } else {
					      bytes_argument++;
					    }
					  }
					  if ((*cur & MASK_MOD) == 0x00 && (*cur & MASK_RM) == 0x05) {
					    /* mod bits are 00, R/M bits are 101 -> another case of 32bit displacement */
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
					    /* mod bits are 00, R/M bits are 110 -> another case of 16bit displacement */
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
			ts->action = opcode->opcode_handler;

			/* read info regarding immediate arguments (immediates, code offset, ...) */
			/* this checks for the operand size and if we have a different os, then we
			   check for 4byte parameters and change them into 2byte..
			   not correct in all cases (imm>4byte)! (check assertion below) */
			if (prefix!=PREFIX_OP_SZ_OVR) {
			  cur += MASK_ARG & opcode->opcode_flags;
			} else {
			  PRINT_DEBUG("Prefix found: OP_SZ_OVR!");
			  assert((MASK_ARG & opcode->opcode_flags)<=ARG_D);
			  if ((MASK_ARG & opcode->opcode_flags)==ARG_D) {
			    cur += 2;
			  } else {
			    cur += MASK_ARG & opcode->opcode_flags;
			  }
			}
		}

		/* if we reach this point we are done */
		break;
	}

	/* now we know how many bytes the current instructions takes */
	ts->next_instr = cur+1;
}

