#include "stm_settings.h"
#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG  */

#include "stm_disassemble.h"
#include "stm_opcode_tables.h"

/* bit test masks for ModR/M byte */
#define MASK_MOD 0xC0
#define MASK_REG 0x38
#define MASK_RM  0x07


/* 
 * TODO: replace offset with real memory address --> allows us to handle prefixes, multi-byte opcodes, 
 * floating point opcodes, opcode extensions into ModR/M, etc, all with one mechanism
 */

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
int disasm_insn(unsigned char *insn_addr, enum finalize_tu (**action)(int32_t, int32_t, int, int*), struct ia32_opcode *opcode_table) 
{
	struct ia32_opcode *opcode = &opcode_table[*insn_addr];
	int length = 1;
	
	/* check for OPCODE_ESC */
	if ((opcode->opcode_flags & OPCODE_ESC) != 0) {
		/* this is a multi-byte opcode -> we have to perform a lookup in an additional table */
#ifdef DEBUG
		printf("we've got a multi-byte opcode, first byte is %#x\n", *insn_addr); /* debug */
#endif /* DEBUG */
		length += disasm_insn(insn_addr + 1, action, opcode->opcode_table);
	} else {
		/* check for OPCODE_RM */
		if ((opcode->opcode_flags & OPCODE_RM) != 0) {
			/* this opcode extends into the reg bits of the ModR/M byte */
			/* call disasm_oe */
			length += disasm_oe(*(insn_addr + 1), opcode->opcode_table, action, opcode->opcode_flags);
		} else {
			/* determine length of ModR/M construction if necessary */
			if ((opcode->opcode_flags & ARG_RM) != 0) {
				/* this instruction uses a ModR/M byte */
				length += decode_modrm(*(insn_addr + 1));
			}
			
			/* read info regarding arguments (immediates, code offset, ...) */
			length += MASK_ARG & opcode->opcode_flags;
			
			/* retrieve action function */
			*action = opcode->opcode_handler;
		}
	}
	return length;	
}



/**
 * reads the opcode extension in the reg bits of the ModR/M byte.
 * This function is required for instructions that use the reg bits (5, 4, 3) of the ModR/M byte 
 * as an extension of the opcode. For these instructions, this is the function that reads the 
 * pointer to the function that handles the instruction. Opcode flags ar ORed with the ones from 
 * the base opcode.
 * @param modrm_addr the address of the ModR/M byte
 * @param group address where the entries for this group are
 * @param action pointer to the function pointer used to handle the disassembled instruction
 * @param opcode_flag the flags of the base opcode
 * @return the length of the instruction in bytes (including addressing mode encoding bytes /SIB of ModR/M, but not including the opcode)
 */
int disasm_oe(unsigned char modrm_byte, struct ia32_opcode *group, enum finalize_tu (**action)(int32_t, int32_t, int, int*), uint16_t opcode_flags) 
{
	/* jump to correct offset in table according to reg bits of ModR/M byte (hence 0x38 mask MASK_REG) */
	struct ia32_opcode *opcode = group + (MASK_REG & modrm_byte) * sizeof(struct ia32_opcode);
	uint16_t flags = opcode_flags | opcode->opcode_flags;
	int length = MASK_ARG & flags;
	length += decode_modrm(modrm_byte);
	*action = opcode->opcode_handler;
	return length;
}

/**
 * calculates the length of ModR/M.
 * Reads the ModR/M byte and calculates the length of the ModR/M, SIB and displacement bytes/dwords, if they exist.
 * @param modrm_addr the address of the ModR/M byte
 * @return the calculated length
 */
int decode_modrm(unsigned char modrm_byte) 
{
	int length = 1;
	
	/* TODO: check if problem solved: stm_disassemble.c:87: warning: comparison is always false due to limited range of data type */
	if (MASK_MOD == (modrm_byte & MASK_MOD)) {
		/* argument is a register --> nothing after ModR/M byte */
		return length;
	}
	
	if (0x04 == (modrm_byte & MASK_RM)) {
		/* R/M bits are 100 --> we have a SIB byte */
		length++;
	}
	if (0x80 == (modrm_byte & MASK_MOD)) {
		/* Mod bits are 10 -> 32bit displacement */
		length += 4;
	} else if (0x40 == (modrm_byte & MASK_MOD)) {
		/* mod bits are 01 -> 8bit displacement */
		length++;
	} else if (0x05 == (modrm_byte & MASK_RM)) {
		/* mod bits are 00, R/M bits are 101 -> another case of 32bit displacement */
		length += 4;
	}
	return length;
}

