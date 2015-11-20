/**
 * This header file contains all macros needed in fbt_actions.c to
 * generate the machine code used in the ccfs
 *
 * If FBT is compiled with statistics, then they will be collected here.
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Antonio Barresi <abarresi@student.ethz.ch>
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
#ifndef FBT_ASM_MACROS_H
#define FBT_ASM_MACROS_H


#define PUSHL_IMM32(dst, imm) *dst++=0x68; *((int32_t*)dst) = imm; dst+=4

#define PUSHL_EAX(dst) *dst++=0x50
#define PUSHL_EBX(dst) *dst++=0x53
#define PUSHL_ECX(dst) *dst++=0x51
#define PUSHL_EDX(dst) *dst++=0x52
#define POPL_EAX(dst) *dst++=0x58
#define POPL_EBX(dst) *dst++=0x5b
#define POPL_ECX(dst) *dst++=0x59
#define POPL_EDX(dst) *dst++=0x5a

#define PUSHFL(dst) *dst++=0x9c
#define POPFL(dst) *dst++=0x9d

#define LEAVE(dst) *dst++ = 0xc9

/* mem32 = mem32 - imm */
#define SUBL_IMM8_MEM32(dst, imm, mem) *dst++=0x83; *dst++=0x2d; *((int32_t*)dst) = mem; dst+=4; *dst++=imm

#define SUBL_R32_IMM32RM32SIB(dst, modrm, sib, imm32) *dst++=0x2b; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm32; dst+=4

#define ADDL_IMM8_MEM32(dst, imm, mem) *dst++=0x83; *dst++=0x05; *((int32_t*)dst) = mem; dst+=4; *dst++=imm
#define ADDL_IMM32_RM32(dst, modrm, imm32) *dst++=0x81; *dst++=modrm; *((int32_t*)dst) = imm32; dst+=4
#define ADDL_IMM8_RM32(dst, modrm, imm8) *dst++=0x83; *dst++=modrm; *dst++=imm8

#define ANDL_IMM32_RM32(dst, modrm, imm32) ADDL_IMM32_RM32(dst, modrm, imm32)

#define LEAL_IMM8RM32_R(dst, modrm, sib, imm8) *dst++=0x8d; *dst++=modrm; *dst++=sib; *dst++=imm8
#define LEAL_IMM32RM32_R(dst, modrm, sib, imm32) *dst++=0x8d; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm32; dst+=4

/* movl (mem32), %eax */
#define MOVL_MEM32_EAX(dst, m32) *dst++=0xa1; *((int32_t*)dst) = m32; dst+=4
/* movl r32, rm32 */
#define MOVL_R32_RM32(dst, modrm) *dst++=0x89; *dst++=modrm
/* movl r32, rm32 */
#define MOVL_RM32SIB_R32(dst, modrm, sib) *dst++=0x8b; *dst++=modrm; *dst++=sib
/* movl r32, i8rm32 */
#define MOVL_R32_IMM8RM32SIB(dst, modrm, sib, imm8) *dst++=0x89; *dst++=modrm; *dst++=sib; *dst++=imm8
#define MOVL_R32_SIBIMM8(dst, reg1, sib, imm8) MOVL_R32_IMM8RM32SIB(dst, MODRM(MODE_MEM_REG_IMM8, OP1_SIB, reg1), sib, imm8)
/* movl r32, i32rm32 */
#define MOVL_R32_IMM32RM32(dst, modrm, imm32) *dst++=0x89; *dst++=modrm; *((int32_t*)dst) = imm32; dst+=4
/* movl r32, i32rm32 */
#define MOVL_R32_IMM32RM32SIB(dst, modrm, sib, imm32) *dst++=0x89; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm32; dst+=4
#define MOVL_R32_SIBIMM32(dst, reg1, sib, imm32) MOVL_R32_IMM32RM32SIB(dst, MODRM(MODE_MEM_REG, OP1_SIB, reg1), sib, imm32)
/* cmpl rm32, imm32 */
#define CMPL_IMM32_RM32(dst, modrm, i32) *dst++=0x81; *dst++=modrm; *((int32_t*)dst) = i32; dst+=4

/* movl rm32, r32 */
#define MOVL_RM32_R32(dst, modrm) *dst++=0x8b; *dst++=modrm
/* movl imm8(mem32), rm32 */
#define MOVL_IMM8RM32_R32(dst, modrm, imm8) *dst++=0x8b; *dst++=modrm; *dst++=imm8
/* movl imm8(mem32), rm32 */
#define MOVL_IMM8RM32SIB_R32(dst, modrm, sib, imm8) *dst++=0x8b; *dst++=modrm; *dst++=sib; *dst++=imm8
#define MOVL_SIBIMM8_R32(dst, sib, imm8, reg2) MOVL_IMM8RM32SIB_R32(dst, MODRM(MODE_MEM_REG_IMM8, OP1_SIB, reg2), sib, imm8)

/* movl imm32(mem32), rm32 */
#define MOVL_IMM32RM32SIB_R32(dst, modrm, sib, imm32) *dst++=0x8b; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm32; dst+=4

/* movl imm32, (modrm) */
#define MOVL_IMM32_RM32(dst, imm, modrm) *dst++=0xc7; *dst++=modrm; *((int32_t*)dst) = imm; dst+=4

/* movl imm, (imm2) */
#define MOVL_IMM32_IMM32RM32(dst, modrm, imm, imm2) *dst++=0xc7; *dst++=modrm; *((int32_t*)dst) = imm2; dst+=4; *((int32_t*)dst) = imm; dst+=4;

/* movl imm32, imm8(modrm) */
#define MOVL_IMM32_RM32I8(dst, imm, modrm, i8) *dst++=0xc7; *dst++=modrm; *dst++=i8; *((int32_t*)dst) = imm; dst+=4
#define MOVL_IMM32_I8RM32SIB(dst,imm,modrm,sib,i8) *dst++=0xc7; *dst++=modrm; *dst++=sib; *dst++=i8; *((int32_t*)dst) = imm; dst+=4

#define MOVL_IMM32_RM32SIB(dst, modrm, sib, imm) *dst++=0xc7; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm; dst+=4

#define MOVZBL_IMM8RM8SIB_R32(dst, modrm, sib, imm8) *dst++=0x0f; *dst++=0xb6; *dst++=modrm; *dst++=sib; *dst++=imm8

#define MOVZBL_RM8SIB_R32(dst, modrm, sib) *dst++=0x0f; *dst++=0xb6; *dst++=modrm; *dst++=sib

/* cmpl r32, imm32(modrm) */
#define CMPL_R32_RM32I32(dst, modrm, i32) *dst++=0x39; *dst++=modrm; *((int32_t*)dst) = i32; dst+=4
/* cmpl i8rm32, r32 */
#define CMPL_I8RM32SIB_R32(dst, modrm, sib, i8) *dst++=0x3b; *dst++=modrm; *dst++=sib; *dst++=i8
/* cmpl imm32, rm32 */
#define CMPL_IMM32_RM32SIB(dst, modrm, sib, i32) *dst++=0x81; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = i32; dst+=4
/* cmpl imm32, rm32i8 */
#define CMPL_IMM32_RM32I8SIB(dst, modrm, sib, i8, i32) *dst++=0x81; *dst++=modrm; *dst++=sib; *dst++=i8; *((int32_t*)dst) = i32; dst+=4
#define CMPL_R32_IMM32RM32SIB(dst, modrm, sib, imm32) *dst++=0x3b; *dst++=modrm; *dst++=sib; *((int32_t*)dst) = imm32; dst+=4

#define JE_I8(dst, i8) *dst++=0x74; *dst++=i8
#define JE_I32(dst, rel32) *dst++=0x0F; *dst++=0x84; *((int32_t*)dst) = rel32 - (int32_t)dst - 4; dst+=4
#define JNE_I8(dst, i8) *dst++=0x75; *dst++=i8
#define JNE_I32(dst, rel32) *dst++=0x0f; *dst++=0x85; *((int32_t*)dst) = rel32 - (int32_t)dst - 4; dst+=4
#define JECXZ_I8(dst, i8) *dst++=0xe3; *dst++=i8
#define JCC_2B(dst, jcc, rel32) *((int16_t*)dst)=(int16_t)jcc; dst+=2; *((int32_t*)dst) = rel32 - (int32_t)dst - 4; dst+=4

#define CALL_REL32(dst, rel32) *dst++=0xe8; *((int32_t*)dst) = rel32 - (int32_t)dst - 4; dst+=4
#define JMP_REL32(dst, rel32) *dst++=0xe9; *((int32_t*)dst) = rel32 - (int32_t)dst - 4; dst+=4
#define JMP_IND_M32(dst, m32) *dst++ = 0xff; *dst++ = 0x25; *((int32_t*)dst) = m32; dst+=4
#define JMP_IND_IMM32RM(dst, modrm, imm32) *dst++ = 0xff; *dst++ = 0x24; *dst++=modrm; *((int32_t*)dst) = imm32; dst+=4

#define RET(dst) *dst++=0xc3
#define RET_IMM16(dst, imm16) *dst++=0xc2; *((int16_t*)dst) = imm16; dst+=2

#define INCL_M32(dst, m32) *dst++=0xff; *dst++=0x05; *((int32_t*)dst) = m32; dst+=4
#define INCL_M64(dst, m32) *dst++=0x83; *dst++=0x05; *((int32_t*)dst) = m32; dst+=4; *dst++=0x01; *dst++=0x83; *dst++=0x15; *((int32_t*)dst) = m32+4; dst+=4; *dst++=0x00

#define SAL_RM32IMM8(dst, modrm, i8) *dst++=0xc1; *dst++=modrm; *dst++=i8

#define SYSENTER(dst) *dst++=0x0f; *dst++=0x34
#define INT80(dst) *dst++=0xcd; *dst++=0x80

#define NOP(dst) *dst++=0x90

#define TEST_REG32_REG32(dst, reg1, reg2) *dst++=0x85; *dst++=MODRM(MODE_REG, reg1, reg2)

#define MODRM(mode, reg1, reg2) (mode | reg1 | (reg2<<3))
#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define OPCODE4 4
#define EBP 5
#define ESI 6
#define EDI 7
#define OP1_SIB 4
#define OP1_IMM32 5
#define MODE_MEM_REG       0x00 // [reg1]
#define MODE_MEM_REG_IMM8  0x40 // [reg1+imm8]    
#define MODE_MEM_REG_IMM32 0x80 // [reg1+imm32]
#define MODE_REG           0xC0 // reg1

// [reg1 + reg2*scale] where: scale is 1, 2, 4 or 8; reg2 is not ESP; reg1 is not EBP (special cases are possible)
#define SIB(reg1, reg2, scale) (reg1 | (reg2<<3) | (scale<<6))
#define SIB_REG2_NONE 4
#define SIB_REG1_NONE 5
#define SIB_ESP SIB(ESP, SIB_REG2_NONE, 0) // [esp]
#define SIB_EBP SIB(EBP, SIB_REG2_NONE, 0) // [ebp]

#endif
