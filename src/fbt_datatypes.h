/**
 * This file defines all datatypes and macros which are public.
 * These can and should be used by a user program.
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
#ifndef FBT_OPCODE_H
#define FBT_OPCODE_H

#include <stdint.h>

/** no arguments to this instruction */
#define ARG_NONE 0x0

/** 1 byte code offset value or immediate operand */
#define ARG_B 0x1

/** 2 byte code offset value or immediate operand */
#define ARG_W 0x2

/** 4 byte code offset value or immediate operand */
#define ARG_D 0x4

/** 6 byte code offset value or immediate operand */
#define ARG_P 0x6

/** 8 byte code offset value or immediate operand */
#define ARG_Q 0x8

/** 10 byte code offset value or immediate operand */
#define ARG_S 0xA

/**
 * ModR/M flags
 */

/** a ModR/M byte follows the opcode */
#define ARG_RM 0x10

/** opcode extension in the three reg bits of the ModR/M byte */
#define OPCODE_RM 0x20


/**
 * escape flags
 */

/** escape for prefix */
#define PREFIX_ESC 0x100

/** escape for multibyte opcodes */
#define OPCODE_ESC 0x200

/** escape for fpu opcodes */
#define FPU_ESC 0x400

/**
 * do not end TU after this instruction.
 * Use for instructions that set a condition code. Condition codes
 * may not be preserved over TU boundaries.
 */
#define NO_SPLIT 0x1000

/** this is a special action that is not called from within the disasm_insn-function */
#define NO_ACTION_CALLED 0

/* special prefixes */
#define PREFIX_LOCK       0xf0
#define PREFIX_REPNEZ     0xf2
#define PREFIX_MISC       0xf3
#define PREFIX_CS_SEG_OVR 0x2e
#define PREFIX_SS_SEG_OVR 0x36
#define PREFIX_DS_SEG_OVR 0x3e
#define PREFIX_ES_SEG_OVR 0x26
#define PREFIX_FS_SEG_OVR 0x64
#define PREFIX_GS_SEG_OVR 0x65
#define PREFIX_OP_SZ_OVR  0x66
#define PREFIX_ADDR_SZ_OVR  0x67
#define PREFIX_SIMD_EXT   0x0f

/* does the given opcode use a prefix? */
#define HAS_PREFIX(opc_byte) \
    ((opc_byte==PREFIX_LOCK) \
  || (opc_byte==PREFIX_REPNEZ) \
  || (opc_byte==PREFIX_CS_SEG_OVR) \
  || (opc_byte==PREFIX_SS_SEG_OVR) \
  || (opc_byte==PREFIX_DS_SEG_OVR) \
  || (opc_byte==PREFIX_ES_SEG_OVR) \
  || (opc_byte==PREFIX_FS_SEG_OVR) \
  || (opc_byte==PREFIX_GS_SEG_OVR) \
  || (opc_byte==PREFIX_OP_SZ_OVR) \
  || (opc_byte==PREFIX_ADDR_SZ_OVR) \
  /*|| (opc_byte==PREFIX_SIMD_EXT)*/)

/**
 * This enum characterises the state of the translation unit
 * After each instruction it is checked if we have to close the translation unit,
 * if we can continue translating until we reach the instruction number limit, or if
 * we have to continue translating even if the instruction number limit is reached.
 */
enum finalize_tu {
    tu_open,		/* TU must not be closed after this instruction, even if the instruction number limit is reached */
    tu_neutral,		/* TU can be closed after this instruction, if the instruction number limit is reached */
    tu_close,		/* TU has to be closed after this instruction in any case */
    tu_close_glue           /* TU has to be closed after this instruction, but fixup code must be inserted as when instr nr limit is reached */
};
typedef enum finalize_tu finalize_tu_t;

/* internal data structure which is passed to some functions */
typedef struct thread_local_data thread_local_data_t;
/* ahead definition which is needed because of circular dependencies */
typedef struct translate_struct translate_struct_t;

/* function pointer to an action (used in fbt_disassemble and fbt_actions) */
typedef finalize_tu_t (*actionFunP_t)(translate_struct_t* ts);

/** * This struct is handed to each action - containing all needed
    parameters * WARNING: code in fbt_asm_function.S depends on the layout
    of this structure!  */
struct translate_struct {
    unsigned char  *cur_instr;                 /* Pointer to the first byte of the current instruction */
    unsigned char  *first_byte_after_opcode;   /* Pointer to the first byte after the current intruction */
    unsigned int    num_prefixes;              /* Number of prefixes of this instruction */
    unsigned char  *next_instr;                /* Pointer to the first byte of the next instruction */
    unsigned char  *transl_instr;              /* Pointer to the translated code */
    actionFunP_t    action;                    /* Action to be
							called in order to translate the current instruction */
    thread_local_data_t  *tld;                       /* thread local data
						      */
    void           *app_data;                  /* Pointer to the application specific data */
#if defined(FBT_INLINE_CALLS)
    struct inline_struct *inlined_frames;    
#endif
};

#if defined(FBT_INLINE_CALLS)
/* linear list for static just-in-time inlining of recursive calls */
struct inline_struct {
    void *ret_addr;
    struct inline_struct *next;
};
#endif
	     /** entry for opcode table */
	     struct ia32_opcode {
		 /** what kind of arguments the instruction has */
		 uint32_t opcode_flags;

		 /** offset in opcode extension table (only relevant if OPCODE_RM flag set) */
		 struct ia32_opcode *opcode_table;

		 /** pointer to the function that shall handle that instruction */
		 actionFunP_t opcode_handler;

	     };
typedef struct ia32_opcode ia32_opcode_t;


#endif
