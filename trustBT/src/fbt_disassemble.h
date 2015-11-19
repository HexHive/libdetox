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
#ifndef FBT_DISASSEMBLE_H
#define FBT_DISASSEMBLE_H

/**
 * mask for testing operand bits.
 * As it is defined now, the length of the arguments in bytes is just opcode_flags & MASK_ARG.
 * This only works as long as we don't have, for example, a 4 byte and a 6 byte argument at
 * the same time, or several arguments of the same length. If this ever changes, we will have
 * to adjust the functions in fbt_disassemble.c
 */
#define MASK_ARG 0x0F



/* disassembles one instruction */
void disasm_instr(translate_struct_t *ts);


#endif /* FBT_DISASSEMBLE_H */
