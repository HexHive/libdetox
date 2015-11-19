
#ifndef STM_OPCODE_TABLES_H
#define STM_OPCODE_TABLES_H


/*
 * special action functions:
 * action_none
 * action_warn
 * action_jcc
 * action_jmp
 * action_call
 * action_call_indirect
 * action_ret
 *
 */

/*
 * order of CC fields:
 * O NO B/C/NAE AE/NB/NC E/Z NE/NZ BE/NA A/NBE
 * S NS P/PE NP/PO L/NGE NL/GE LE/NG NLE/G
 */


/**
 * table for opcode extensions in ModR/M byte.
 * The opcodes extend into the reg bits (5, 4, 3) of the ModR/M byte.
 * Flags are ORed with the ones from one- or two-byte opcode, respectively.
 */
struct ia32_opcode table_opcode_rm[] = {
	/* offset 0: group 1 OPCODES 0x80 to 0x83 */
	{ ARG_NONE, 0, action_copy },	/* 000 ADD */
	{ ARG_NONE, 0, action_copy },	/* 001 OR */
	{ ARG_NONE, 0, action_copy },	/* 010 ADC */
	{ ARG_NONE, 0, action_copy },	/* 011 SBB */
	{ ARG_NONE, 0, action_copy },	/* 100 AND */
	{ ARG_NONE, 0, action_copy },	/* 101 SUB */
	{ ARG_NONE, 0, action_copy },	/* 110 XOR */
	{ NO_SPLIT, 0, action_copy },	/* 111 CMP */
	
	/* offset 1: group 2 OPCODES 0xD0 to 0xD3 and 0xC0, 0xC1 */
	{ ARG_NONE, 0, action_copy },	/* 000 ROL */
	{ ARG_NONE, 0, action_copy },	/* 001 ROR */
	{ ARG_NONE, 0, action_copy },	/* 010 RCL */
	{ ARG_NONE, 0, action_copy },	/* 011 RCR */
	{ ARG_NONE, 0, action_copy },	/* 100 SHL */
	{ ARG_NONE, 0, action_copy },	/* 101 SHR */
	{ ARG_NONE, 0, action_warn },	/* 110 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 111 SAR */
	
	
	/* offset 2: group 3 OPCODE 0xF6 */
	{ ARG_B, 0, action_copy },		/* 000 TEST */
	{ ARG_NONE, 0, action_warn },	/* 001 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 010 NOT */
	{ ARG_NONE, 0, action_copy },	/* 011 NEG */
	{ ARG_NONE, 0, action_copy },	/* 100 MUL */
	{ ARG_NONE, 0, action_copy },	/* 101 IMUL */
	{ ARG_NONE, 0, action_copy },	/* 110 DIV */
	{ ARG_NONE, 0, action_copy },	/* 111 IDIV */
	
	/* offset 3: group 3 OPCODE 0xF7 */
	{ ARG_D, 0, action_copy },		/* 000 TEST */
	{ ARG_NONE, 0, action_warn },	/* 001 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 010 NOT */
	{ ARG_NONE, 0, action_copy },	/* 011 NEG */
	{ ARG_NONE, 0, action_copy },	/* 100 MUL */
	{ ARG_NONE, 0, action_copy },	/* 101 IMUL */
	{ ARG_NONE, 0, action_copy },	/* 110 DIV */
	{ ARG_NONE, 0, action_copy },	/* 111 IDIV */
	
	
	/* offset 4: group 4 OPCODE 0xFE */
	{ ARG_NONE, 0, action_copy },	/* 000 INC */
	{ ARG_NONE, 0, action_copy },	/* 001 DEC */
	{ ARG_NONE, 0, action_warn },	/* 010 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 011 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 100 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 101 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 110 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 111 undefined opcode */
	
	/* offset 5: group 5 OPCODE 0xFF */
	{ ARG_NONE, 0, action_copy },	/* 000 INC */
	{ ARG_NONE, 0, action_copy },	/* 001 DEC */
	{ ARG_NONE, 0, action_call_indirect },	/* 010 near indirect CALL */
	{ ARG_NONE, 0, action_warn },	/* 011 far CALL */
	{ ARG_NONE, 0, action_jmp },	/* 100 near JMP */
	{ ARG_NONE, 0, action_warn },	/* 101 far JMP */
	{ ARG_NONE, 0, action_copy },	/* 110 PUSH */
	{ ARG_NONE, 0, action_warn },	/* 111 undefined opcode */
	
	/* offset 6: group 6 */
	{ ARG_NONE, 0, action_copy },	/* 000 SLDT */
	{ ARG_NONE, 0, action_copy },	/* 001 STR */
	{ ARG_NONE, 0, action_copy },	/* 010 LLDT */
	{ ARG_NONE, 0, action_copy },	/* 011 LTR */
	{ ARG_NONE, 0, action_copy },	/* 100 VERR */
	{ ARG_NONE, 0, action_copy },	/* 101 VERW */
	{ ARG_NONE, 0, action_warn },	/* 110 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 111 undefined opcode */
	
	/* offset 7: group 7 */
	{ ARG_NONE, 0, action_copy },	/* 000 SGDT */
	{ ARG_NONE, 0, action_copy },	/* 001 SIDT */
	{ ARG_NONE, 0, action_copy },	/* 010 LGDT */
	{ ARG_NONE, 0, action_copy },	/* 011 LIDT */
	{ ARG_NONE, 0, action_copy },	/* 100 SMSW */
	{ ARG_NONE, 0, action_warn },	/* 101 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 110 LMSW */
	{ ARG_NONE, 0, action_warn },	/* 111 undefined opcode */
	
	/* offset 8: group 8 */
	{ ARG_NONE, 0, action_warn },	/* 000 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 001 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 010 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 011 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 100 BT */
	{ ARG_NONE, 0, action_copy },	/* 101 BTS */
	{ ARG_NONE, 0, action_copy },	/* 110 BTR */
	{ ARG_NONE, 0, action_copy },	/* 111 BTC */
	
	
};


/** table for two-byte opcodes */
struct ia32_opcode table_twobyte[] = {
	
	/* begin group 6: SLDT, STR, LLDT, LTR, VERR, VERW */
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[48], action_none },	/* 0x0F00 group 6; TODO: all have to do with segment selector and the like, we should probably use action_warn */
	/* end group 6 */
	/* begin group 7: SGDT, SIDT, LGDT, LIDT, SMSW, INVLPG, VMCALL, VMLAUNCH, VMRESUME, VMXOFF, MONITOR, MWAIT, SWAPGS */
	{ ARG_DEP_RM | ARG_RM | OPCODE_RM, &table_opcode_rm[56], action_none },	/* 
 							 	 * 0x0F01 group 7; TODO: all are either privileged operations, or are only useful in 
 							 	 * operating system code, or are instructions for interaction with monitoring hardware.
 							 	 */
	/* end group 7 */
	{ ARG_RM, 0, action_copy },	/* 0x0F02 LAR */
	{ ARG_RM, 0, action_copy },	/* 0x0F03 LSL */
	{ ARG_NONE, 0, action_warn },	/* 0x0F04 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F05 undefined opcode (only available in 64bit mode: SYSCALL */
	{ ARG_NONE, 0, action_warn },	/* 0x0F06 CLTS (privileged instruction) */
	{ ARG_NONE, 0, action_warn },	/* 0x0F07 undefined opcode (only available in 64bit mode: SYSRET */
	{ ARG_NONE, 0, action_warn },	/* 0x0F08 INVD (privileged instruction) */
	{ ARG_NONE, 0, action_warn },	/* 0x0F09 WBINVD (privileged instruction) */
	{ ARG_NONE, 0, action_warn },	/* 0x0F0A undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F0B illegal opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F0C undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 0x0F0D two-byte NOP */
	{ ARG_NONE, 0, action_warn },	/* 0x0F0E undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F0F undefined opcode */


	{ ARG_RM, 0, action_warn },	/* 0x0F10 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F11 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F12 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F13 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F14 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F15 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F16 MMX move and unpack instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F17 MMX move and unpack instructions */
	/* begin group 16 not handled for now*/
	/*{ ARG_RM | OPCODE_RM, 0, action_copy },	/* 0x0F18 prefetch */
	{ ARG_RM, 0, action_warn },	/* 0x0F18 prefetch */
	/* end group 16 */
	{ ARG_NONE, 0, action_warn },	/* 0x0F19 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F1A undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F1B undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F1C undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F1D undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F1E undefined opcode */
	{ ARG_RM, 0, action_copy },	/* 0x0F1F two-byte NOP */
	
	
	{ ARG_RM | OPCODE_RM, 0, action_copy },	/* 0x0F20 MOV */
	{ ARG_RM | OPCODE_RM, 0, action_copy },	/* 0x0F21 MOV */
	{ ARG_RM | OPCODE_RM, 0, action_copy },	/* 0x0F22 MOV */
	{ ARG_RM | OPCODE_RM, 0, action_copy },	/* 0x0F23 MOV */
	{ ARG_NONE, 0, action_warn },	/* 0x0F24 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F25 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F26 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F27 undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0F28 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F29 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2A miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2B miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2C miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2D miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2E miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F2F miscellaneous MMX instructions */
	
	{ ARG_NONE, 0, action_copy },	/* 0x0F30 WRMSR */
	{ ARG_NONE, 0, action_copy },	/* 0x0F31 RDTSC */
	{ ARG_NONE, 0, action_copy },	/* 0x0F32 RDMSR */
	{ ARG_NONE, 0, action_copy },	/* 0x0F33 RDPMC */
	{ ARG_NONE, 0, action_copy },	/* 0x0F34 SYSENTER */
	{ ARG_NONE, 0, action_copy },	/* 0x0F35 SYSEXIT */
	{ ARG_NONE, 0, action_warn },	/* 0x0F36 undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0x0F37 GETSEC */
	{ ARG_NONE, 0, action_warn },	/* 0x0F38 TODO: escape for three byte opcodes */
	{ ARG_NONE, 0, action_warn },	/* 0x0F39 undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3A TODO: escape for three byte opcodes */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3B undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3C undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3D undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3E undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F3F undefined opcode */
	
	
	/* conditional moves */
	{ ARG_RM, 0, action_copy },	/* 0x0F40 CMOVO */
	{ ARG_RM, 0, action_copy },	/* 0x0F41 CMOVNO */
	{ ARG_RM, 0, action_copy },	/* 0x0F42 CMOVB / CMOVC /CMOVNAE */
	{ ARG_RM, 0, action_copy },	/* 0x0F43 CMOVAE / CMOVNV / CMOVNC */
	{ ARG_RM, 0, action_copy },	/* 0x0F44 CMOVE / CMOVZ */
	{ ARG_RM, 0, action_copy },	/* 0x0F45 CMOVNE / CMOVNZ */
	{ ARG_RM, 0, action_copy },	/* 0x0F46 CMOVBE / CMOVNA */
	{ ARG_RM, 0, action_copy },	/* 0x0F47 CMOVA / CMOVNBE */
	{ ARG_RM, 0, action_copy },	/* 0x0F48 CMOVS */
	{ ARG_RM, 0, action_copy },	/* 0x0F49 CMOVNS */
	{ ARG_RM, 0, action_copy },	/* 0x0F4A CMOVP / CMOVPE */
	{ ARG_RM, 0, action_copy },	/* 0x0F4B CMOVNP / CMOVPO */
	{ ARG_RM, 0, action_copy },	/* 0x0F4C CMOVL / CMOVNGE */
	{ ARG_RM, 0, action_copy },	/* 0x0F4D CMOVNL / CMOVGE */
	{ ARG_RM, 0, action_copy },	/* 0x0F4E CMOVLE / CMOVNG */
	{ ARG_RM, 0, action_copy },	/* 0x0F4F CMOVNLE / CMOVG */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0F50 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F51 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F52 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F53 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F54 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F55 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F56 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F57 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F58 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F59 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5A miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5B miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5C miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5D miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5E miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F5F miscellaneous MMX instructions */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0F60 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F61 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F62 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F63 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F64 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F65 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F66 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F67 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F68 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F69 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6A miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6B miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6C miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6D miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6E miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F6F miscellaneous MMX instructions */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0F70 miscellaneous MMX instructions */
	/* { ARG_RM | OPCODE_RM, 0, action_warn },	/* 0x0F71 group 12 */
	/* { ARG_RM | OPCODE_RM, 0, action_warn },	/* 0x0F72 group 13 */
	/* { ARG_RM | OPCODE_RM, 0, action_warn },	/* 0x0F73 group 14 */
	{ ARG_RM, 0, action_warn },	/* 0x0F71 group 12 */
	{ ARG_RM, 0, action_warn },	/* 0x0F72 group 13 */
	{ ARG_RM, 0, action_warn },	/* 0x0F73 group 14 */
	{ ARG_RM, 0, action_warn },	/* 0x0F74 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F75 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F76 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F77 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F78 VMREAD */
	{ ARG_RM, 0, action_warn },	/* 0x0F79 VMWRITE */
	{ ARG_NONE, 0, action_warn },	/* 0x0F7A undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* 0x0F7B undefined opcode */
	{ ARG_RM, 0, action_warn },	/* 0x0F7C miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F7D miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F7E miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0F7F miscellaneous MMX instructions */
	
	
	/* conditional near jumps */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F80	JO */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F81	JNO */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F82	JB / JNAE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F83	JNB / JAE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F84	JE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F85	JNE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F86	JNA / JBE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F87	JA / JNBE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F88	JS */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F89	JNS */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8A	JP / JPE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8B	JNP / JPO */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8C	JL / JNGE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8D	JNL / JGE */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8E	JLE / JNG */
	{ ARG_D, 0, action_jcc }, 	/* 0x0F8F	JNLE / JG */
	
	
	/* set byte on condition */
	{ ARG_RM, 0, action_copy },	/* 0x0F90 SETO */
	{ ARG_RM, 0, action_copy },	/* 0x0F91 SETNO */
	{ ARG_RM, 0, action_copy },	/* 0x0F92 SETB / SETC /SETNAE */
	{ ARG_RM, 0, action_copy },	/* 0x0F93 SETAE / SETNV / SETNC */
	{ ARG_RM, 0, action_copy },	/* 0x0F94 SETE / SETZ */
	{ ARG_RM, 0, action_copy },	/* 0x0F95 SETNE / SETNZ */
	{ ARG_RM, 0, action_copy },	/* 0x0F96 SETBE / SETNA */
	{ ARG_RM, 0, action_copy },	/* 0x0F97 SETA / SETNBE */
	{ ARG_RM, 0, action_copy },	/* 0x0F98 SETS */
	{ ARG_RM, 0, action_copy },	/* 0x0F99 SETNS */
	{ ARG_RM, 0, action_copy },	/* 0x0F9A SETP / SETPE */
	{ ARG_RM, 0, action_copy },	/* 0x0F9B SETNP / SETPO */
	{ ARG_RM, 0, action_copy },	/* 0x0F9C SETL / SETNGE */
	{ ARG_RM, 0, action_copy },	/* 0x0F9D SETNL / SETGE */
	{ ARG_RM, 0, action_copy },	/* 0x0F9E SETLE / SETNG */
	{ ARG_RM, 0, action_copy },	/* 0x0F9F SETNLE / SETG */
	
	
	{ ARG_NONE, 0, action_copy },	/* 0x0FA0 PUSH FS*/
	{ ARG_NONE, 0, action_copy },	/* 0x0FA1 POP FS*/
	{ ARG_NONE, 0, action_copy },	/* 0x0FA2 CPUID */
	{ ARG_RM, 0, action_copy },	/* 0x0FA3 BT */
	{ ARG_RM, 0, action_copy },	/* 0x0FA4 SHLD */
	{ ARG_RM, 0, action_copy },	/* 0x0FA5 SHLD */
	{ ARG_NONE, 0, action_warn },	/* undefined opcode */
	{ ARG_NONE, 0, action_warn },	/* undefined opcode */
	{ ARG_NONE, 0, action_copy },	/* 0x0FA8 PUSH GS*/
	{ ARG_NONE, 0, action_copy },	/* 0x0FA9 POP GS*/
	{ ARG_NONE, 0, action_copy },	/* 0X0FAA RSM */
	{ ARG_RM, 0, action_copy },	/* 0x0FAB BTS */
	{ ARG_RM, 0, action_copy },	/* 0x0FAC SHRD */
	{ ARG_RM, 0, action_copy },	/* 0x0FAD SHRD */
	{ ARG_NONE, 0, action_warn },	/* 0x0FAE group 15 */
	{ ARG_RM, 0, action_copy },	/* 0x0FAF IMUL */
	
	
	{ ARG_RM, 0, action_copy },	/* 0x0FB0 CMPXCHG */
	{ ARG_RM, 0, action_copy },	/* 0x0FB1 CMPXCHG */
	{ ARG_RM, 0, action_copy },	/* 0x0FB2 LSS */
	{ ARG_RM, 0, action_copy },	/* 0x0FB3 BTR */
	{ ARG_RM, 0, action_copy },	/* 0x0FB4 LFS */
	{ ARG_RM, 0, action_copy },	/* 0x0FB5 LGS */
	{ ARG_RM, 0, action_copy },	/* 0x0FB6 MOVZX */
	{ ARG_RM, 0, action_copy },	/* 0x0FB7 MOVZX */
	{ ARG_NONE, 0, action_warn },	/* 0x0FB8 JMPE */
	{ ARG_NONE, 0, action_warn },	/* 0x0FB9 invalid opcode */
	{ ARG_RM | OPCODE_RM | ARG_B, &table_opcode_rm[64], action_copy },	/* 0x0FBA group 8: BT, BTS, BTR, BTC */
	{ ARG_RM, 0, action_copy },	/* 0x0FBB BTC */
	{ ARG_RM, 0, action_copy },	/* Ox0FBC BSF */
	{ ARG_RM, 0, action_copy },	/* Ox0FBD BSR */
	{ ARG_RM, 0, action_copy },	/* Ox0FBE MOVSX */
	{ ARG_RM, 0, action_copy },	/* Ox0FBF MOVSX */
	
	/* after that, only MMX/SSE opcodes */

	{ ARG_RM, 0, action_warn },	/* 0x0FC0 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC1 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC2 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC3 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC4 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC5 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC6 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC7 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC8 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FC9 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCA miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCB miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCC miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCD miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCE miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FCF miscellaneous MMX instructions */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0FD0 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD1 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD2 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD3 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD4 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD5 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD6 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD7 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD8 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FD9 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDA miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDB miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDC miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDD miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDE miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FDF miscellaneous MMX instructions */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0FE0 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE1 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE2 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE3 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE4 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE5 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE6 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE7 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE8 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FE9 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FEA miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FEB miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FEC miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FED miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FEE miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FEF miscellaneous MMX instructions */
	
	
	{ ARG_RM, 0, action_warn },	/* 0x0FF0 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF1 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF2 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF3 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF4 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF5 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF6 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF7 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF8 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FF9 miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFA miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFB miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFC miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFD miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFE miscellaneous MMX instructions */
	{ ARG_RM, 0, action_warn },	/* 0x0FFF miscellaneous MMX instructions */
	

};





struct ia32_opcode table_onebyte[] = {
	{ ARG_RM, 0, action_copy }, 		/* 0x00	ADD */
 	{ ARG_RM, 0, action_copy }, 		/* 0x01	ADD */
 	{ ARG_RM, 0, action_copy }, 		/* 0x02	ADD */
 	{ ARG_RM, 0, action_copy }, 		/* 0x03	ADD */
 	{ ARG_B, 0, action_copy }, 		/* 0x04	ADD to AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x05	ADD to EAX */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x06	PUSH ES */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x07	POP ES */
 	{ ARG_RM, 0, action_copy }, 		/* 0x08 OR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x09 OR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x0A OR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x0B OR */
 	{ ARG_B, 0, action_copy }, 		/* 0x0C OR AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x0D OR EAX */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x0E	PUSH CS */
 	{ OPCODE_ESC, table_twobyte, action_none }, /* 0x0F	escape for two byte opcodes */
	
	
  	{ ARG_RM, 0, action_copy }, 		/* 0x10	ADC */
 	{ ARG_RM, 0, action_copy }, 		/* 0x11	ADC */
 	{ ARG_RM, 0, action_copy }, 		/* 0x12	ADC */
 	{ ARG_RM, 0, action_copy }, 		/* 0x13	ADC */
 	{ ARG_B, 0, action_copy }, 		/* 0x14	ADC to AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x15	ADC to EAX */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x16	PUSH SS */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x17	POP SS */
 	{ ARG_RM, 0, action_copy }, 		/* 0x18 SBB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x19 SBB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x1A SBB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x1B SBB */
 	{ ARG_B, 0, action_copy }, 		/* 0x1C SBB from AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x1D SBB from EAX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x1E	PUSH DS */
	{ ARG_NONE, 0, action_copy }, 		/* 0x1F	POP DS */
	
	
	{ ARG_RM, 0, action_copy }, 		/* 0x20	AND */
 	{ ARG_RM, 0, action_copy }, 		/* 0x21	AND */
 	{ ARG_RM, 0, action_copy }, 		/* 0x22	AND */
 	{ ARG_RM, 0, action_copy }, 		/* 0x23	AND */
 	{ ARG_B, 0, action_copy }, 		/* 0x24	AND AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x25	AND EAX */
 	{ OPCODE_ESC, 0, action_none }, 		/* 0x26	prefix ES segment override */
  	{ ARG_NONE, 0, action_copy }, 		/* 0x27	DAA */
  	{ ARG_RM, 0, action_copy }, 		/* 0x28 SUB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x29 SUB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x2A SUB */
 	{ ARG_RM, 0, action_copy }, 		/* 0x2B SUB */
 	{ ARG_B, 0, action_copy }, 		/* 0x2C SUB from AL*/
 	{ ARG_D, 0, action_copy }, 		/* 0x2D SUB from EAX*/
 	{ OPCODE_ESC, 0, action_none }, 		/* 0x2E	prefix CS segment override */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x2F	DAS */
	
 	
  	{ ARG_RM, 0, action_copy }, 		/* 0x30	XOR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x31	XOR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x32	XOR */
 	{ ARG_RM, 0, action_copy }, 		/* 0x33	XOR */
 	{ ARG_B, 0, action_copy }, 		/* 0x34	XOR AL */
 	{ ARG_D, 0, action_copy }, 		/* 0x35	XOR EAX */
 	{ OPCODE_ESC, 0, action_none }, 		/* 0x36	prefix SS segment override */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x37	AAA */
  	/* TODO: confirm we can just copy CMP */
  	{ ARG_RM | NO_SPLIT, 0, action_copy }, 		/* 0x38 CMP */
 	{ ARG_RM | NO_SPLIT, 0, action_copy }, 		/* 0x39 CMP */
 	{ ARG_RM | NO_SPLIT, 0, action_copy }, 		/* 0x3A CMP */
 	{ ARG_RM | NO_SPLIT, 0, action_copy }, 		/* 0x3B CMP */
 	{ ARG_B | NO_SPLIT, 0, action_copy }, 		/* 0x3C CMP with AL */
 	{ ARG_D | NO_SPLIT, 0, action_copy }, 		/* 0x3D CMP with EAX */
  	{ OPCODE_ESC, 0, action_none }, 		/* 0x3E	prefix DS segment override */
 	{ ARG_NONE, 0, action_copy }, 		/* 0x3F	AAS */
	
	
	{ ARG_NONE, 0, action_copy }, 		/* 0x40	INC EAX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x41	INC ECX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x42	INC EDX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x43	INC EBX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x44	INC ESP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x45	INC EBP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x46	INC ESI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x47	INC EDI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x48	DEC EAX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x49	DEC ECX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4A	DEC EDX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4B	DEC EBX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4C	DEC ESP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4D	DEC EBP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4E	DEC ESI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x4F	DEC EDI */
	
	
	{ ARG_NONE, 0, action_copy }, 		/* 0x50	PUSH EAX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x51	PUSH ECX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x52	PUSH EDX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x53	PUSH EBX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x54	PUSH ESP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x55	PUSH EBP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x56	PUSH ESI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x57	PUSH EDI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x58	POP EAX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x59	POP ECX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5A	POP EDX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5B	POP EBX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5C	POP ESP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5D	POP EBP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5E	POP ESI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x5F	POP EDI */
	
	
	{ ARG_NONE, 0, action_copy }, 		/* 0x60	PUSHAD */
	{ ARG_NONE, 0, action_copy }, 		/* 0x61	POPAD */
	{ ARG_RM, 0, action_copy }, 		/* 0x62	BOUND */
	{ ARG_RM, 0, action_copy }, 		/* 0X63 ARPL */
	{ OPCODE_ESC, table_onebyte, action_none }, 		/* 0x64	prefix FS segment override */
	{ OPCODE_ESC, table_onebyte, action_none }, 		/* 0x65	prefix GS segment override */
	{ OPCODE_ESC, table_onebyte, action_none }, 		/* 0x66	prefix operand size override */
	{ OPCODE_ESC, table_onebyte, action_none }, 		/* 0x67	prefix address size override */
	{ ARG_D, 0, action_copy }, 		/* 0x68	PUSH immediate operand (32bit); immediate missing in intel documentation */
	{ ARG_D | ARG_RM, 0, action_copy }, 	/* 0x69	IMUL (32bit immediate) */
	{ ARG_B, 0, action_copy }, 		/* 0x6A	PUSH immediate operand (8bit); immediate missing in intel documentation */
	{ ARG_B | ARG_RM, 0, action_copy }, 	/* 0x6B	IMUL (8bit immediate) */
	{ ARG_NONE, 0, action_copy }, 		/* 0x6C	INS */
	{ ARG_NONE, 0, action_copy }, 		/* 0x6D	INS */
	{ ARG_NONE, 0, action_copy }, 		/* 0x6E	OUTS */
	{ ARG_NONE, 0, action_copy }, 		/* 0x6F	OUTS */
	
	
 	/* conditional short (relative) jumps */
	{ ARG_B, 0, action_jcc }, 		/* 0x70	JO */
	{ ARG_B, 0, action_jcc }, 		/* 0x71	JNO */
	{ ARG_B, 0, action_jcc }, 		/* 0x72	JB / JNAE */
	{ ARG_B, 0, action_jcc }, 		/* 0x73	JNB / JAE */
	{ ARG_B, 0, action_jcc }, 		/* 0x74	JE */
	{ ARG_B, 0, action_jcc }, 		/* 0x75	JNE */
	{ ARG_B, 0, action_jcc }, 		/* 0x76	JNA / JBE */
	{ ARG_B, 0, action_jcc }, 		/* 0x77	JA / JNBE */
	{ ARG_B, 0, action_jcc }, 		/* 0x78	JS */
	{ ARG_B, 0, action_jcc }, 		/* 0x79	JNS */
	{ ARG_B, 0, action_jcc }, 		/* 0x7A	JP / JPE */
	{ ARG_B, 0, action_jcc }, 		/* 0x7B	JNP / JPO */
	{ ARG_B, 0, action_jcc }, 		/* 0x7C	JL / JNGE */
	{ ARG_B, 0, action_jcc }, 		/* 0x7D	JNL / JGE */
	{ ARG_B, 0, action_jcc }, 		/* 0x7E	JLE / JNG */
	{ ARG_B, 0, action_jcc }, 		/* 0x7F	JNLE / JG */
	
	
	/* immediate group 1: ADD, OR, ADC, SBB, AND, SUB, XOR, CMP */
	{ ARG_B | ARG_RM | OPCODE_RM, table_opcode_rm, action_none }, 	/* 0x80	immediate group 1 */
	{ ARG_D | ARG_RM | OPCODE_RM, table_opcode_rm, action_none }, 	/* 0x81	immediate group 1 (32bit immediate) */
 	{ ARG_B | ARG_RM | OPCODE_RM, table_opcode_rm, action_none }, 	/* 0x82	immediate group 1 TODO: opcode 0x82 not found in instructions */
	{ ARG_B | ARG_RM | OPCODE_RM, table_opcode_rm, action_none }, 	/* 0x83	immediate group 1 */
	/* end immediate group 1 */
 	{ ARG_RM, 0, action_copy }, 		/* 0x84	TEST (8bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x85	TEST (32bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x86	XCHG (8bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x87	XCHG (32bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x88	MOV (8bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x89	MOV (32bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x8A	MOV (8bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x8B	MOV (32bit) */
	{ ARG_RM, 0, action_copy }, 		/* 0x8C	MOV from segment register */
	{ ARG_RM, 0, action_copy }, 		/* 0x8D LEA */
	{ ARG_RM, 0, action_copy }, 		/* 0x8E	MOV to segment register */
	/* group 1A */
	/*{ ARG_RM | OPCODE_RM, 0, action_none }, 0x8F group 1A, at the moment only POP, reg=000 */
	{ ARG_RM, 0, action_copy }, 		/* 0x8F POP (POP in i386 manual, 1-element group 1A in recent manuals) */
	/* end group 1A */

	
	{ ARG_NONE, 0, action_copy }, 		/* 0x90 NOP, actually XCHG EAX, EAX; with prefix 0xF3 PAUSE*/
	{ ARG_NONE, 0, action_copy }, 		/* 0x91 XCHG EAX, ECX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x92 XCHG EAX, EDX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x93 XCHG EAX, EBX */
	{ ARG_NONE, 0, action_copy }, 		/* 0x94 XCHG EAX, ESP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x95 XCHG EAX, EBP */
	{ ARG_NONE, 0, action_copy }, 		/* 0x96 XCHG EAX, ESI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x97 XCHG EAX, EDI */
	{ ARG_NONE, 0, action_copy }, 		/* 0x98 CBWE */
	{ ARG_NONE, 0, action_copy }, 		/* 0x99 CDQ */
	{ ARG_P, 0, action_warn }, 		/* 0x9A CALL (far call, we don't handle that) */
	{ ARG_NONE, 0, action_copy }, 		/* 0x9B WAIT / FWAIT */
	{ ARG_NONE, 0, action_copy }, 		/* 0x9C PUSHFD */
	{ ARG_NONE, 0, action_copy }, 		/* 0x9D POPFD */
	{ ARG_NONE, 0, action_copy }, 		/* 0x9E SAHF */
	{ ARG_NONE, 0, action_copy }, 		/* 0x9F LAHF */
	
	
	{ ARG_B, 0, action_copy }, 		/* 0xA0 MOV from moffs8 */
	{ ARG_D, 0, action_copy }, 		/* 0xA1 MOV from moffs32 */
	{ ARG_B, 0, action_copy }, 		/* 0xA2 MOV to moffs8 */
	{ ARG_D, 0, action_copy }, 		/* 0xA3 MOV to moffs32 */
	{ ARG_NONE, 0, action_copy }, 		/* 0xA4 MOVSB */
	{ ARG_NONE, 0, action_copy }, 		/* 0xA5 MOVSD */
	{ ARG_NONE, 0, action_copy }, 		/* 0xA6 CMPSB */
	{ ARG_NONE, 0, action_copy }, 		/* 0xA7 CMPSD */
	{ ARG_B, 0, action_copy }, 		/* 0xA8 TEST */
	{ ARG_D, 0, action_copy }, 		/* 0xA9 TEST */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAA STOSB */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAB STOSD */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAC LODSB */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAD LODSD */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAE SCASB */
	{ ARG_NONE, 0, action_copy }, 		/* 0xAF SCASD */
	
	
	{ ARG_B, 0, action_copy }, 		/* 0xB0 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB1 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB2 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB3 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB4 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB5 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB6 MOV imm8 to r8 */
	{ ARG_B, 0, action_copy }, 		/* 0xB7 MOV imm8 to r8 */
	{ ARG_D, 0, action_copy }, 		/* 0xB8 MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xB9 MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBA MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBB MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBC MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBD MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBE MOV imm32 to r32 */
	{ ARG_D, 0, action_copy }, 		/* 0xBF MOV imm32 to r32 */
	
	
	/* shift group 2 fragment 1: ROL, ROR, RCR, RCL, SHL, SHR, SAL, SAR */
	{ ARG_B | ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_none }, 		/* 0xC0 shift group 2 */
	{ ARG_B | ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_none }, 		/* 0xC1 shift group 2 */
	/* end shift group 2 fragment 1 */
	{ ARG_W, 0, action_ret }, 		/* 0xC2 RET near return, pop imm16 bytes from stack */
	{ ARG_NONE, 0, action_ret }, 		/* 0xC3 RET near return */
	{ ARG_RM, 0, action_copy }, 		/* 0xC4 LES TODO: copy or warn? */
	{ ARG_RM, 0, action_copy }, 		/* 0xC5 LDS TODO: copy or warn? */
	/* group 11 MOV */
	/* { ARG_B | ARG_RM | OPCODE_RM, 0, action_copy }, 		 0xC6 MOV */
	/* { ARG_D | ARG_RM | OPCODE_RM, 0, action_copy }, 		 0xC7 MOV */
	{ ARG_B | ARG_RM, 0, action_copy }, 		 /* 0xC6 MOV */
	{ ARG_D | ARG_RM, 0, action_copy }, 		 /* 0xC7 MOV */
	/* end group 11 MOV */
	{ ARG_W | ARG_B, 0, action_copy }, 	/* 0xC8 ENTER TODO: how to handle */
	{ ARG_NONE, 0, action_copy }, 		/* 0xC9 LEAVE */
	{ ARG_W, 0, action_warn }, 		/* 0xCA RET far return TODO: how to handle */
	{ ARG_NONE, 0, action_warn }, 		/* 0xCB RET far return TODO: how to handle */
	{ ARG_NONE, 0, action_copy }, 		/* 0xCC INT 3; trap to debugger TODO: how to handle */
	{ ARG_B, 0, action_copy }, 		/* 0xCD INT imm8; interrupt vector imm8 TODO: how to handle */
	{ ARG_NONE, 0, action_copy }, 		/* 0xCE INTO 3; trap to debugger TODO: how to handle */
	{ ARG_NONE, 0, action_copy }, 		/* 0xCF IRET; interrupt return TODO: how to handle */
	
	
	/* shift group 2 fragment 2 */
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_copy }, 	/* 0xD0 shift group 2*/
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_copy }, 	/* 0xD1 shift group 2*/
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_copy }, 	/* 0xD2 shift group 2*/
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[8], action_copy }, 	/* 0xD3 shift group 2*/
	/* end shift group 2 fragment 2 */
	{ ARG_B, 0, action_copy }, 		/* 0xD4 AAM */
	{ ARG_B, 0, action_copy }, 		/* 0xD5 AAD */
	{ ARG_NONE, 0, action_warn }, 		/* 0xD6 undefined opcode */
	{ ARG_NONE, 0, action_copy }, 		/* 0xD7 XLATB */
	/* TODO: escapes to coprocessor instruction set */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xD8 ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xD9 ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDA ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDB ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDC ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDD ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDE ESC */
	{ ARG_NONE | OPCODE_ESC, 0, action_none }, 	/* 0xDF ESC */
	/* end escapes to coprocessor instruction set */
	
	
	{ ARG_B, 0, action_copy }, 		/* 0xE0 LOOPNE */
	{ ARG_B, 0, action_copy }, 		/* 0xE1 LOOPE */
	{ ARG_B, 0, action_copy }, 		/* 0xE2 LOOP */
	{ ARG_B, 0, action_jcc }, 		/* 0xE3 JCXZ / JECXZ / JRCXZ */
	{ ARG_B, 0, action_copy }, 		/* 0xE4 IN */
	{ ARG_B, 0, action_copy }, 		/* 0xE5 IN */
	{ ARG_B, 0, action_copy }, 		/* 0xE6 OUT */
	{ ARG_B, 0, action_copy }, 		/* 0xE7 OUT */
	{ ARG_D, 0, action_call }, 		/* 0xE8 CALL near relative */
	{ ARG_D, 0, action_jmp }, 		/* 0xE9 JMP near relative */
	{ ARG_P, 0, action_warn }, 		/* 0xEA JMP far absolute */
	{ ARG_B, 0, action_jmp }, 		/* 0xEB JMP short */
	{ ARG_NONE, 0, action_copy }, 		/* 0xEC IN */
	{ ARG_NONE, 0, action_copy }, 		/* 0xED IN */
	{ ARG_NONE, 0, action_copy }, 		/* 0xEE OUT */
	{ ARG_NONE, 0, action_copy }, 		/* 0xEF OUT */
	
	
	{ OPCODE_ESC, 0, action_none }, 	/* 0xF0 LOCK prefix */
	{ ARG_NONE, 0, action_warn }, 		/* 0xF1 undefined opcode */
	{ OPCODE_ESC, 0, action_none }, 		/* 0xF2 REPNE repeat string operation prefix */
	{ OPCODE_ESC, 0, action_none }, 		/* 0xF3 REP / REPE repeat string operation prefix */
	{ ARG_NONE, 0, action_copy }, 		/* 0xF4 HLT */
	{ ARG_NONE, 0, action_copy }, 		/* 0xF5 CMC */
	/* begin unary group 3: TEST, NOT, NEG, MUL, IMUL, DIV, IDIV; ARGH! Argument count and type depends on opcode extension!! */
	{ ARG_DEP_RM | ARG_RM | OPCODE_RM, &table_opcode_rm[16], action_none }, 	/* 0xF6 unary group 3; arguments depend on opcode extension */
	{ ARG_DEP_RM | ARG_RM | OPCODE_RM, &table_opcode_rm[24], action_none }, 	/* 0xF7 unary group 3; arguments depend on opcode extension */
	/* end unary group 3 */
	{ ARG_NONE, 0, action_copy }, 		/* 0xF8 CLC */
	{ ARG_NONE, 0, action_copy }, 		/* 0xF9 STC */
	{ ARG_NONE, 0, action_copy }, 		/* 0xFA CLI */
	{ ARG_NONE, 0, action_copy }, 		/* 0xFB STI */
	{ ARG_NONE, 0, action_copy }, 		/* 0xFC CLD */
	{ ARG_NONE, 0, action_copy }, 		/* 0xFD STD */
	/* begin INC / DEC group 4 */
	{ ARG_RM | OPCODE_RM, &table_opcode_rm[32], action_none }, 	/* 0xFE inc / dec group 4 */
	/* end INC / DEC group 4 */
	/* begin INC / DEC group 5 */
	{ ARG_DEP_RM | ARG_RM | OPCODE_RM, &table_opcode_rm[40], action_none }, 	/* 0xFF inc / dec group 5, also near and far call/jmp! arguments depend on opcode extension */
	/* end INC / DEC group 5 */
	
};



#endif /* STM_OPCODE_TABLES_H */
