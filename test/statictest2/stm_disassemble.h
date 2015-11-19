
#ifndef STM_DISASSEMBLE_H
#define STM_DISASSEMBLE_H

#include <stdint.h>
#include "stm_actions.h"


/** no arguments to this instruction */
#define ARG_NONE 0x0

/*
 * code offset value or immediate operand flags
 */

/** 1 byte code offset value or immediate operand */
#define ARG_B 0x1

/** 2 byte code offset value or immediate operand */
#define ARG_W 0x2

/** 4 byte code offset value or immediate operand */
#define ARG_D 0x4

/** 6 byte code offset value or immediate operand */
#define ARG_P 0x6

/** 8 byte code offset value or immediate operand */
#define ARG_O 0x8

/** 10 byte code offset value or immediate operand */
#define ARG_T 0xA

/**
 * mask for testing operand bits.
 * As it is defined now, the length of the arguments in bytes is just opcode_flags & MASK_ARG.
 * This only works as long as we don't have, for example, a 4 byte and a 6 byte argument at 
 * the same time, or several arguments of the same length. If this ever changes, we will have 
 * to adjust the functions in stm_disassemble.c
 */
#define MASK_ARG 0x0F


/*
 * ModR/M flags
 */

/** a ModR/M byte follows the opcode */
#define ARG_RM 0x10

/** opcode extension in the three reg bits of the ModR/M byte */
#define OPCODE_RM 0x20

/** number and type of arguments depends on opcode extension in reg bits of ModR/M byte! */
#define ARG_DEP_RM 0x40



/*
 * other flags
 */
 
/** escape for multibyte opcodes */
#define OPCODE_ESC 0x80

/**
 * do not end TU after this instruction.
 * Use for instructions that set a condition code. Condition codes 
 * may not be preserved over TU boundaries.
 */
#define NO_SPLIT 0x100




/** entry for opcode table */
struct ia32_opcode {
	/** what kind of arguments the instruction has */
	uint16_t opcode_flags;
	
	/** offset in opcode extension table (only relevant if OPCODE_RM flag set) */
	struct ia32_opcode *opcode_table;
	
	/** pointer to the function that shall handle that instruction */
	enum finalize_tu (*opcode_handler)(int32_t, int32_t, int, int*);
	
};

int disasm_insn(unsigned char *insn_addr, enum finalize_tu (**action)(int32_t, int32_t, int, int*), struct ia32_opcode *opcode_table);

int disasm_oe(unsigned char modrm_byte, struct ia32_opcode *group, enum finalize_tu (**action)(int32_t, int32_t, int, int*), uint16_t opcode_flags);

int decode_modrm(unsigned char modrm_byte);

#endif /* STM_DISASSEMBLE_H */
