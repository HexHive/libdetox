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
	    (OPCODETABLE(opcode->tableFlags) && ((opcode->opcode.table + *(cur+2))->opcode.handler)!=0))
	    {
	    /* this is an opcode escape -> we have to perform a lookup in an additional table */
	    PRINT_DEBUG("opcode escape found to 3byte opcode: %x", *cur);
	    /* there are some 3byte opcodes which start with a prefix and then escape through 0x0f
	       we take care of them _here_
	       otherwise, we just go back to the first table and continue our translation
	    */
	    opcode_table = opcode->opcode.table;
	    cur+=2; /* skip the 0x0f escape byte */
	    continue;  /* do one more iteration */
		    
	    /* check for OPCODE_ESC */
	} else if ((opcode->tableFlags & OPCODE_ESC) != 0) {
	    /* this is an opcode escape -> we have to perform a lookup in an additional table */
	    PRINT_DEBUG("opcode escape found: %x", *cur);
	    opcode_table = opcode->opcode.table;
	    cur++;
	    continue;  /* do one more iteration */

	} else if ((opcode->tableFlags & OPCODE_PREFIX_ESC) != 0) {
	    /* this is a prefix -> we have to perform a lookup in an additional table */
	    PRINT_DEBUG("prefix found: %x", *cur);
	    ts->num_prefixes++;
	    /* for performance reasons we _now_ only support 1 out of ADDR or OP SZ OVR prefix. more can be added. */
	    if (!(ts->num_prefixes<=1 || ((prefix!=PREFIX_ADDR_SZ_OVR) && (prefix!=PREFIX_OP_SZ_OVR)))) {
		/* we only support one prefix atm */
		_exit(139);
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
	    if (anyModRMOp(opcode) || (opcode->tableFlags&OPCODE_MODRM_ESC) || (opcode->tableFlags&OPCODE_FPU_ESC)) {
		cur++;
		bytes_argument = 0;

		/* check for OPCODE_RM */
		if (opcode->tableFlags&OPCODE_MODRM_ESC) {
		    /* this opcode extends into the reg bits of the ModR/M byte */
		    /* get the opcode from the group table */
		    opcode = opcode->opcode.table + ((*cur & MASK_REG) >> SHIFT_REG);

		    /* check for FPU_ESC */
		} else if (opcode->tableFlags&OPCODE_FPU_ESC) {
		    /* this opcode extends into a fpu table */
		    /* get the opcode from the fpu table */
		    opcode = opcode->opcode.table + (unsigned int)*cur;
		}

		if ((opcode->tableFlags&HAS_MODRM) && (*cur & MASK_MOD) != 0xC0) {
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
	    ts->action = opcode->opcode.handler;
	    ts->cur_instr_info = opcode;
	    assert(!OPCODETABLE(opcode->tableFlags));

#ifdef __LP64__
	    if (rex && (rex&REXW)) {
		/* rex.b size extenstion */
		if ((MASK_ARG & opcode->opcode_flags)==ARG_D) {
		    cur += 8;
		} else {
		    printf("unhandled rexw prefix!\n");
		}
	    } else {
#endif
		/* read info regarding immediate arguments (immediates, code offset, ...) */
		if (hasImmOp(opcode->destFlags))
		    cur += operandSize(opcode->destFlags, prefix);
		if (hasImmOp(opcode->srcFlags))
		    cur += operandSize(opcode->srcFlags, prefix);
		if (hasImmOp(opcode->auxFlags))
		    cur += operandSize(opcode->auxFlags, prefix);
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

unsigned int operandSize(unsigned int operandFlags, unsigned char prefix)
{
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
	if (prefix==PREFIX_OP_SZ_OVR) return 1;
	return 2;
    case OPT_a:
    case OPT_z:
	if (prefix==PREFIX_OP_SZ_OVR) return 2;
	return 4;
    case OPT_v:
	/* TODO: special handling for 64bit x86_64 ! */
	if (prefix==PREFIX_OP_SZ_OVR) return 2;
	return 4;
    case OPT_p:
	/* 32bit (16b addr mode), 48bit (32b addr mode) or 80b (64b addr mode) ptr */
    case OPT_si:
	/* doubleword register - unhandled */
	_exit(139);
	return -1;
    }
    return 0;
}
