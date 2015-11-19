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

/* REX bits */
#define REXW 0x8

/**
 * ModR/M flags
 */
/** a ModR/M byte follows the opcode */
#define HAS_MODRM 0x10
#define NO_MODRM 0x00

/* ModR/M fields */
#define MODRM_MOD(x) ((x>>6)&0x3)
#define MODRM_REG(x) ((x>>3)&0x7)
#define MODRM_RM(x) (x&0x7)
/* sib fields */
#define SIB_SCALE(x) MODRM_MOD(x)
#define SIB_INDEX(x) MODRM_REG(x)
#define SIB_BASE(x) MODRM_RM(x)

/**
 * escape flags
 */
/** escape for prefix */
#define OPCODE_PREFIX_ESC 0x01
/** escape for multibyte opcodes */
#define OPCODE_ESC 0x02
/** escape for fpu opcodes */
#define OPCODE_FPU_ESC 0x04
/** opcode extension in the three reg bits of the ModR/M byte */
#define OPCODE_MODRM_ESC 0x08
#define OPCODETABLE(x) (x&0xF) /* we escape into a table */

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

/*******************************************************\
 * operand flags                                       *
\*******************************************************/
/* operand not used by this instruction */
#define		NONE				0x00000000
#define		ARG_NONE			0x00000000

/* operand masks */
#define		OP_USE_MASK			0x0000000F		/* how is this operand used */
#define		OP_SPECIAL_MASK			0x000000F0		/* special flags for this operand */
#define		OP_ADDRM_MASK			0x00001F00		/* addressing method */
#define		OPT_MASK			0x00FF0000		/* type of operand */



/* how is the opeand used in this instruction */
#define		RWX				0x00000007
#define		READ				0x00000001		/* operand is read */
#define		WRITE				0x00000002		/* operand is written */
#define		MODIFY				READ | WRITE	/* operand is modified */
#define		EXECUTE				0x00000004		/* operand is executed */

/* special operands and flags */
#define		IMPL				0x00000010		/* implizit register */
#define		CONST				0x00000020		/* implizit constant value */
#define		SIGEND				0x00000040		/* operand is signed */

/* addresing methods - taken from intel manual */
#define		ADDRM_A				0x00000100
#define		ADDRM_C				0x00000200
#define		ADDRM_D				0x00000300
#define		ADDRM_E				0x00000400
#define		ADDRM_F				0x00000500
#define		ADDRM_G				0x00000600
#define		ADDRM_I				0x00000700
#define		ADDRM_J				0x00000800
#define		ADDRM_M				0x00000900
#define		ADDRM_N				0x00000A00
#define		ADDRM_O				0x00000B00
#define		ADDRM_P				0x00000C00
#define		ADDRM_Q				0x00000D00
#define		ADDRM_R				0x00000E00
#define		ADDRM_S				0x00000F00
#define		ADDRM_U				0x00001000
#define		ADDRM_V				0x00001100
#define		ADDRM_W				0x00001200
#define		ADDRM_X				0x00001300
#define		ADDRM_Y				0x00001400

#define hasModRMOp(x) (((x&OP_ADDRM_MASK)==ADDRM_C)||((x&OP_ADDRM_MASK)==ADDRM_D)||((x&OP_ADDRM_MASK)==ADDRM_E)||((x&OP_ADDRM_MASK)==ADDRM_G)||((x&OP_ADDRM_MASK)==ADDRM_M)||((x&OP_ADDRM_MASK)==ADDRM_N)||((x&OP_ADDRM_MASK)==ADDRM_P)||((x&OP_ADDRM_MASK)==ADDRM_Q)||((x&OP_ADDRM_MASK)==ADDRM_R)||((x&OP_ADDRM_MASK)==ADDRM_S)||((x&OP_ADDRM_MASK)==ADDRM_U)||((x&OP_ADDRM_MASK)==ADDRM_V)||((x&OP_ADDRM_MASK)==ADDRM_W))
#define anyModRMOp(x) (hasModRMOp(x->destFlags)||hasModRMOp(x->srcFlags)||hasModRMOp(x->auxFlags))

#define ModRMparseRM(x) (((x&OP_ADDRM_MASK)==ADDRM_E)||((x&OP_ADDRM_MASK)==ADDRM_M)||((x&OP_ADDRM_MASK)==ADDRM_Q)||((x&OP_ADDRM_MASK)==ADDRM_R)||((x&OP_ADDRM_MASK)==ADDRM_U)||((x&OP_ADDRM_MASK)==ADDRM_W))
#define ModRMparseREG(x) (((x&OP_ADDRM_MASK)==ADDRM_G)||((x&OP_ADDRM_MASK)==ADDRM_N)||((x&OP_ADDRM_MASK)==ADDRM_P)||((x&OP_ADDRM_MASK)==ADDRM_S)||((x&OP_ADDRM_MASK)==ADDRM_V)||((x&OP_ADDRM_MASK)==ADDRM_C)||((x&OP_ADDRM_MASK)==ADDRM_D))

#define hasImmOp(x) (((x&OP_ADDRM_MASK)==ADDRM_A)||((x&OP_ADDRM_MASK)==ADDRM_I)||((x&OP_ADDRM_MASK)==ADDRM_J)||((x&OP_ADDRM_MASK)==ADDRM_O))
#define anyImmOp(x) (hasModRMOp(x->destFlags)||hasModRMOp(x->srcFlags)||hasModRMOp(x->auxFlags))

#define immIsPtr(x) (hasImmOp(x) && ((x&OP_ADDRM_MASK)!=ADDRM_I)) /* address or offset */
#define hasMemOp(x) (((x&OP_ADDRM_MASK)==ADDRM_A)||((x&OP_ADDRM_MASK)==ADDRM_E)||((x&OP_ADDRM_MASK)==ADDRM_J)||((x&OP_ADDRM_MASK)==ADDRM_M)||((x&OP_ADDRM_MASK)==ADDRM_O)||((x&OP_ADDRM_MASK)==ADDRM_Q)||((x&OP_ADDRM_MASK)==ADDRM_W)||((x&OP_ADDRM_MASK)==ADDRM_Y))

/* operand types - taken from intel manual */
#define		OPT_a				0x00010000
#define		OPT_b				0x00020000
#define		OPT_c				0x00030000
#define		OPT_d				0x00040000
#define		OPT_dq				0x00050000
#define		OPT_p				0x00060000
#define		OPT_pd				0x00070000
#define		OPT_pi				0x00080000
#define		OPT_ps				0x00090000
#define		OPT_q				0x000A0000
#define		OPT_s				0x000B0000
#define		OPT_ss				0x000C0000
#define		OPT_sd				0x000D0000 /* this one is not in the intel manual! */
#define		OPT_si				0x000E0000
#define		OPT_v				0x000F0000
#define		OPT_w				0x00100000
#define		OPT_z				0x00110000
/* operand types for FPU - these are not found in the intel manual */
#define		OPT_fs				0x00200000		/* single-real*/
#define		OPT_fd				0x00210000		/* double-real */
#define		OPT_fe				0x00220000		/* extended-real */
#define		OPT_fp				0x00230000		/* packed-BCD */
#define		OPT_fv				0x00240000		/* FPU env (14/28 bytes) */
#define		OPT_fst				0x00250000		/* FPU state (98/108 bytes) */

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
    tu_close_glue       /* TU has to be closed after this instruction, but fixup code must be inserted as when instr nr limit is reached */
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
    const struct ia32_opcode* cur_instr_info;  /* information about the current instruction */
    unsigned char  *first_byte_after_opcode;   /* Pointer to the first byte after the current intruction */
    unsigned int    num_prefixes;              /* Number of prefixes of this instruction */
    unsigned char  *next_instr;                /* Pointer to the first byte of the next instruction */
    unsigned char  *transl_instr;              /* Pointer to the translated code */
    actionFunP_t    action;                    /* Action to be
							called in order to translate the current instruction */
    thread_local_data_t  *tld;                 /* thread local data */
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
    /* the following hold flags for the operands */
    /* the value NONE means the opcode does not make use of this operand */
    const unsigned int destFlags;
    const unsigned int srcFlags;
    const unsigned int auxFlags;

    /*implicit flags */
    const unsigned char implDestFlags;
    const unsigned char implSrcFlags;
    const unsigned char implAuxFlags;

    /* table or function ptr */
    const unsigned char tableFlags;

    union {
	/** offset in opcode extension table (only relevant if OPCODE_RM flag set) */
	const struct ia32_opcode* const table;
	/** pointer to the function that shall handle that instruction */
	const actionFunP_t handler;
    } opcode;

    //#ifdef DEBUG
    const char* mnemonic;
    //#endif
    
};
typedef struct ia32_opcode ia32_opcode_t;


#endif
