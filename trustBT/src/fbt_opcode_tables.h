/***  !! THIS IS A GENERATED FILE !!  ***/
/***  DO NOT MODIFY THIS FILE - ALL CHANGES WILL BE OVERWRITTEN WHEN A NEW VERSION IS GENERATED  ***/


#ifndef FBT_OPCODE_TABLES_H
#define FBT_OPCODE_TABLES_H


/*
 * tables for opcode extensions in ModR/M byte.
 * The opcodes extend into the reg bits (5, 4, 3) of the ModR/M byte.
 */

/** group 1 OPCODE 0x80 */
const struct ia32_opcode opcode_table_80_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 ADD imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 001 OR imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 010 ADC imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 011 SBB imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 AND imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 SUB imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 110 XOR imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 CMP imm8, r/m8 */
};


/** group 1 OPCODE 0x81 */
const struct ia32_opcode opcode_table_81_rm[] = {
	{ ARG_D | ARG_RM, 0, action_copy },	/* 000 ADD imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 001 OR imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 010 ADC imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 011 SBB imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 100 AND imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 101 SUB imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 110 XOR imm32, r/m32 */
	{ ARG_D | ARG_RM, 0, action_copy }	/* 111 CMP imm32, r/m32 */
};


/** group 1 OPCODE 0x82 */
const struct ia32_opcode opcode_table_82_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 ADD imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 001 OR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 010 ADC imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 011 SBB imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 AND imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 SUB imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 110 XOR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 CMP imm8, r/m32 */
};


/** group 1 OPCODE 0x83 */
const struct ia32_opcode opcode_table_83_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 ADD imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 001 OR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 010 ADC imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 011 SBB imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 AND imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 SUB imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 110 XOR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 CMP imm8, r/m32 */
};


/** group 2 OPCODE 0xC0 */
const struct ia32_opcode opcode_table_C0_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 ROL imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 001 ROR imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 010 RCL imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 011 RCR imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 SHL imm8, r/m8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 SHR imm8, r/m8 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 SAR imm8, r/m8 */
};


/** group 2 OPCODE 0xC1 */
const struct ia32_opcode opcode_table_C1_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 ROL imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 001 ROR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 010 RCL imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 011 RCR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 SHL imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 SHR imm8, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 SAR imm8, r/m32 */
};


/** group 2 OPCODE 0xD0 */
const struct ia32_opcode opcode_table_D0_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 ROL $1, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 001 ROR $1, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 010 RCL $1, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 011 RCR $1, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 100 SHL $1, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 101 SHR $1, r/m8 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_RM, 0, action_copy }	/* 111 SAR $1, r/m8 */
};


/** group 2 OPCODE 0xD1 */
const struct ia32_opcode opcode_table_D1_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 ROL $1, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 001 ROR $1, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 010 RCL $1, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 011 RCR $1, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 100 SHL $1, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 101 SHR $1, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_RM, 0, action_copy }	/* 111 SAR $1, r/m32 */
};


/** group 2 OPCODE 0xD2 */
const struct ia32_opcode opcode_table_D2_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 ROL CL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 001 ROR CL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 010 RCL CL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 011 RCR CL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 100 SHL CL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 101 SHR CL, r/m8 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_RM, 0, action_copy }	/* 111 SAR CL, r/m8 */
};


/** group 2 OPCODE 0xD3 */
const struct ia32_opcode opcode_table_D3_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 ROL CL, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 001 ROR CL, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 010 RCL CL, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 011 RCR CL, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 100 SHL CL, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 101 SHR CL, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_RM, 0, action_copy }	/* 111 SAR CL, r/m32 */
};


/** group 3 OPCODE 0xF6 */
const struct ia32_opcode opcode_table_F6_rm[] = {
	{ ARG_B | ARG_RM, 0, action_copy },	/* 000 TEST imm8, r/m8 */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 010 NOT, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 011 NEG, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 100 MUL %ax = %al * r/m8 */
	{ ARG_RM, 0, action_copy },	/* 101 IMUL %ax = %al * r/m8 */
	{ ARG_RM, 0, action_copy },	/* 110 DIV %al = %eax / r/m8   %ah = %eax % r/m8 */
	{ ARG_RM, 0, action_copy }	/* 111 IDIV %al = %eax / r/m8   %ah = %eax % r/m8 */
};


/** group 3 OPCODE 0xF7 */
const struct ia32_opcode opcode_table_F7_rm[] = {
	{ ARG_D | ARG_RM, 0, action_copy },	/* 000 TEST imm32, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 010 NOT, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 011 NEG, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 100 MUL %edx:%eax = %eax * r/m32 */
	{ ARG_RM, 0, action_copy },	/* 101 IMUL %edx:%eax = %eax * r/m32 */
	{ ARG_RM, 0, action_copy },	/* 110 DIV %eax = %edx:%eax / r/m32   %edx = %edx:%eax % r/m32 */
	{ ARG_RM, 0, action_copy }	/* 111 IDIV %eax = %edx:%eax / r/m32   %edx = %edx:%eax % r/m32 */
};


/** group 4 OPCODE 0xFE */
const struct ia32_opcode opcode_table_FE_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 INC, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 001 DEC, r/m8 */
	{ ARG_NONE, 0, action_fail },	/* 010  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 100  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 5 OPCODE 0xFF */
const struct ia32_opcode opcode_table_FF_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 INC, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 001 DEC, r/m32 */
	{ ARG_RM, 0, action_call_indirect },	/* 010 CALL near indirect */
	{ ARG_RM, 0, action_fail },	/* 011 CALL far -> we should not CALL kernel functions directly */
	{ ARG_RM, 0, action_jmp_indirect },	/* 100 JMP, r/m32 */
	{ ARG_RM, 0, action_fail },	/* 101 JMP far -> we should not JMP to kernel functions directly */
	{ ARG_RM, 0, action_copy },	/* 110 PUSH, r/m32 */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 6 OPCODE 0x0F00 */
const struct ia32_opcode opcode_table_0F00_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 SLDT, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 001 STR, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 010 LLDT, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 011 LTR, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 100 VERR, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 101 VERW, r/m16 */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 7 OPCODE 0x0F01 */
const struct ia32_opcode opcode_table_0F01_rm[] = {
	{ ARG_RM, 0, action_copy },	/* 000 SGDT, m80 */
	{ ARG_RM, 0, action_copy },	/* 001 SIDT, m80 */
	{ ARG_RM, 0, action_copy },	/* 010 LGDT, r/m80 */
	{ ARG_RM, 0, action_copy },	/* 011 LIDT, r/m80 */
	{ ARG_RM, 0, action_copy },	/* 100 SMSW, r/m16 */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 110 LMSW, r/m16 */
	{ ARG_RM, 0, action_copy }	/* 111 INVLPG, r/m8 */
};


/** group 8 OPCODE Ox0FBA */
const struct ia32_opcode opcode_table_0FBA_rm[] = {
	{ ARG_NONE, 0, action_fail },	/* 000  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 010  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 100 BT imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 101 BTS imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 110 BTR imm8, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy }	/* 111 BTC imm8, r/m32 */
};


/** group 9 OPCODE Ox0FC7 */
const struct ia32_opcode opcode_table_0FC7_rm[] = {
	{ ARG_NONE, 0, action_fail },	/* 000  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 001 CMPXCH8B, r/m64 */
	{ ARG_NONE, 0, action_fail },	/* 010  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 100  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 12 OPCODE Ox0F71 */
const struct ia32_opcode opcode_table_0F71_rm[] = {
	{ ARG_NONE, 0, action_fail },	/* 000  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 010 PSRLW imm8, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 100 PSRAW imm8, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 110 PSLLW imm8, reg64 */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 13 OPCODE Ox0F72 */
const struct ia32_opcode opcode_table_0F72_rm[] = {
	{ ARG_NONE, 0, action_fail },	/* 000  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 010 PSRLD imm8, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 100 PSRAD imm8, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 110 PSLLD imm8, reg64 */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 14 OPCODE Ox0F73 */
const struct ia32_opcode opcode_table_0F73_rm[] = {
	{ ARG_NONE, 0, action_fail },	/* 000  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 001  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 010 PSRLQ imm8, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 011  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 100  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 110 PSLLQ imm8, reg64 */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};


/** group 15 OPCODE Ox0FAE */
const struct ia32_opcode opcode_table_0FAE_rm[] = {
	{ ARG_NONE, 0, action_warn },	/* 000 FXSAVE */
	{ ARG_NONE, 0, action_warn },	/* 001 FXRSTOR */
	{ ARG_NONE, 0, action_warn },	/* 010 LDMXCSR */
	{ ARG_NONE, 0, action_warn },	/* 011 STMXCSR */
	{ ARG_NONE, 0, action_warn },	/* 100 SFENCE */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_RM, 0, action_warn }	/* 111 CLFLUSH, m8 */
};


/** group 16 OPCODE Ox0F18 */
const struct ia32_opcode opcode_table_0F18_rm[] = {
	{ ARG_RM, 0, action_warn },	/* 000 PREFETCH NTA */
	{ ARG_RM, 0, action_warn },	/* 001 PREFETCH T0 */
	{ ARG_RM, 0, action_warn },	/* 010 PREFETCH T1 */
	{ ARG_RM, 0, action_warn },	/* 011 PREFETCH T2 */
	{ ARG_NONE, 0, action_fail },	/* 100  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 101  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 110  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 111  undefined opcode */
};




/*
 * tables for fpu opcodes.
 */

/** fpu OPCODE 0xD8 */
const struct ia32_opcode opcode_table_D8_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xD800 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD801 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD802 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD803 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD804 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD805 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD806 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD807 FADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD808 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD809 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80A FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80B FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80C FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80D FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80E FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD80F FMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD810 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD811 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD812 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD813 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD814 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD815 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD816 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD817 FCOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD818 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD819 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81A FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81B FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81C FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81D FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81E FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD81F FCOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD820 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD821 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD822 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD823 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD824 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD825 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD826 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD827 FSUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD828 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD829 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82A FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82B FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82C FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82D FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82E FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD82F FSUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xD830 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD831 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD832 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD833 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD834 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD835 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD836 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD837 FDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD838 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD839 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83A FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83B FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83C FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83D FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83E FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD83F FDIVR ST0 = m32 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xD840 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD841 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD842 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD843 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD844 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD845 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD846 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD847 FADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD848 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD849 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84A FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84B FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84C FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84D FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84E FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD84F FMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD850 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD851 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD852 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD853 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD854 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD855 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD856 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD857 FCOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD858 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD859 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85A FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85B FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85C FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85D FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85E FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD85F FCOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD860 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD861 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD862 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD863 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD864 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD865 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD866 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD867 FSUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD868 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD869 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86A FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86B FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86C FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86D FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86E FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD86F FSUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xD870 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD871 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD872 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD873 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD874 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD875 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD876 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD877 FDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD878 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD879 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87A FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87B FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87C FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87D FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87E FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD87F FDIVR ST0 = m32 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xD880 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD881 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD882 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD883 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD884 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD885 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD886 FADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD887 FADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD888 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD889 FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88A FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88B FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88C FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88D FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88E FMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD88F FMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD890 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD891 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD892 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD893 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD894 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD895 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD896 FCOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD897 FCOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD898 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD899 FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89A FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89B FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89C FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89D FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89E FCOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD89F FCOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD8A0 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A1 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A2 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A3 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A4 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A5 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A6 FSUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8A7 FSUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD8A8 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8A9 FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AA FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AB FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AC FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AD FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AE FSUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8AF FSUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xD8B0 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B1 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B2 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B3 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B4 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B5 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B6 FDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD8B7 FDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xD8B8 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8B9 FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BA FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BB FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BC FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BD FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BE FDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xD8BF FDIVR ST0 = m32 / ST0 */

	{ ARG_NONE, 0, action_copy },	/* 0xD8C0 FADD ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C1 FADD ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C2 FADD ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C3 FADD ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C4 FADD ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C5 FADD ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C6 FADD ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C7 FADD ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8C8 FMUL ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8C9 FMUL ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CA FMUL ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CB FMUL ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CC FMUL ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CD FMUL ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CE FMUL ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8CF FMUL ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8D0 FCOM ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D1 FCOM ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D2 FCOM ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D3 FCOM ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D4 FCOM ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D5 FCOM ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D6 FCOM ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D7 FCOM ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8D8 FCOMP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8D9 FCOMP ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DA FCOMP ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DB FCOMP ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DC FCOMP ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DD FCOMP ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DE FCOMP ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8DF FCOMP ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8E0 FSUB ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E1 FSUB ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E2 FSUB ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E3 FSUB ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E4 FSUB ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E5 FSUB ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E6 FSUB ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E7 FSUB ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8E8 FSUBR ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8E9 FSUBR ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8EA FSUBR ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8EB FSUBR ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8EC FSUBR ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8ED FSUBR ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8EE FSUBR ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8EF FSUBR ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8F0 FDIV ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F1 FDIV ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F2 FDIV ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F3 FDIV ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F4 FDIV ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F5 FDIV ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F6 FDIV ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F7 FDIV ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD8F8 FDIVR ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8F9 FDIVR ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8FA FDIVR ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8FB FDIVR ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8FC FDIVR ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8FD FDIVR ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD8FE FDIVR ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy }	/* 0xD8FF FDIVR ST(7), ST(0) */
};


/** fpu OPCODE 0xD9 */
const struct ia32_opcode opcode_table_D9_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xD900 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD901 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD902 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD903 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD904 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD905 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD906 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD907 FLD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xD908  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD909  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD90F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xD910 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD911 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD912 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD913 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD914 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD915 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD916 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD917 FST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD918 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD919 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91A FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91B FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91C FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91D FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91E FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD91F FSTP ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD920 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD921 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD922 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD923 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD924 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD925 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD926 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD927 FLDENV, r/m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD928 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD929 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92A FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92B FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92C FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92D FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92E FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD92F FLDCW, r/m16 */

	{ ARG_RM, 0, action_copy },	/* 0xD930 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD931 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD932 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD933 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD934 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD935 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD936 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD937 FSTENV, m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD938 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD939 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93A FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93B FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93C FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93D FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93E FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD93F FSTCW, m16 */

	{ ARG_RM, 0, action_copy },	/* 0xD940 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD941 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD942 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD943 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD944 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD945 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD946 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD947 FLD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xD948  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD949  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD94F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xD950 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD951 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD952 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD953 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD954 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD955 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD956 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD957 FST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD958 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD959 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95A FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95B FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95C FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95D FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95E FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD95F FSTP ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD960 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD961 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD962 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD963 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD964 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD965 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD966 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD967 FLDENV, r/m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD968 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD969 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96A FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96B FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96C FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96D FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96E FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD96F FLDCW, r/m16 */

	{ ARG_RM, 0, action_copy },	/* 0xD970 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD971 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD972 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD973 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD974 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD975 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD976 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD977 FSTENV, m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD978 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD979 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97A FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97B FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97C FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97D FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97E FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD97F FSTCW, m16 */

	{ ARG_RM, 0, action_copy },	/* 0xD980 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD981 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD982 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD983 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD984 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD985 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD986 FLD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xD987 FLD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xD988  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD989  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD98F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xD990 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD991 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD992 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD993 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD994 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD995 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD996 FST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD997 FST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD998 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD999 FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99A FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99B FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99C FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99D FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99E FSTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xD99F FSTP ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xD9A0 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A1 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A2 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A3 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A4 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A5 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A6 FLDENV, r/m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A7 FLDENV, r/m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD9A8 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9A9 FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AA FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AB FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AC FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AD FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AE FLDCW, r/m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9AF FLDCW, r/m16 */

	{ ARG_RM, 0, action_copy },	/* 0xD9B0 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B1 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B2 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B3 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B4 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B5 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B6 FSTENV, m224 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B7 FSTENV, m224 */

	{ ARG_RM, 0, action_copy },	/* 0xD9B8 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9B9 FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BA FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BB FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BC FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BD FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BE FSTCW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xD9BF FSTCW, m16 */

	{ ARG_NONE, 0, action_copy },	/* 0xD9C0 FLD ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C1 FLD ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C2 FLD ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C3 FLD ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C4 FLD ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C5 FLD ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C6 FLD ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C7 FLD ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD9C8 FXCH ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9C9 FXCH ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CA FXCH ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CB FXCH ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CC FXCH ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CD FXCH ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CE FXCH ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xD9CF FXCH ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xD9D0 FNOP */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xD9D8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9D9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9DF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xD9E0 FCHS */
	{ ARG_NONE, 0, action_copy },	/* 0xD9E1 FABS */
	{ ARG_NONE, 0, action_fail },	/* 0xD9E2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9E3  undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0xD9E4 FTST */
	{ ARG_NONE, 0, action_copy },	/* 0xD9E5 FXAM */
	{ ARG_NONE, 0, action_fail },	/* 0xD9E6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xD9E7  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xD9E8 FLD1 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9E9 FLD2T */
	{ ARG_NONE, 0, action_copy },	/* 0xD9EA FLD2T */
	{ ARG_NONE, 0, action_copy },	/* 0xD9EB FLDPI */
	{ ARG_NONE, 0, action_copy },	/* 0xD9EC FLDLG2 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9ED FLDLN2 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9EE FLDZ */
	{ ARG_NONE, 0, action_fail },	/* 0xD9EF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xD9F0 F2XM1 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F1 FYL2X */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F2 FPTAN */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F3 FPATAN */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F4 FXTRACT */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F5 FPREM1 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F6 FDECSTP */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F7 FINCSTP */

	{ ARG_NONE, 0, action_copy },	/* 0xD9F8 FPREM */
	{ ARG_NONE, 0, action_copy },	/* 0xD9F9 FYL2XP1 */
	{ ARG_NONE, 0, action_copy },	/* 0xD9FA FSQRT */
	{ ARG_NONE, 0, action_copy },	/* 0xD9FB FSINCOS */
	{ ARG_NONE, 0, action_copy },	/* 0xD9FC FRNDINT */
	{ ARG_NONE, 0, action_copy },	/* 0xD9FD FSCALE */
	{ ARG_NONE, 0, action_copy },	/* 0xD9FE FSIN */
	{ ARG_NONE, 0, action_copy }	/* 0xD9FF FCOS */
};


/** fpu OPCODE 0xDA */
const struct ia32_opcode opcode_table_DA_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDA00 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA01 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA02 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA03 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA04 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA05 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA06 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA07 FIADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA08 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA09 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0A FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0B FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0C FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0D FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0E FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA0F FIMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA10 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA11 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA12 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA13 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA14 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA15 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA16 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA17 FICOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA18 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA19 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1A FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1B FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1C FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1D FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1E FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA1F FICOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA20 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA21 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA22 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA23 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA24 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA25 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA26 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA27 FISUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA28 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA29 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2A FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2B FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2C FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2D FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2E FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA2F FISUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDA30 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA31 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA32 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA33 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA34 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA35 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA36 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA37 FIDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA38 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA39 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3A FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3B FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3C FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3D FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3E FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA3F FIDIVR ST0 = m32 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDA40 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA41 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA42 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA43 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA44 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA45 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA46 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA47 FIADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA48 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA49 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4A FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4B FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4C FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4D FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4E FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA4F FIMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA50 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA51 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA52 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA53 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA54 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA55 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA56 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA57 FICOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA58 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA59 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5A FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5B FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5C FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5D FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5E FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA5F FICOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA60 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA61 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA62 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA63 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA64 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA65 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA66 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA67 FISUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA68 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA69 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6A FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6B FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6C FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6D FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6E FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA6F FISUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDA70 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA71 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA72 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA73 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA74 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA75 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA76 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA77 FIDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA78 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA79 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7A FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7B FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7C FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7D FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7E FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDA7F FIDIVR ST0 = m32 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDA80 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA81 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA82 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA83 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA84 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA85 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA86 FIADD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA87 FIADD r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA88 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA89 FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8A FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8B FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8C FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8D FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8E FIMUL r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA8F FIMUL r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA90 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA91 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA92 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA93 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA94 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA95 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA96 FICOM r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA97 FICOM r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDA98 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA99 FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9A FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9B FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9C FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9D FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9E FICOMP r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDA9F FICOMP r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDAA0 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA1 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA2 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA3 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA4 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA5 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA6 FISUB r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAA7 FISUB r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDAA8 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAA9 FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAA FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAB FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAC FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAD FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAE FISUBR ST0 = m32 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAAF FISUBR ST0 = m32 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDAB0 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB1 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB2 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB3 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB4 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB5 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB6 FIDIV r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDAB7 FIDIV r/m32, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDAB8 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDAB9 FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABA FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABB FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABC FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABD FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABE FIDIVR ST0 = m32 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDABF FIDIVR ST0 = m32 / ST0 */

	{ ARG_NONE, 0, action_copy },	/* 0xDAC0 FCMOVB ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC1 FCMOVB ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC2 FCMOVB ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC3 FCMOVB ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC4 FCMOVB ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC5 FCMOVB ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC6 FCMOVB ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC7 FCMOVB ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDAC8 FCMOVE ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAC9 FCMOVE ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACA FCMOVE ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACB FCMOVE ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACC FCMOVE ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACD FCMOVE ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACE FCMOVE ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDACF FCMOVE ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDAD0 FCMOVBE ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD1 FCMOVBE ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD2 FCMOVBE ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD3 FCMOVBE ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD4 FCMOVBE ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD5 FCMOVBE ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD6 FCMOVBE ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD7 FCMOVBE ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDAD8 FCMOVU ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDAD9 FCMOVU ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADA FCMOVU ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADB FCMOVU ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADC FCMOVU ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADD FCMOVU ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADE FCMOVU ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDADF FCMOVU ST(7), ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDAE0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAE7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDAE8  undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0xDAE9 FUCOMPP */
	{ ARG_NONE, 0, action_fail },	/* 0xDAEA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAEB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAEC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAED  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAEE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAEF  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDAF0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDAF8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAF9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAFA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAFB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAFC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAFD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDAFE  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 0xDAFF  undefined opcode */
};


/** fpu OPCODE 0xDB */
const struct ia32_opcode opcode_table_DB_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDB00 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB01 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB02 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB03 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB04 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB05 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB06 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB07 FILD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDB08  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB09  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB0F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB10 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB11 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB12 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB13 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB14 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB15 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB16 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB17 FIST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xDB18 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB19 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1A FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1B FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1C FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1D FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1E FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB1F FISTP ST(0), m32 */

	{ ARG_NONE, 0, action_fail },	/* 0xDB20  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB21  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB22  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB23  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB24  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB25  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB26  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB27  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB28 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB29 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2A FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2B FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2C FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2D FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2E FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB2F FLD m80, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDB30  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB31  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB32  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB33  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB34  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB35  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB36  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB37  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB38 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB39 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3A FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3B FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3C FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3D FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3E FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB3F FSTP ST(0), m80 */

	{ ARG_RM, 0, action_copy },	/* 0xDB40 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB41 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB42 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB43 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB44 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB45 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB46 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB47 FILD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDB48  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB49  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB4F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB50 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB51 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB52 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB53 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB54 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB55 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB56 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB57 FIST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xDB58 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB59 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5A FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5B FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5C FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5D FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5E FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB5F FISTP ST(0), m32 */

	{ ARG_NONE, 0, action_fail },	/* 0xDB60  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB61  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB62  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB63  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB64  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB65  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB66  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB67  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB68 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB69 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6A FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6B FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6C FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6D FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6E FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB6F FLD m80, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDB70  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB71  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB72  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB73  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB74  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB75  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB76  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB77  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB78 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB79 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7A FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7B FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7C FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7D FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7E FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDB7F FSTP ST(0), m80 */

	{ ARG_RM, 0, action_copy },	/* 0xDB80 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB81 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB82 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB83 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB84 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB85 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB86 FILD r/m32, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDB87 FILD r/m32, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDB88  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB89  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDB8F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDB90 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB91 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB92 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB93 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB94 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB95 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB96 FIST ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB97 FIST ST(0), m32 */

	{ ARG_RM, 0, action_copy },	/* 0xDB98 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB99 FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9A FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9B FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9C FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9D FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9E FISTP ST(0), m32 */
	{ ARG_RM, 0, action_copy },	/* 0xDB9F FISTP ST(0), m32 */

	{ ARG_NONE, 0, action_fail },	/* 0xDBA0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBA7  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDBA8 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBA9 FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAA FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAB FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAC FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAD FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAE FLD m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDBAF FLD m80, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDBB0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBB7  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDBB8 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBB9 FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBA FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBB FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBC FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBD FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBE FSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDBBF FSTP ST(0), m80 */

	{ ARG_NONE, 0, action_copy },	/* 0xDBC0 FCMOVNB ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC1 FCMOVNB ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC2 FCMOVNB ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC3 FCMOVNB ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC4 FCMOVNB ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC5 FCMOVNB ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC6 FCMOVNB ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC7 FCMOVNB ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDBC8 FCMOVNE ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBC9 FCMOVNE ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCA FCMOVNE ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCB FCMOVNE ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCC FCMOVNE ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCD FCMOVNE ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCE FCMOVNE ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBCF FCMOVNE ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDBD0 FCMOVNBE ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD1 FCMOVNBE ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD2 FCMOVNBE ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD3 FCMOVNBE ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD4 FCMOVNBE ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD5 FCMOVNBE ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD6 FCMOVNBE ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD7 FCMOVNBE ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDBD8 FCMOVNU ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBD9 FCMOVNU ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDA FCMOVNU ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDB FCMOVNU ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDC FCMOVNU ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDD FCMOVNU ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDE FCMOVNU ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBDF FCMOVNU ST(7), ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDBE0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBE1  undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0xDBE2 FCLEX */
	{ ARG_NONE, 0, action_copy },	/* 0xDBE3 FINIT */
	{ ARG_NONE, 0, action_fail },	/* 0xDBE4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBE5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBE6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBE7  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDBE8 FUCOMI ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBE9 FUCOMI ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBEA FUCOMI ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBEB FUCOMI ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBEC FUCOMI ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBED FUCOMI ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBEE FUCOMI ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBEF FUCOMI ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDBF0 FCOMI ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF1 FCOMI ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF2 FCOMI ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF3 FCOMI ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF4 FCOMI ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF5 FCOMI ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF6 FCOMI ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDBF7 FCOMI ST(7), ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDBF8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBF9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBFA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBFB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBFC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBFD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDBFE  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 0xDBFF  undefined opcode */
};


/** fpu OPCODE 0xDC */
const struct ia32_opcode opcode_table_DC_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDC00 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC01 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC02 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC03 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC04 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC05 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC06 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC07 FADD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC08 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC09 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0A FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0B FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0C FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0D FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0E FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC0F FMUL r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC10 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC11 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC12 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC13 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC14 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC15 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC16 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC17 FCOM r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC18 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC19 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1A FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1B FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1C FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1D FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1E FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC1F FCOMP r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC20 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC21 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC22 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC23 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC24 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC25 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC26 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC27 FSUB r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC28 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC29 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2A FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2B FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2C FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2D FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2E FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC2F FSUBR ST0 = m64 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDC30 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC31 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC32 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC33 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC34 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC35 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC36 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC37 FDIV r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC38 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC39 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3A FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3B FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3C FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3D FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3E FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC3F FDIVR ST0 = m64 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDC40 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC41 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC42 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC43 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC44 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC45 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC46 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC47 FADD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC48 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC49 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4A FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4B FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4C FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4D FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4E FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC4F FMUL r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC50 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC51 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC52 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC53 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC54 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC55 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC56 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC57 FCOM r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC58 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC59 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5A FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5B FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5C FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5D FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5E FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC5F FCOMP r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC60 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC61 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC62 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC63 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC64 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC65 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC66 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC67 FSUB r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC68 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC69 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6A FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6B FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6C FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6D FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6E FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC6F FSUBR ST0 = m64 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDC70 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC71 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC72 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC73 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC74 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC75 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC76 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC77 FDIV r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC78 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC79 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7A FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7B FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7C FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7D FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7E FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDC7F FDIVR ST0 = m64 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDC80 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC81 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC82 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC83 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC84 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC85 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC86 FADD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC87 FADD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC88 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC89 FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8A FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8B FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8C FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8D FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8E FMUL r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC8F FMUL r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC90 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC91 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC92 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC93 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC94 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC95 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC96 FCOM r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC97 FCOM r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDC98 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC99 FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9A FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9B FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9C FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9D FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9E FCOMP r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDC9F FCOMP r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDCA0 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA1 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA2 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA3 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA4 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA5 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA6 FSUB r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCA7 FSUB r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDCA8 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCA9 FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAA FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAB FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAC FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAD FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAE FSUBR ST0 = m64 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCAF FSUBR ST0 = m64 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDCB0 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB1 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB2 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB3 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB4 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB5 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB6 FDIV r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDCB7 FDIV r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDCB8 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCB9 FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBA FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBB FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBC FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBD FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBE FDIVR ST0 = m64 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDCBF FDIVR ST0 = m64 / ST0 */

	{ ARG_NONE, 0, action_copy },	/* 0xDCC0 FADD ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC1 FADD ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC2 FADD ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC3 FADD ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC4 FADD ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC5 FADD ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC6 FADD ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC7 FADD ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDCC8 FMUL ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCC9 FMUL ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCA FMUL ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCB FMUL ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCC FMUL ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCD FMUL ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCE FMUL ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCCF FMUL ST(0), ST(7) */

	{ ARG_NONE, 0, action_fail },	/* 0xDCD0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDCD8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCD9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDCDF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDCE0 FSUBR ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE1 FSUBR ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE2 FSUBR ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE3 FSUBR ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE4 FSUBR ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE5 FSUBR ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE6 FSUBR ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE7 FSUBR ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDCE8 FSUB ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCE9 FSUB ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCEA FSUB ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCEB FSUB ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCEC FSUB ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCED FSUB ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCEE FSUB ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCEF FSUB ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDCF0 FDIVR ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF1 FDIVR ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF2 FDIVR ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF3 FDIVR ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF4 FDIVR ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF5 FDIVR ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF6 FDIVR ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF7 FDIVR ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDCF8 FDIV ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCF9 FDIV ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCFA FDIV ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCFB FDIV ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCFC FDIV ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCFD FDIV ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDCFE FDIV ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy }	/* 0xDCFF FDIV ST(0), ST(7) */
};


/** fpu OPCODE 0xDD */
const struct ia32_opcode opcode_table_DD_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDD00 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD01 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD02 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD03 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD04 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD05 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD06 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD07 FLD r/m64, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDD08  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD09  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD0F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDD10 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD11 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD12 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD13 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD14 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD15 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD16 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD17 FST ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDD18 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD19 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1A FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1B FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1C FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1D FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1E FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD1F FSTP ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDD20 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD21 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD22 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD23 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD24 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD25 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD26 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD27 FRSTOR, r/m864 */

	{ ARG_NONE, 0, action_fail },	/* 0xDD28  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD29  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD2F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDD30 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD31 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD32 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD33 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD34 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD35 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD36 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD37 FSAVE, m864 */

	{ ARG_RM, 0, action_copy },	/* 0xDD38 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD39 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3A FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3B FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3C FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3D FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3E FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD3F FSTSW, m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDD40 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD41 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD42 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD43 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD44 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD45 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD46 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD47 FLD r/m64, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDD48  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD49  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD4F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDD50 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD51 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD52 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD53 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD54 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD55 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD56 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD57 FST ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDD58 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD59 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5A FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5B FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5C FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5D FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5E FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD5F FSTP ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDD60 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD61 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD62 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD63 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD64 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD65 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD66 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD67 FRSTOR, r/m864 */

	{ ARG_NONE, 0, action_fail },	/* 0xDD68  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD69  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD6F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDD70 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD71 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD72 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD73 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD74 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD75 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD76 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDD77 FSAVE, m864 */

	{ ARG_RM, 0, action_copy },	/* 0xDD78 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD79 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7A FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7B FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7C FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7D FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7E FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDD7F FSTSW, m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDD80 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD81 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD82 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD83 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD84 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD85 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD86 FLD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDD87 FLD r/m64, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDD88  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD89  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDD8F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDD90 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD91 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD92 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD93 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD94 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD95 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD96 FST ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD97 FST ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDD98 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD99 FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9A FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9B FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9C FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9D FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9E FSTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDD9F FSTP ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDDA0 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA1 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA2 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA3 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA4 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA5 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA6 FRSTOR, r/m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDA7 FRSTOR, r/m864 */

	{ ARG_NONE, 0, action_fail },	/* 0xDDA8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDA9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDAF  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDDB0 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB1 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB2 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB3 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB4 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB5 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB6 FSAVE, m864 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB7 FSAVE, m864 */

	{ ARG_RM, 0, action_copy },	/* 0xDDB8 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDB9 FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBA FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBB FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBC FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBD FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBE FSTSW, m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDDBF FSTSW, m16 */

	{ ARG_NONE, 0, action_copy },	/* 0xDDC0 FFREE, ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC1 FFREE, ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC2 FFREE, ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC3 FFREE, ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC4 FFREE, ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC5 FFREE, ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC6 FFREE, ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDC7 FFREE, ST(7) */

	{ ARG_NONE, 0, action_fail },	/* 0xDDC8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDC9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDCF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDDD0 FST, ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD1 FST, ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD2 FST, ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD3 FST, ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD4 FST, ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD5 FST, ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD6 FST, ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD7 FST, ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDDD8 FSTP, ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDD9 FSTP, ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDA FSTP, ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDB FSTP, ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDC FSTP, ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDD FSTP, ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDE FSTP, ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDDF FSTP, ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDDE0 FUCOM ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE1 FUCOM ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE2 FUCOM ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE3 FUCOM ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE4 FUCOM ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE5 FUCOM ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE6 FUCOM ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE7 FUCOM ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDDE8 FUCOMP, ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDE9 FUCOMP, ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDEA FUCOMP, ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDEB FUCOMP, ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDEC FUCOMP, ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDED FUCOMP, ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDEE FUCOMP, ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDDEF FUCOMP, ST(7) */

	{ ARG_NONE, 0, action_fail },	/* 0xDDF0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDDF8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDF9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDFA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDFB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDFC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDFD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDDFE  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 0xDDFF  undefined opcode */
};


/** fpu OPCODE 0xDE */
const struct ia32_opcode opcode_table_DE_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDE00 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE01 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE02 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE03 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE04 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE05 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE06 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE07 FIADD r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE08 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE09 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0A FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0B FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0C FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0D FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0E FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE0F FIMUL r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE10 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE11 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE12 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE13 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE14 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE15 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE16 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE17 FICOM r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE18 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE19 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1A FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1B FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1C FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1D FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1E FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE1F FICOMP r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE20 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE21 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE22 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE23 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE24 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE25 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE26 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE27 FISUB r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE28 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE29 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2A FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2B FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2C FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2D FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2E FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE2F FISUBR T0 = m16 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDE30 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE31 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE32 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE33 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE34 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE35 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE36 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE37 FIDIV r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE38 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE39 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3A FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3B FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3C FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3D FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3E FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE3F FIDIVR T0 = m16 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDE40 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE41 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE42 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE43 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE44 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE45 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE46 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE47 FIADD r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE48 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE49 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4A FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4B FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4C FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4D FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4E FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE4F FIMUL r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE50 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE51 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE52 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE53 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE54 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE55 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE56 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE57 FICOM r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE58 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE59 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5A FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5B FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5C FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5D FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5E FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE5F FICOMP r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE60 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE61 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE62 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE63 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE64 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE65 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE66 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE67 FISUB r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE68 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE69 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6A FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6B FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6C FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6D FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6E FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE6F FISUBR T0 = m16 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDE70 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE71 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE72 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE73 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE74 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE75 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE76 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE77 FIDIV r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE78 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE79 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7A FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7B FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7C FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7D FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7E FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDE7F FIDIVR T0 = m16 / ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDE80 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE81 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE82 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE83 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE84 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE85 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE86 FIADD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE87 FIADD r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE88 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE89 FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8A FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8B FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8C FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8D FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8E FIMUL r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE8F FIMUL r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE90 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE91 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE92 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE93 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE94 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE95 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE96 FICOM r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE97 FICOM r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDE98 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE99 FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9A FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9B FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9C FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9D FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9E FICOMP r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDE9F FICOMP r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDEA0 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA1 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA2 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA3 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA4 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA5 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA6 FISUB r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEA7 FISUB r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDEA8 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEA9 FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAA FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAB FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAC FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAD FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAE FISUBR T0 = m16 - ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEAF FISUBR T0 = m16 - ST0 */

	{ ARG_RM, 0, action_copy },	/* 0xDEB0 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB1 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB2 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB3 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB4 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB5 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB6 FIDIV r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDEB7 FIDIV r/m16, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDEB8 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEB9 FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBA FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBB FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBC FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBD FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBE FIDIVR T0 = m16 / ST0 */
	{ ARG_RM, 0, action_copy },	/* 0xDEBF FIDIVR T0 = m16 / ST0 */

	{ ARG_NONE, 0, action_copy },	/* 0xDEC0 FADDP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC1 FADDP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC2 FADDP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC3 FADDP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC4 FADDP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC5 FADDP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC6 FADDP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC7 FADDP ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDEC8 FMULP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEC9 FMULP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECA FMULP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECB FMULP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECC FMULP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECD FMULP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECE FMULP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDECF FMULP ST(0), ST(7) */

	{ ARG_NONE, 0, action_fail },	/* 0xDED0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDED7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDED8  undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0xDED9 FCOMPP */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDEDF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDEE0 FSUBRP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE1 FSUBRP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE2 FSUBRP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE3 FSUBRP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE4 FSUBRP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE5 FSUBRP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE6 FSUBRP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE7 FSUBRP ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDEE8 FSUBP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEE9 FSUBP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEEA FSUBP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEEB FSUBP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEEC FSUBP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEED FSUBP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEEE FSUBP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEEF FSUBP ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDEF0 FDIVRP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF1 FDIVRP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF2 FDIVRP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF3 FDIVRP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF4 FDIVRP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF5 FDIVRP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF6 FDIVRP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF7 FDIVRP ST(0), ST(7) */

	{ ARG_NONE, 0, action_copy },	/* 0xDEF8 FDIVP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEF9 FDIVP ST(0), ST(1) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEFA FDIVP ST(0), ST(2) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEFB FDIVP ST(0), ST(3) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEFC FDIVP ST(0), ST(4) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEFD FDIVP ST(0), ST(5) */
	{ ARG_NONE, 0, action_copy },	/* 0xDEFE FDIVP ST(0), ST(6) */
	{ ARG_NONE, 0, action_copy }	/* 0xDEFF FDIVP ST(0), ST(7) */
};


/** fpu OPCODE 0xDF */
const struct ia32_opcode opcode_table_DF_fpu[] = {
	{ ARG_RM, 0, action_copy },	/* 0xDF00 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF01 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF02 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF03 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF04 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF05 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF06 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF07 FILD r/m16, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDF08  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF09  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF0F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDF10 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF11 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF12 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF13 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF14 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF15 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF16 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF17 FIST ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDF18 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF19 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1A FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1B FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1C FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1D FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1E FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF1F FISTP ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDF20 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF21 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF22 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF23 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF24 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF25 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF26 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF27 FBLD r/m80, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDF28 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF29 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2A FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2B FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2C FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2D FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2E FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF2F FILD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDF30 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF31 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF32 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF33 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF34 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF35 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF36 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF37 FBSTP ST(0), m80 */

	{ ARG_RM, 0, action_copy },	/* 0xDF38 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF39 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3A FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3B FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3C FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3D FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3E FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF3F FISTP ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDF40 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF41 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF42 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF43 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF44 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF45 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF46 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF47 FILD r/m16, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDF48  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF49  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF4F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDF50 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF51 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF52 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF53 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF54 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF55 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF56 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF57 FIST ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDF58 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF59 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5A FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5B FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5C FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5D FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5E FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF5F FISTP ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDF60 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF61 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF62 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF63 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF64 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF65 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF66 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF67 FBLD r/m80, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDF68 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF69 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6A FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6B FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6C FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6D FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6E FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF6F FILD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDF70 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF71 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF72 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF73 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF74 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF75 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF76 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDF77 FBSTP ST(0), m80 */

	{ ARG_RM, 0, action_copy },	/* 0xDF78 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF79 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7A FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7B FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7C FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7D FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7E FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDF7F FISTP ST(0), m64 */

	{ ARG_RM, 0, action_copy },	/* 0xDF80 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF81 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF82 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF83 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF84 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF85 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF86 FILD r/m16, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDF87 FILD r/m16, ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDF88  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF89  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDF8F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0xDF90 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF91 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF92 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF93 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF94 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF95 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF96 FIST ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF97 FIST ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDF98 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF99 FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9A FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9B FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9C FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9D FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9E FISTP ST(0), m16 */
	{ ARG_RM, 0, action_copy },	/* 0xDF9F FISTP ST(0), m16 */

	{ ARG_RM, 0, action_copy },	/* 0xDFA0 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA1 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA2 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA3 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA4 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA5 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA6 FBLD r/m80, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA7 FBLD r/m80, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDFA8 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFA9 FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAA FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAB FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAC FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAD FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAE FILD r/m64, ST(0) */
	{ ARG_RM, 0, action_copy },	/* 0xDFAF FILD r/m64, ST(0) */

	{ ARG_RM, 0, action_copy },	/* 0xDFB0 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB1 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB2 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB3 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB4 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB5 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB6 FBSTP ST(0), m80 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB7 FBSTP ST(0), m80 */

	{ ARG_RM, 0, action_copy },	/* 0xDFB8 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFB9 FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBA FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBB FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBC FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBD FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBE FISTP ST(0), m64 */
	{ ARG_RM, 0, action_copy },	/* 0xDFBF FISTP ST(0), m64 */

	{ ARG_NONE, 0, action_fail },	/* 0xDFC0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDFC8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFC9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFCF  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDFD0  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD7  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0xDFD8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFD9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDE  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFDF  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDFE0 FSTSW, AX */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE1  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE2  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE3  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE4  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE5  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFE7  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0xDFE8 FUCOMIP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFE9 FUCOMIP ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFEA FUCOMIP ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFEB FUCOMIP ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFEC FUCOMIP ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFED FUCOMIP ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFEE FUCOMIP ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFEF FUCOMIP ST(7), ST(0) */

	{ ARG_NONE, 0, action_copy },	/* 0xDFF0 FCOMIP ST(0), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF1 FCOMIP ST(1), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF2 FCOMIP ST(2), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF3 FCOMIP ST(3), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF4 FCOMIP ST(4), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF5 FCOMIP ST(5), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF6 FCOMIP ST(6), ST(0) */
	{ ARG_NONE, 0, action_copy },	/* 0xDFF7 FCOMIP ST(7), ST(0) */

	{ ARG_NONE, 0, action_fail },	/* 0xDFF8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFF9  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFFA  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFFB  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFFC  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFFD  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0xDFFE  undefined opcode */
	{ ARG_NONE, 0, action_fail }	/* 0xDFFF  undefined opcode */
};




/*
 * tables for one and two byte opcodes.
 */

/** table for two byte opcodes with 0x66 (Operand-size override) PREFIX */
const struct ia32_opcode opcode_table_660F[] = {
	{ ARG_NONE, 0, 0 },	/* 0x660F00  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F01  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F02  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F03  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F04  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F05  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F06  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F07  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F08  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F09  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F0F  undefined opcode */

	{ ARG_RM, 0, action_warn },	/* 0x660F10 MOVUPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F11 MOVUPD reg128, r/m128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F12 MOVLPD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x660F13 MOVLPD reg64, m64 */
	{ ARG_RM, 0, action_warn },	/* 0x660F14 UNPCKLPD r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F15 UNPCKHPD r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F16 MOVHPD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x660F17 MOVHPD reg64, m64 */

	{ ARG_NONE, 0, 0 },	/* 0x660F18  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F19  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F1F  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F20  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F21  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F22  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F23  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F24  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F25  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F26  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F27  undefined opcode */

	{ ARG_RM, 0, action_warn },	/* 0x660F28 MOVAPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F29 MOVAPD reg128, r/m128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2A CVTPI2PD r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2B MOVNTPD reg128, m128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2C CVTTPD2PI r/m128, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2D CVTPD2PI r/m128, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2E UCOMISD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F2F COMISD r/m128, reg128 */

	{ ARG_NONE, 0, 0 },	/* 0x660F30  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F31  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F32  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F33  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F34  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F35  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F36  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F37  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F38  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F39  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F3F  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F40  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F41  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F42  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F43  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F44  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F45  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F46  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F47  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F48  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F49  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F4F  undefined opcode */

	{ ARG_RM, 0, action_warn },	/* 0x660F50 MOVMSKPD reg128, reg32 */
	{ ARG_RM, 0, action_warn },	/* 0x660F51 SQRTPD r/m128, reg128 */
	{ ARG_NONE, 0, 0 },	/* 0x660F52  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F53  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x660F54 ANDPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F55 ANDNPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F56 ORPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F57 XORPD r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660F58 ADDPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F59 MULPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5A CVTPD2PS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5B CVTPS2DQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5C SUBPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5D MINPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5E DIVPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F5F MAXPD r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660F60 PUNPCKLBW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F61 PUNPCKLWD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F62 PUNPCKLDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F63 PACKSSWB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F64 PCMPGTB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F65 PCMPGTW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F66 PCMPGTD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F67 PACKUSWB r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660F68 PUNPCKHBW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F69 PUNPCKHWD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6A PUNPCKHDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6B PACKSSDW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6C PUNPACKLQDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6D PUNPACKHQDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6E MOVD r/m32, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F6F MOVDQA r/m128, reg128 */

	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x660F70 PSHUFD r/m128, imm8, reg128 */
	{ ARG_NONE, 0, 0 },	/* 0x660F71  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F72  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F73  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x660F74 PCMPEQB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F75 PCMPEQW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F76 PCMPEQD r/m128, reg128 */
	{ ARG_NONE, 0, 0 },	/* 0x660F77  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F78  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F79  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F7A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F7B  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x660F7C HADDPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F7D HSUBPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660F7E MOVD reg128, r/m32 */
	{ ARG_RM, 0, action_warn },	/* 0x660F7F MOVQ reg128, r/m128 */

	{ ARG_NONE, 0, 0 },	/* 0x660F80  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F81  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F82  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F83  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F84  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F85  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F86  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F87  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F88  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F89  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F8F  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F90  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F91  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F92  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F93  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F94  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F95  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F96  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F97  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660F98  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F99  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9A  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9B  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9C  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9D  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9E  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660F9F  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FA0  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA1  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA2  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA3  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA4  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA5  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA6  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA7  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FA8  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FA9  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAA  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAB  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAC  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAD  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAE  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FAF  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FB0  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB1  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB2  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB3  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB4  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB5  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB6  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB7  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FB8  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FB9  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBA  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBB  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBC  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBD  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBE  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FBF  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FC0  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FC1  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FC2  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x660FC3 CMPPD r/m128, imm8, reg128 */
	{ ARG_NONE, 0, 0 },	/* 0x660FC4  undefined opcode */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x660FC5 PINSRW r/m16, imm8, reg128 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x660FC6 PEXTRW reg128, imm8, reg32 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x660FC7 SHUFPD r/m128, imm8, reg128 */

	{ ARG_NONE, 0, 0 },	/* 0x660FC8  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FC9  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCA  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCB  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCC  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCD  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCE  undefined opcode */
	{ ARG_NONE, 0, 0 },	/* 0x660FCF  undefined opcode */

	{ ARG_NONE, 0, 0 },	/* 0x660FD0  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x660FD1 ADDSUBPD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD2 PSRLW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD3 PSRLD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD4 PSRLQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD5 PADDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD6 PMULLW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD7 MOVQ reg64, r/m64 */

	{ ARG_RM, 0, action_warn },	/* 0x660FD8 PMOVMSKB reg128, reg32 */
	{ ARG_RM, 0, action_warn },	/* 0x660FD9 PSUBUSB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDA PSUBUSW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDB PMINUB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDC PAND r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDD PADDUSB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDE PADDUSW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FDF PMAXUB r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660FE0 PANDN r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE1 PAVGB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE2 PSRAW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE3 PSRAD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE4 PAVGW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE5 PMULHUW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE6 PMULHW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE7 CVTTPD2DQ r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660FE8 MOVNTDQ reg128, m128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FE9 PSUBSB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FEA PSUBSW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FEB PMINSW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FEC POR r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FED PADDSB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FEE PADDSW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FEF PMAXSW r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660FF0 PXOR r/m128, reg128 */
	{ ARG_NONE, 0, 0 },	/* 0x660FF1  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x660FF2 PSLLW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF3 PSLLD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF4 PSLLQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF5 PMULUDQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF6 PMADDWD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF7 PSADBW r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x660FF8 MASKMOVDQU reg128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FF9 PSUBB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FFA PSUBW r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FFB PSUBD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FFC PSUBQ r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FFD PADDB r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x660FFE PADDW r/m128, reg128 */
	{ ARG_RM, 0, action_warn }	/* 0x660FFF PADDD r/m128, reg128 */
};


/** table for two byte opcodes with 0xF2 (REPZ override) PREFIX */
const struct ia32_opcode opcode_table_F20F[] = {
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F00   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F01   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F02   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F03   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F04   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F05   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F06   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F07   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F08   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F09   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F0F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F10   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F11   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F12   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F13   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F14   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F15   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F16   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F17   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F18   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F19   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F1F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F20   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F21   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F22   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F23   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F24   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F25   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F26   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F27   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F28   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F29   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F2F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F30   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F31   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F32   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F33   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F34   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F35   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F36   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F37   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F38   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F39   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F3F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F40   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F41   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F42   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F43   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F44   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F45   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F46   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F47   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F48   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F49   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F4F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F50   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F51   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F52   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F53   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F54   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F55   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F56   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F57   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F58   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F59   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F5F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F60   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F61   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F62   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F63   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F64   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F65   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F66   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F67   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F68   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F69   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F6F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F70   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F71   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F72   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F73   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F74   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F75   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F76   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F77   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F78   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F79   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F7F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F80   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F81   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F82   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F83   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F84   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F85   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F86   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F87   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F88   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F89   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F8F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F90   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F91   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F92   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F93   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F94   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F95   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F96   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F97   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F98   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F99   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9A   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9B   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9C   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9D   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9E   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20F9F   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FA9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAD   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FAF   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FB9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBD   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FBF   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FC9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCD   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FCF   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FD9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDD   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FDF   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FE9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FEA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FEB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FEC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FED   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FEE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FEF   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF0   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF1   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF2   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF3   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF4   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF5   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF6   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF7   repne prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF8   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FF9   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FFA   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FFB   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FFC   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FFD   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF20FFE   repne prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED }	/* 0xF20FFF   repne prefix */
};


/** table for two byte opcodes with 0xF3 (REPZ override) PREFIX */
const struct ia32_opcode opcode_table_F30F[] = {
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F00  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F01  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F02  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F03  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F04  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F05  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F06  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F07  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F08  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F09  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F0F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F10  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F11  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F12  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F13  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F14  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F15  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F16  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F17  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F18  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F19  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F1F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F20  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F21  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F22  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F23  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F24  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F25  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F26  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F27  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F28  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F29  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F2F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F30  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F31  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F32  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F33  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F34  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F35  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F36  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F37  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F38  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F39  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F3F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F40  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F41  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F42  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F43  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F44  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F45  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F46  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F47  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F48  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F49  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F4F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F50  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F51  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F52  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F53  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F54  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F55  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F56  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F57  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F58  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F59  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F5F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F60  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F61  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F62  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F63  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F64  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F65  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F66  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F67  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F68  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F69  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F6F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F70  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F71  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F72  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F73  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F74  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F75  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F76  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F77  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F78  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F79  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F7F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F80  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F81  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F82  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F83  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F84  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F85  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F86  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F87  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F88  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F89  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F8F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F90  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F91  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F92  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F93  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F94  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F95  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F96  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F97  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F98  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F99  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9A  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9B  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9C  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9D  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9E  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30F9F  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FA9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAD  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FAF  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FB9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBD  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FBF  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FC9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCD  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FCF  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FD9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDD  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FDF  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FE9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FEA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FEB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FEC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FED  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FEE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FEF  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF0  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF1  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF2  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF3  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF4  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF5  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF6  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF7  rep prefix */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF8  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FF9  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FFA  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FFB  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FFC  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FFD  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF30FFE  rep prefix */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED }	/* 0xF30FFF  rep prefix */
};


/** table for two byte opcodes */
const struct ia32_opcode opcode_table_twobyte[] = {
	{ OPCODE_RM, opcode_table_0F00_rm, NO_ACTION_CALLED },	/* 0x0F00  group 6 */
	{ OPCODE_RM, opcode_table_0F01_rm, NO_ACTION_CALLED },	/* 0x0F01  group 7 */
	{ ARG_RM, 0, action_copy },	/* 0x0F02 LAR r/m16, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F03 LSL r/m16, reg32 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F04  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F05  undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F06 CLTS */
	{ ARG_NONE, 0, action_fail },	/* 0x0F07  undefined opcode */

	{ ARG_NONE, 0, action_warn },	/* 0x0F08 INVD */
	{ ARG_NONE, 0, action_warn },	/* 0x0F09 WBINVD */
	{ ARG_NONE, 0, action_fail },	/* 0x0F0A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F0B UD2 illegal opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F0C  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 0x0F0D NOP, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F0E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F0F  undefined opcode */

	{ ARG_RM, 0, action_warn },	/* 0x0F10 MOVUPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F11 MOVUPS reg128, r/m128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F12 MOVLPS r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F13 MOVLPS reg64, m64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F14 UNPCKLPS r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F15 UNPCKHPS r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F16 MOVHPS r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F17 MOVHPS reg64, m64 */

	{ OPCODE_RM, opcode_table_0F18_rm, NO_ACTION_CALLED },	/* 0x0F18  group 16 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F19  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F1A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F1B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F1C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F1D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F1E  undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 0x0F1F NOP, r/m32 */

	{ ARG_RM, 0, action_copy },	/* 0x0F20 MOV reg32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F21 MOV reg32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F22 MOV reg32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F23 MOV reg32, reg32 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F24  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F25  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F26  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F27  undefined opcode */

	{ ARG_RM, 0, action_warn },	/* 0x0F28 MOVAPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F29 MOVAPS reg128, r/m128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2A CVTPI2PS r/m64, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2B MOVNTPS reg128, m128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2C CVTTPS2PI r/m128, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2D CVTPS2PI r/m128, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2E UCOMISS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F2F COMISS r/m128, reg128 */

	{ ARG_NONE, 0, action_copy },	/* 0x0F30 WRMSR */
	{ ARG_NONE, 0, action_copy },	/* 0x0F31 RDTSC */
	{ ARG_NONE, 0, action_copy },	/* 0x0F32 RDMSR */
	{ ARG_NONE, 0, action_copy },	/* 0x0F33 RDPMC */
	{ ARG_NONE, 0, action_sysenter },	/* 0x0F34 SYSENTER */
	{ ARG_NONE, 0, action_warn },	/* 0x0F35 SYSEXIT */
	{ ARG_NONE, 0, action_fail },	/* 0x0F36  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F37  undefined opcode */

	{ ARG_NONE, 0, action_fail },	/* 0x0F38  escape for three byte opcodes */
	{ ARG_NONE, 0, action_fail },	/* 0x0F39  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3A  escape for three byte opcodes */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3D  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3E  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F3F  undefined opcode */

	{ ARG_RM, 0, action_copy },	/* 0x0F40 CMOVO r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F41 CMOVNO r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F42 CMOVC r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F43 CMOVNC r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F44 CMOVZ r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F45 CMOVNZ r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F46 CMOVBE r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F47 CMOVA r/m32, reg32 */

	{ ARG_RM, 0, action_copy },	/* 0x0F48 CMOVS r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F49 CMOVNS r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4A CMOVPE r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4B CMOVPO r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4C CMOVL r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4D CMOVGE r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4E CMOVLE r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0F4F CMOVG r/m32, reg32 */

	{ ARG_RM, 0, action_warn },	/* 0x0F50 MOVMSKPS reg128, reg32 */
	{ ARG_RM, 0, action_warn },	/* 0x0F51 SQRTPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F52 RSQRTPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F53 RCPPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F54 ANDPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F55 ANDNPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F56 ORPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F57 XORPS r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x0F58 ADDPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F59 MULPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5A CVTPS2PD r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5B CVTDQ2PS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5C SUBPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5D MINPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5E DIVPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0F5F MAXPS r/m128, reg128 */

	{ ARG_RM, 0, action_warn },	/* 0x0F60 PUNPCKLBW r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F61 PUNPCKLWD r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F62 PUNPCKLDQ r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F63 PACKSSWB r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F64 PCMPGTB r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F65 PCMPGTW r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F66 PCMPGTD r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F67 PACKUSWB r/m32, reg64 */

	{ ARG_RM, 0, action_warn },	/* 0x0F68 PUNPCKHBW r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F69 PUNPCKHWD r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F6A PUNPCKHDQ r/m32, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F6B PACKSSDW r/m32, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F6C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F6D  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0F6E MOVD r/m32, reg32 */
	{ ARG_RM, 0, action_warn },	/* 0x0F6F MOVQ r/m64, reg64 */

	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x0F70 PSHUF r/m64, imm8, reg64 */
	{ OPCODE_RM, opcode_table_0F71_rm, NO_ACTION_CALLED },	/* 0x0F71  group 12 */
	{ OPCODE_RM, opcode_table_0F72_rm, NO_ACTION_CALLED },	/* 0x0F72  group 13 */
	{ OPCODE_RM, opcode_table_0F73_rm, NO_ACTION_CALLED },	/* 0x0F73  group 14 */
	{ ARG_RM, 0, action_warn },	/* 0x0F74 PCMPEQB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F75 PCMPEQW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0F76 PCMPEQD r/m64, reg64 */
	{ ARG_NONE, 0, action_warn },	/* 0x0F77 EMMS */

	{ ARG_RM, 0, action_warn },	/* 0x0F78 VMREAD reg32, r/m32 */
	{ ARG_RM, 0, action_warn },	/* 0x0F79 VMWRITE r/m32, reg32 */
	{ ARG_NONE, 0, action_fail },	/* 0x0F7A  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F7B  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F7C  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0F7D  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0F7E MOVD reg32, r/m32 */
	{ ARG_RM, 0, action_warn },	/* 0x0F7F MOVQ reg64, r/m64 */

	{ ARG_D, 0, action_jcc },	/* 0x0F80 JO, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F81 JNO, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F82 JC, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F83 JNC, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F84 JZ, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F85 JNZ, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F86 JBE, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F87 JA, rel32 */

	{ ARG_D, 0, action_jcc },	/* 0x0F88 JS, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F89 JNS, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8A JPE, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8B JPO, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8C JL, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8D JGE, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8E JLE, rel32 */
	{ ARG_D, 0, action_jcc },	/* 0x0F8F JG, rel32 */

	{ ARG_RM, 0, action_copy },	/* 0x0F90 SETO, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F91 SETNO, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F92 SETC, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F93 SETNC, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F94 SETZ, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F95 SETNZ, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F96 SETBE, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F97 SETA, r/m8 */

	{ ARG_RM, 0, action_copy },	/* 0x0F98 SETS, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F99 SETNS, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9A SETPE, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9B SETPO, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9C SETL, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9D SETGE, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9E SETLE, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0F9F SETG, r/m8 */

	{ ARG_NONE, 0, action_copy },	/* 0x0FA0 PUSH, DS */
	{ ARG_NONE, 0, action_copy },	/* 0x0FA1 POP, DS */
	{ ARG_NONE, 0, action_copy },	/* 0x0FA2 CPUID */
	{ ARG_RM, 0, action_copy },	/* 0x0FA3 BT reg32, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 0x0FA4 SHLD reg32, imm8, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FA5 SHLD reg32, CL, r/m32 */
	{ ARG_NONE, 0, action_fail },	/* 0x0FA6  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0FA7  undefined opcode */

	{ ARG_NONE, 0, action_copy },	/* 0x0FA8 PUSH, FS */
	{ ARG_NONE, 0, action_copy },	/* 0x0FA9 POP, FS */
	{ ARG_NONE, 0, action_copy },	/* 0x0FAA RSM */
	{ ARG_RM, 0, action_copy },	/* 0x0FAB BTS reg32, r/m32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 0x0FAC SHRD reg32, imm8, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FAD SHRD reg32, CL, r/m32 */
	{ OPCODE_RM, opcode_table_0FAE_rm, NO_ACTION_CALLED },	/* 0x0FAE  group 15 */
	{ ARG_RM, 0, action_copy },	/* 0x0FAF IMUL r/m32, reg32 */

	{ ARG_RM, 0, action_copy },	/* 0x0FB0 CMPXCHG reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB1 CMPXCHG reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB2 LSS r/m48, GS, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB3 BTR reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB4 LFS r/m48, DS, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB5 LGS r/m48, FS, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB6 MOVZX r/m8, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FB7 MOVZX r/m16, reg32 */

	{ ARG_NONE, 0, action_fail },	/* 0x0FB8  undefined opcode */
	{ ARG_NONE, 0, action_fail },	/* 0x0FB9 UD1 undefined opcode? */
	{ OPCODE_RM, opcode_table_0FBA_rm, NO_ACTION_CALLED },	/* 0x0FBA  group 8 */
	{ ARG_RM, 0, action_copy },	/* 0x0FBB BTC reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FBC BSF r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FBD BSR r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FBE MOVSX r/m8, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x0FBF MOVSX r/m16, reg32 */

	{ ARG_RM, 0, action_copy },	/* 0x0FC0 XADD reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x0FC1 XADD reg32, r/m32 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x0FC2 CMPPS r/m128, imm8, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0FC3 MOVNTI reg32, m32 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x0FC4 PINSRW r/m16, imm8, reg64 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x0FC5 PEXTRW reg64, imm8, reg32 */
	{ ARG_B | ARG_RM, 0, action_warn },	/* 0x0FC6 SHUFPS r/m128, imm8, reg128 */
	{ OPCODE_RM, opcode_table_0FC7_rm, NO_ACTION_CALLED },	/* 0x0FC7  group 9 */

	{ ARG_NONE, 0, action_copy },	/* 0x0FC8 BSWAP, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x0FC9 BSWAP, ECX */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCA BSWAP, EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCB BSWAP, EBX */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCC BSWAP, ESP */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCD BSWAP, EBP */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCE BSWAP, ESI */
	{ ARG_NONE, 0, action_copy },	/* 0x0FCF BSWAP, EDI */

	{ ARG_RM, 0, action_warn },	/* 0x0FD0 ADDSUBPS r/m128, reg128 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD1 PSRLW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD2 PSRLD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD3 PSRLQ r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD4 PADDQ r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD5 PMULLW r/m64, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 0x0FD6  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0FD7 PMOVMSKB reg64, reg32 */

	{ ARG_RM, 0, action_warn },	/* 0x0FD8 PSUBUSB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FD9 PSUBUSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDA PMINUB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDB PAND r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDC PADDUSB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDD PADDUSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDE PMAXUB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FDF PANDN r/m64, reg64 */

	{ ARG_RM, 0, action_warn },	/* 0x0FE0 PAVGB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE1 PSRAW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE2 PSRAD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE3 PAVGW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE4 PMULHUW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE5 PMULHW r/m64, reg64 */
	{ ARG_NONE, 0, action_fail },	/* 0x0FE6  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0FE7 MOVNTQ reg64, m64 */

	{ ARG_RM, 0, action_warn },	/* 0x0FE8 PSUBSB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FE9 PSUBSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FEA PMINSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FEB POR r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FEC PADDSB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FED PADDSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FEE PMAXSW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FEF PXOR r/m64, reg64 */

	{ ARG_NONE, 0, action_fail },	/* 0x0FF0  undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0FF1 PSLLW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF2 PSLLD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF3 PSLLQ r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF4 PMULUDQ r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF5 PMADDWD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF6 PSADBW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF7 MASKMOVQ reg64, reg64 */

	{ ARG_RM, 0, action_warn },	/* 0x0FF8 PSUBB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FF9 PSUBW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FFA PSUBD r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FFB PSUBQ r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FFC PADDB r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FFD PADDW r/m64, reg64 */
	{ ARG_RM, 0, action_warn },	/* 0x0FFE PADDD r/m64, reg64 */
	{ ARG_NONE, 0, action_fail }	/* 0x0FFF  undefined opcode */
};


/** table for one byte opcodes */
const struct ia32_opcode opcode_table_onebyte[] = {
	{ ARG_RM, 0, action_copy },	/* 0x00 ADD reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x01 ADD reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x02 ADD r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x03 ADD r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x04 ADD imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x05 ADD imm32, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x06 PUSH, SS */
	{ ARG_NONE, 0, action_copy },	/* 0x07 POP, SS */

	{ ARG_RM, 0, action_copy },	/* 0x08 OR reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x09 OR reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x0A OR r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x0B OR r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x0C OR imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x0D OR imm32, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x0E PUSH, ES */
	{ OPCODE_ESC, opcode_table_twobyte, NO_ACTION_CALLED },	/* 0x0F  escape for two byte opcodes */

	{ ARG_RM, 0, action_copy },	/* 0x10 ADC reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x11 ADC reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x12 ADC r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x13 ADC r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x14 ADC imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x15 ADC imm32, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x16 PUSH, GS */
	{ ARG_NONE, 0, action_copy },	/* 0x17 POP, GS */

	{ ARG_RM, 0, action_copy },	/* 0x18 SBB reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x19 SBB reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x1A SBB r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x1B SBB r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x1C SBB imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x1D SBB imm32, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x1E PUSH, CS */
	{ ARG_NONE, 0, action_copy },	/* 0x1F POP, CS */

	{ ARG_RM, 0, action_copy },	/* 0x20 AND reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x21 AND reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x22 AND r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x23 AND r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x24 AND imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x25 AND imm32, EAX */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x26  prefix ES segment override */
	{ ARG_NONE, 0, action_copy },	/* 0x27 DAA */

	{ ARG_RM, 0, action_copy },	/* 0x28 SUB reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x29 SUB reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x2A SUB r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x2B SUB r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x2C SUB imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x2D SUB imm32, EAX */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x2E  prefix CS segment override */
	{ ARG_NONE, 0, action_copy },	/* 0x2F DAS */

	{ ARG_RM, 0, action_copy },	/* 0x30 XOR reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x31 XOR reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x32 XOR r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x33 XOR r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x34 XOR imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x35 XOR imm32, EAX */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x36  prefix SS segment override */
	{ ARG_NONE, 0, action_copy },	/* 0x37 AAA */

	{ ARG_RM, 0, action_copy },	/* 0x38 CMP reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x39 CMP reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x3A CMP r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x3B CMP r/m32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x3C CMP imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0x3D CMP imm32, EAX */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x3E  prefix DS segment override */
	{ ARG_NONE, 0, action_copy },	/* 0x3F AAS */

	{ ARG_NONE, 0, action_copy },	/* 0x40 INC, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x41 INC, ECX */
	{ ARG_NONE, 0, action_copy },	/* 0x42 INC, EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x43 INC, EBX */
	{ ARG_NONE, 0, action_copy },	/* 0x44 INC, ESP */
	{ ARG_NONE, 0, action_copy },	/* 0x45 INC, EBP */
	{ ARG_NONE, 0, action_copy },	/* 0x46 INC, ESI */
	{ ARG_NONE, 0, action_copy },	/* 0x47 INC, EDI */

	{ ARG_NONE, 0, action_copy },	/* 0x48 DEC, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x49 DEC, ECX */
	{ ARG_NONE, 0, action_copy },	/* 0x4A DEC, EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x4B DEC, EBX */
	{ ARG_NONE, 0, action_copy },	/* 0x4C DEC, ESP */
	{ ARG_NONE, 0, action_copy },	/* 0x4D DEC, EBP */
	{ ARG_NONE, 0, action_copy },	/* 0x4E DEC, ESI */
	{ ARG_NONE, 0, action_copy },	/* 0x4F DEC, EDI */

	{ ARG_NONE, 0, action_copy },	/* 0x50 PUSH, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x51 PUSH, ECX */
	{ ARG_NONE, 0, action_copy },	/* 0x52 PUSH, EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x53 PUSH, EBX */
	{ ARG_NONE, 0, action_copy },	/* 0x54 PUSH, ESP */
	{ ARG_NONE, 0, action_copy },	/* 0x55 PUSH, EBP */
	{ ARG_NONE, 0, action_copy },	/* 0x56 PUSH, ESI */
	{ ARG_NONE, 0, action_copy },	/* 0x57 PUSH, EDI */

	{ ARG_NONE, 0, action_copy },	/* 0x58 POP, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x59 POP, ECX */
	{ ARG_NONE, 0, action_copy },	/* 0x5A POP, EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x5B POP, EBX */
	{ ARG_NONE, 0, action_copy },	/* 0x5C POP, ESP */
	{ ARG_NONE, 0, action_copy },	/* 0x5D POP, EBP */
	{ ARG_NONE, 0, action_copy },	/* 0x5E POP, ESI */
	{ ARG_NONE, 0, action_copy },	/* 0x5F POP, EDI */

	{ ARG_NONE, 0, action_copy },	/* 0x60 PUSHAD */
	{ ARG_NONE, 0, action_copy },	/* 0x61 POPAD */
	{ ARG_RM, 0, action_copy },	/* 0x62 BOUND r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x63 ARPL reg16, r/m16 */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x64  prefix FS segment override */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x65  prefix GS segment override */
	{ PREFIX_ESC, opcode_table_660F, NO_ACTION_CALLED },	/* 0x66  prefix operand size override */
	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0x67  prefix address size override */

	{ ARG_D, 0, action_copy },	/* 0x68 PUSH, imm32 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 0x69 IMUL r/m32, imm32, reg32 */
	{ ARG_B, 0, action_copy },	/* 0x6A PUSH, imm8 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 0x6B IMUL r/m32, imm8, reg32 */
	{ ARG_NONE, 0, action_copy },	/* 0x6C INSB EDX,  */
	{ ARG_NONE, 0, action_copy },	/* 0x6D INSD EDX,  */
	{ ARG_NONE, 0, action_copy },	/* 0x6E OUTSB , EDX */
	{ ARG_NONE, 0, action_copy },	/* 0x6F OUTSB , EDX */

	{ ARG_B, 0, action_jcc },	/* 0x70 JO, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x71 JNO, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x72 JC, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x73 JNC, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x74 JZ, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x75 JNZ, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x76 JBE, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x77 JA, rel8 */

	{ ARG_B, 0, action_jcc },	/* 0x78 JS, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x79 JNS, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7A JPE, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7B JPO, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7C JL, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7D JGE, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7E JLE, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0x7F JG, rel8 */

	{ OPCODE_RM, opcode_table_80_rm, NO_ACTION_CALLED },	/* 0x80  immediate group 1 */
	{ OPCODE_RM, opcode_table_81_rm, NO_ACTION_CALLED },	/* 0x81  immediate group 1 */
	{ OPCODE_RM, opcode_table_82_rm, NO_ACTION_CALLED },	/* 0x82  immediate group 1 */
	{ OPCODE_RM, opcode_table_83_rm, NO_ACTION_CALLED },	/* 0x83  immediate group 1 */
	{ ARG_RM, 0, action_copy },	/* 0x84 TEST reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x85 TEST reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x86 XCHG reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x87 XCHG reg32, r/m32 */

	{ ARG_RM, 0, action_copy },	/* 0x88 MOV reg8, r/m8 */
	{ ARG_RM, 0, action_copy },	/* 0x89 MOV reg32, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x8A MOV r/m8, reg8 */
	{ ARG_RM, 0, action_copy },	/* 0x8B MOV r/m32, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x8C MOV reg16, r/m32 */
	{ ARG_RM, 0, action_copy },	/* 0x8D LEA m0, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0x8E MOV r/m16, reg16 */
	{ ARG_RM, 0, action_copy },	/* 0x8F POP, r/m32 */

	{ ARG_NONE, 0, action_copy },	/* 0x90 NOP */
	{ ARG_NONE, 0, action_copy },	/* 0x91 XCHG ECX, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x92 XCHG EDX, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x93 XCHG EBX, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x94 XCHG ESP, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x95 XCHG EBP, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x96 XCHG ESI, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0x97 XCHG EDI, EAX */

	{ ARG_NONE, 0, action_copy },	/* 0x98 CWDE */
	{ ARG_NONE, 0, action_copy },	/* 0x99 CDQ */
	{ ARG_P, 0, action_warn },	/* 0x9A CALLF, rel48 */
	{ ARG_NONE, 0, action_copy },	/* 0x9B WAIT */
	{ ARG_NONE, 0, action_copy },	/* 0x9C PUSHFD */
	{ ARG_NONE, 0, action_copy },	/* 0x9D POPFD */
	{ ARG_NONE, 0, action_copy },	/* 0x9E SAHF */
	{ ARG_NONE, 0, action_copy },	/* 0x9F LAHF */

	{ ARG_D, 0, action_copy },	/* 0xA0 MOV only one byte is read, but location addr is 2 or 4 byte */
	{ ARG_D, 0, action_copy },	/* 0xA1 MOV (imm32), EAX */
	{ ARG_D, 0, action_copy },	/* 0xA2 MOV AL, (imm32) */
	{ ARG_D, 0, action_copy },	/* 0xA3 MOV EAX, (imm32) */
	{ ARG_NONE, 0, action_copy },	/* 0xA4 MOVSB */
	{ ARG_NONE, 0, action_copy },	/* 0xA5 MOVSD */
	{ ARG_NONE, 0, action_copy },	/* 0xA6 CMPSB */
	{ ARG_NONE, 0, action_copy },	/* 0xA7 CMPSD */

	{ ARG_B, 0, action_copy },	/* 0xA8 TEST imm8, AL */
	{ ARG_D, 0, action_copy },	/* 0xA9 TEST imm32, EAX */
	{ ARG_NONE, 0, action_copy },	/* 0xAA STOSB */
	{ ARG_NONE, 0, action_copy },	/* 0xAB STOSD */
	{ ARG_NONE, 0, action_copy },	/* 0xAC LODSB */
	{ ARG_NONE, 0, action_copy },	/* 0xAD LODSD */
	{ ARG_NONE, 0, action_copy },	/* 0xAE SCASB */
	{ ARG_NONE, 0, action_copy },	/* 0xAF SCASD */

	{ ARG_B, 0, action_copy },	/* 0xB0 MOV imm8, AL */
	{ ARG_B, 0, action_copy },	/* 0xB1 MOV imm8, CL */
	{ ARG_B, 0, action_copy },	/* 0xB2 MOV imm8, DL */
	{ ARG_B, 0, action_copy },	/* 0xB3 MOV imm8, BL */
	{ ARG_B, 0, action_copy },	/* 0xB4 MOV imm8, AH */
	{ ARG_B, 0, action_copy },	/* 0xB5 MOV imm8, CH */
	{ ARG_B, 0, action_copy },	/* 0xB6 MOV imm8, DH */
	{ ARG_B, 0, action_copy },	/* 0xB7 MOV imm8, BH */

	{ ARG_D, 0, action_copy },	/* 0xB8 MOV imm32, EAX */
	{ ARG_D, 0, action_copy },	/* 0xB9 MOV imm32, ECX */
	{ ARG_D, 0, action_copy },	/* 0xBA MOV imm32, EDX */
	{ ARG_D, 0, action_copy },	/* 0xBB MOV imm32, EBX */
	{ ARG_D, 0, action_copy },	/* 0xBC MOV imm32, ESP */
	{ ARG_D, 0, action_copy },	/* 0xBD MOV imm32, EBP */
	{ ARG_D, 0, action_copy },	/* 0xBE MOV imm32, ESI */
	{ ARG_D, 0, action_copy },	/* 0xBF MOV imm32, EDI */

	{ OPCODE_RM, opcode_table_C0_rm, NO_ACTION_CALLED },	/* 0xC0  shift group 2 */
	{ OPCODE_RM, opcode_table_C1_rm, NO_ACTION_CALLED },	/* 0xC1  shift group 2 */
	{ ARG_W, 0, action_ret },	/* 0xC2 RET near return, pop imm16 bytes from stack */
	{ ARG_NONE, 0, action_ret },	/* 0xC3 RET near return */
	{ ARG_RM, 0, action_copy },	/* 0xC4 LES r/m48, reg32 */
	{ ARG_RM, 0, action_copy },	/* 0xC5 LDS r/m48, reg32 */
	{ ARG_B | ARG_RM, 0, action_copy },	/* 0xC6 MOV imm8, r/m8 */
	{ ARG_D | ARG_RM, 0, action_copy },	/* 0xC7 MOV imm32, r/m32 */

	{ ARG_W | ARG_B, 0, action_warn },	/* 0xC8 ENTER imm8, imm16 */
	{ ARG_NONE, 0, action_copy },	/* 0xC9 LEAVE */
	{ ARG_W, 0, action_warn },	/* 0xCA RETF far return - should not be used in our context */
	{ ARG_NONE, 0, action_warn },	/* 0xCB RETF far return - should not be used in our context */
	{ ARG_NONE, 0, action_warn },	/* 0xCC INT3 trap to debugger - we copy debugger traps (best effort) */
	{ ARG_B, 0, action_int },	/* 0xCD INT imm8 - interrupt vector */
	{ ARG_NONE, 0, action_warn },	/* 0xCE INTO trap to debugger */
	{ ARG_NONE, 0, action_warn },	/* 0xCF IRET interrupt return - we should not see this */

	{ OPCODE_RM, opcode_table_D0_rm, NO_ACTION_CALLED },	/* 0xD0  shift group 2 */
	{ OPCODE_RM, opcode_table_D1_rm, NO_ACTION_CALLED },	/* 0xD1  shift group 2 */
	{ OPCODE_RM, opcode_table_D2_rm, NO_ACTION_CALLED },	/* 0xD2  shift group 2 */
	{ OPCODE_RM, opcode_table_D3_rm, NO_ACTION_CALLED },	/* 0xD3  shift group 2 */
	{ ARG_B, 0, action_copy },	/* 0xD4 AAM, imm8 */
	{ ARG_B, 0, action_copy },	/* 0xD5 AAD, imm8 */
	{ ARG_NONE, 0, action_fail },	/* 0xD6  undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0xD7 XLAT %al = (%ds:%ebx[%al]) */

	{ FPU_ESC, opcode_table_D8_fpu, NO_ACTION_CALLED },	/* 0xD8  escape for floating point instructions */
	{ FPU_ESC, opcode_table_D9_fpu, NO_ACTION_CALLED },	/* 0xD9  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DA_fpu, NO_ACTION_CALLED },	/* 0xDA  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DB_fpu, NO_ACTION_CALLED },	/* 0xDB  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DC_fpu, NO_ACTION_CALLED },	/* 0xDC  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DD_fpu, NO_ACTION_CALLED },	/* 0xDD  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DE_fpu, NO_ACTION_CALLED },	/* 0xDE  escape for floating point instructions */
	{ FPU_ESC, opcode_table_DF_fpu, NO_ACTION_CALLED },	/* 0xDF  escape for floating point instructions */

	{ ARG_B, 0, action_copy },	/* 0xE0 LOOPNZ, rel8 */
	{ ARG_B, 0, action_copy },	/* 0xE1 LOOPZ, rel8 */
	{ ARG_B, 0, action_copy },	/* 0xE2 LOOP, rel8 */
	{ ARG_B, 0, action_jcc },	/* 0xE3 JCXZ, rel8 */
	{ ARG_B, 0, action_copy },	/* 0xE4 IN port[imm8] to %al */
	{ ARG_B, 0, action_copy },	/* 0xE5 IN port[imm8] to %eax */
	{ ARG_B, 0, action_copy },	/* 0xE6 OUT %al to port[imm8] */
	{ ARG_B, 0, action_copy },	/* 0xE7 OUT %eax to port[imm8] */

	{ ARG_D, 0, action_call },	/* 0xE8 CALL near relative */
	{ ARG_D, 0, action_jmp },	/* 0xE9 JMP near relative */
	{ ARG_P, 0, action_warn },	/* 0xEA JMP far absolute */
	{ ARG_B, 0, action_jmp },	/* 0xEB JMP short */
	{ ARG_NONE, 0, action_copy },	/* 0xEC IN port[%dx] to %al */
	{ ARG_NONE, 0, action_copy },	/* 0xED IN port[%dx] to %eax */
	{ ARG_NONE, 0, action_copy },	/* 0xEE OUT %al to port[%dx] */
	{ ARG_NONE, 0, action_copy },	/* 0xEF OUT %eax to port[%dx] */

	{ PREFIX_ESC, 0, NO_ACTION_CALLED },	/* 0xF0  lock prefix */
	{ ARG_NONE, 0, action_fail },	/* 0xF1  undefined opcode */
	{ PREFIX_ESC, opcode_table_F20F, NO_ACTION_CALLED },	/* 0xF2   repne prefix */
	{ PREFIX_ESC, opcode_table_F30F, NO_ACTION_CALLED },	/* 0xF3  rep prefix */
	{ ARG_NONE, 0, action_copy },	/* 0xF4 HLT */
	{ ARG_NONE, 0, action_copy },	/* 0xF5 CMC */
	{ OPCODE_RM, opcode_table_F6_rm, NO_ACTION_CALLED },	/* 0xF6  group 3 */
	{ OPCODE_RM, opcode_table_F7_rm, NO_ACTION_CALLED },	/* 0xF7  group 3 */

	{ ARG_NONE, 0, action_copy },	/* 0xF8 CLC */
	{ ARG_NONE, 0, action_copy },	/* 0xF9 STC */
	{ ARG_NONE, 0, action_copy },	/* 0xFA CLI */
	{ ARG_NONE, 0, action_copy },	/* 0xFB STI */
	{ ARG_NONE, 0, action_copy },	/* 0xFC CLD */
	{ ARG_NONE, 0, action_copy },	/* 0xFD STD */
	{ OPCODE_RM, opcode_table_FE_rm, NO_ACTION_CALLED },	/* 0xFE  group 4 */
	{ OPCODE_RM, opcode_table_FF_rm, NO_ACTION_CALLED }	/* 0xFF  group 5 */
};


#endif  /* FBT_OPCODE_TABLES_H */

