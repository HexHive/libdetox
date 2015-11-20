/**
 * @file fbt_lmem.h
 * Datatypes used for communication with selDebug
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Enrico Kravina <enrico.kravina@gmail.com>
 *
 * $Date: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedDate: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1184 $
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

#include "fbt_lmem_instruction.h"
#include "fbt_lmem.h"
#include "fbt_lmem_util.h"
#include "fbt_lmem_api.h"
#include "fbt_lmem_instruction.h"

#include "fbt_translate.h"
#include "fbt_actions.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_x86_opcode.h"

#include <assert.h>
#include <unistd.h> // for size_t
#include <asm-generic/mman.h>


#ifdef DEBUG
extern int debugStream;
#endif


/**
 * Implementation #defines
 */
#define		REG_IDX_MASK		0x0000000F
#define		REG_TYPE_MASK		0x000000F0

unsigned compute_flags_info(struct translate* tr){

	// These instructions are useful because they set _all_ the
	// six arithmetic flags. This means that
	static char* ins_chg_all[] = {
			"add", "or", "adc", "sbb", "and", "daa", "sub",
			"das", "xor", "aaa", "cmp", "aas", "imul", "test",
			"sahf", "cmps", "cmpsb", "cmpsw", "cmpsd", "scas",
			"scasb", "scasw", "scasd", "rol", "ror", "rcl", "rcr",
			"shl", "sal", "sar", "shr", "aam", "amx", "aad", "adx",
			"ng", "div", "idiv", "vmread", "vmwrite", "bt", "shld",
			"shrd", "cmpxchg", "btr", "popcnt", "bt", "bts", "btr",
			"btc", "bsf", "bsr", "xadd", "vmptrld", "vmclear", "vmxon",
			"vmptrst", "vmcall", "vmlaunch", "vmresume", "vmxoff",
			NULL
	};

	static char* change_some[] = {
			"bound", "arpl", "sahf", "fucomip", "fcomip", "fucomi", "fcomi",
			"icebp", "cmc", "clc", "stc",
			"inc", "dec", "verr", "verw", "lar", "lsl", "ucomisd",
			"comiss", "comisd", "sysenter", "cmpxchg8b",
			NULL
	};

	// These instructions use the arithmetic flags, before using
	// them we must make sure that the flags register contains the
	// correct arithmetic flags
	static char* ins_uses_fl[] = {
			"adc", "sbb", "daa", "das", "aaa", "aas",
			"jo", "jno", "jb",
			"jnae", "jc", "jnb", "jae", "jnc", "jz", "je", "jnz", "jne",
			"jbe", "jna", "jnbe", "ja", "js", "jns", "jp", "jpe", "jnp",
			"jpo", "jl", "jnge", "jnl", "jge", "jle", "jng", "jnle", "jg",
			"lahf", "rcl", "rcr", "into", "salc", "setalc","fcmovb", "fcmove",
			"fcmovbe", "fcmovu", "fcmovne", "fcmovnbe", "fcmovnu", "loopnz",
			"repnz", "repz", "repe", "cmc", "cmovo", "cmovno", "cmovb", "cmovnae",
			"cmovc", "cmovz", "cmove", "cmovnz", "cmovne", "cmovbe", "cmovna",
			"cmovnbe", "cmova", "cmovs", "cmovns", "cmovp", "cmovpe", "cmovnp",
			"cmovpo", "cmovl", "cmovnge", "cmovnl", "cmovge", "cmovle", "cmovng",
			"cmovnle", "cmovg", "seto", "setno", "setb", "setnae", "setc",
			"setnb", "setae", "setnc", "setz", "sete", "setnz", "stetne", "setbe",
			"setna", "setnbe", "seta", "sets", "setns", "setp", "setpe", "setnp",
			"setpo", "setl", "setnge", "setnl", "setge", "setle", "setng", "setnle",
			"setg",
			NULL
	};

	char** mn;

	int changes_all = 0;
	mn = &ins_chg_all[0];
	while (*mn != NULL){
		if (fbt_strncmp(tr->cur_instr_info->mnemonic, *mn, 256)==0){
			changes_all = 1;
			break;
		}
		mn++;
	}
	int uses_flags = 0;
	mn = &ins_uses_fl[0];
	while (*mn != NULL){
		if (fbt_strncmp(tr->cur_instr_info->mnemonic, *mn, 256)==0){
			uses_flags = 1;
			break;
		}
		mn++;
	}
	int changes_some_flags = 0;
	mn = &change_some[0];
	while (*mn != NULL){
		if (fbt_strncmp(tr->cur_instr_info->mnemonic, *mn, 256)==0){
			changes_some_flags = 1;
			break;
		}
		mn++;
	}

	int result = 0;
	if (uses_flags){
		result |= FL_USES;
	}
	if (changes_all){
		result |= FL_CHANGES_ALL;
	}
	if (changes_some_flags){
		result |= FL_CHANGES_SOME;
	}
	return result;
}

typedef enum {
	OPTYPE_UNINITIALIZED,
	OPTYPE_IMMEDIATE,
	OPTYPE_REGISTER,
	OPTYPE_MEMORY
} OpType;

struct OperandInfo {
	int valid;
	unsigned gp_regs_read;
	unsigned gp_regs_written;
	OpType optype;
};

struct OperandInfo computeOperandInfo(const unsigned int operandFlags,
		const unsigned char implOperandFlags,
		const unsigned char tableFlags,
		const unsigned char operandSize,
		struct translate* ts,
		unsigned int instr_len __attribute__((unused)),
		int print __attribute__((unused)))
{
	struct OperandInfo result;
	result.valid = 1; // set to zero if we encounter an error
	result.gp_regs_read = 0;
	result.gp_regs_written = 0;
	result.optype = OPTYPE_UNINITIALIZED; // avoid uninitialized memory!

#ifdef DEBUG
	int f = debugStream;
#endif

	/* tables with the names of the registers */
	static const char const	*register_names[10][8]={
			{ "al",    "cl",    "dl",    "bl",    "ah",    "ch",    "dh",    "bh"    },
			{ "ax",    "cx",    "dx",    "bx",    "sp",    "bp",    "si",    "di"    },
			{ "eax",   "ecx",   "edx",   "ebx",   "esp",   "ebp",   "esi",   "edi"   },
			{ "mm0",   "mm1",   "mm2",   "mm3",   "mm4",   "mm5",   "mm6",   "mm7"   },
			{ "xmm0",  "xmm1",  "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",  "xmm7"  },
			{ "es",    "cs",    "ss",    "ds",    "fs",    "gs",    "ERR",   "ERR"   },
			{ "tr0",   "tr1",   "tr2",   "tr3",   "tr4",   "tr5",   "tr6",   "tr7"   },
			{ "st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" },
			/* only ring 0 */
			{ "cr0",   "ERR",   "cr2",   "cr3",   "cr4",   "ERR",   "ERR",   "ERR"   },
			/* only ring 0 */
			{ "dr0",   "dr1",   "dr2",   "dr3",   "dr4",   "dr5",   "dr6",   "dr7"   }
	};

	static const unsigned gp_register_bits[3][8]={
		{AL_BITS, CL_BITS, DL_BITS, BL_BITS, AH_BITS, CH_BITS, DH_BITS, BH_BITS},
		{AX_BITS, CX_BITS, DX_BITS, BX_BITS, SP_BITS, BP_BITS, SI_BITS, DI_BITS},
		{EAX_BITS, ECX_BITS, EDX_BITS, EBX_BITS, ESP_BITS, EBP_BITS, ESI_BITS, EDI_BITS}
	};

	unsigned char prefix = 0x0;
	const char const *seg_ovr = NULL;
	if (ts->num_prefixes!=0) {
		prefix = *(ts->cur_instr);
		int nriters = 0;
		/* look out for a prefix we handle */
		for (nriters = 0; nriters < ts->num_prefixes; ++nriters) {
			unsigned char cur_prefix = *(ts->cur_instr+nriters);
			if ((cur_prefix == PREFIX_ADDR_SZ_OVR) || (cur_prefix == PREFIX_OP_SZ_OVR))
				prefix = cur_prefix;
			if (prefix == PREFIX_ES_SEG_OVR) seg_ovr = register_names[5][0];
			if (prefix == PREFIX_CS_SEG_OVR) seg_ovr = register_names[5][1];
			if (prefix == PREFIX_SS_SEG_OVR) seg_ovr = register_names[5][2];
			if (prefix == PREFIX_DS_SEG_OVR) seg_ovr = register_names[5][3];
			if (prefix == PREFIX_FS_SEG_OVR) seg_ovr = register_names[5][4];
			if (prefix == PREFIX_GS_SEG_OVR) seg_ovr = register_names[5][5];
		}
	}

	if (seg_ovr != NULL && hasMemOp(operandFlags)) {
#ifdef DEBUG
		if (print) fllprintf(f, "%%");
		if (print) fllprintf(f, seg_ovr);
		if (print) fllprintf(f, ":");
#endif
		result.valid = 0; // unsupported
		return result;
	}

	if (implOperandFlags!=NONE) {
		/* implicit operands */
		if (!(implOperandFlags & REG_TYPE_MASK)) {
#ifdef DEBUG
			if (print) fllprintf(f, "$%d", implOperandFlags & REG_IDX_MASK);
#endif
			result.optype = OPTYPE_IMMEDIATE;
			return result;
		} else {
			int table_select = ((implOperandFlags&REG_TYPE_MASK)>>4)-1;
			if (prefix==PREFIX_OP_SZ_OVR || prefix==PREFIX_ADDR_SZ_OVR)
				table_select--;
#ifdef DEBUG
			if (print) fllprintf(f, "%%%s", register_names[table_select][(implOperandFlags & REG_IDX_MASK)]);
#endif
			if (table_select == 0 ||
					table_select == 1 ||
					table_select == 2){
				if ((operandFlags & OP_USE_MASK) & READ){
					result.gp_regs_read |= gp_register_bits[table_select][(implOperandFlags & REG_IDX_MASK)];
				}
				if ((operandFlags & OP_USE_MASK) & WRITE){
					result.gp_regs_written |= gp_register_bits[table_select][(implOperandFlags & REG_IDX_MASK)];
				}
				result.optype = OPTYPE_REGISTER;
				return result;
			} else {
				result.valid = 0; // unsupported
				return result;
			}
		}
	} else if (hasImmOp(operandFlags)) {
		/* immediate operands (after whole instruction) */
		int tipo;
		if (operandFlags & EXECUTE) {
#ifdef DEBUG
			if (print) fllprintf(f,"+");
#endif
			tipo = 1;
		} else if (hasMemOp(operandFlags) && !(operandFlags & EXECUTE)) {
#ifdef DEBUG
			if (print) fllprintf(f,"(");
#endif
			tipo = 2;
		} else {
#ifdef DEBUG
			if (print) fllprintf(f,"$");
#endif
			tipo = 3;
		}
#ifdef DEBUG
		if (print) fllprintf(f,"0x");
		unsigned char *startaddr = (ts->cur_instr + instr_len - operandSize);
#endif
		switch (operandSize) {
		case 1:
#ifdef DEBUG
			if (print) fllprintf(f, "%.2x", *(startaddr));
#endif
			break;
		case 2:
#ifdef DEBUG
			if (print) fllprintf(f, "%.4x", *((unsigned short*)startaddr));
#endif
			break;
		case 4:
#ifdef DEBUG
			if (print) fllprintf(f, "%.8x", *((unsigned int*)startaddr));
#endif
			break;
		default:
			;
#ifdef DEBUG
			if (print) fllprintf(f, "not supported");
#endif
		}
		if (hasMemOp(operandFlags) && !(operandFlags&EXECUTE)) {
#ifdef DEBUG
			if (print) fllprintf(f,")");
#endif
		}
		if (tipo == 1){
			result.valid = 0;
			return result;
		} else if (tipo == 2){
			result.optype = OPTYPE_MEMORY;
			return result;
		} else {
			result.optype = OPTYPE_IMMEDIATE;
			return result;
		}
	} else if (tableFlags&HAS_MODRM) {
		/* modrm byte */
		if (ModRMparseRM(operandFlags)) {
			/* we read our information from the RM part, this is the regular 'free'
         option */
			int table_select = 0;
			switch (operandFlags&OP_ADDRM_MASK) {
			case ADDRM_E:
			case ADDRM_M:
			case ADDRM_R:
				table_select = 2;
				break;
			case ADDRM_Q:
			case ADDRM_U:
			case ADDRM_W:
				table_select = 4;
				break;
			}
			if (prefix==PREFIX_OP_SZ_OVR || prefix==PREFIX_ADDR_SZ_OVR)
				table_select--;
			unsigned char modrm = *(ts->first_byte_after_opcode);
			/* decode ModRM byte */
			if (MODRM_MOD(modrm)==0x3) {
				/* only regs - plain and simple, just print it */
#ifdef DEBUG
				if (print) fllprintf(f, "%%%s", register_names[table_select][MODRM_RM(modrm)]);
#endif
				if (table_select < 3) { // gp register
					if ((operandFlags & OP_USE_MASK) & READ){
						result.gp_regs_read |= gp_register_bits[table_select][MODRM_RM(modrm)];
					}
					if ((operandFlags & OP_USE_MASK) & WRITE){
						result.gp_regs_written |= gp_register_bits[table_select][MODRM_RM(modrm)];
					}
					result.optype = OPTYPE_REGISTER;
					return result;
				} else {
					// operand is some non-general-purpuse register
					result.optype = OPTYPE_REGISTER; // questionable... what meaning?
					return result;
				}
			} else if (MODRM_MOD(modrm)==0 && MODRM_RM(modrm)==5) {
				/* special disp32 - we just print int */
#ifdef DEBUG
				if (print) fllprintf(f, "%.8x", *((unsigned int*)(ts->first_byte_after_opcode+1)));
#endif
				result.optype = OPTYPE_MEMORY;
				return result; // TODO: probably it is an immediate
			} else {
				if (prefix==PREFIX_ADDR_SZ_OVR) {
#ifdef DEBUG
					if (print) fllprintf(f, "ModRMERR");
#endif
					result.valid = 0;
					return result;
				};
				/* we have some compination of disp and register and maybe a sib
           escape */
#ifdef DEBUG
			/* offset for disp value */
			int dispstarts = (MODRM_RM(modrm)==4) ? 2 : 1;
#endif 

				switch (MODRM_MOD(modrm)) {
				case 0:
					if (MODRM_RM(modrm)==4 &&
							SIB_BASE(*(ts->first_byte_after_opcode+1))==5) {
						/* sib byte includes a backward reference to an disp32 */
#ifdef DEBUG
						if (print) fllprintf(f, "0x%.8x",
								*((unsigned int*)(ts->first_byte_after_opcode +
										dispstarts)));
#endif
					}
					break;
				case 1:
#ifdef DEBUG
					if (print) fllprintf(f, "0x%.2x", *(ts->first_byte_after_opcode +
							dispstarts));
#endif
break;
				case 2:
#ifdef DEBUG
					if (print) fllprintf(f, "0x%.8x",
							*((unsigned int*)(ts->first_byte_after_opcode +
									dispstarts)));
#endif
					break;
				}
#ifdef DEBUG
				if (print) fllprintf(f, "(");
#endif
				if (MODRM_RM(modrm)==4) {
					/* sib byte - we need to decode that as well */
					unsigned char sib = *(ts->first_byte_after_opcode+1);
					if (SIB_BASE(sib)!=5) {
						/* sib base register */
#ifdef DEBUG
						if (print) fllprintf(f, "%%%s", register_names[2][SIB_BASE(sib)]);
#endif
   					result.gp_regs_read |= gp_register_bits[2][SIB_BASE(sib)];
						//*uses |= (EAX_BITS<<(SIB_BASE(sib)*4));
					} else {
						/* special sib position */
						switch (MODRM_MOD(modrm)) {
						case 1:
						case 2:
							;
#ifdef DEBUG
if (print) fllprintf(f, "%%ebp");
#endif
						result.gp_regs_read |= EBP_BITS;
						}
					}
					if (SIB_INDEX(sib)!=4) {
						/* print scaled index register */
#ifdef DEBUG
						if (print) fllprintf(f, ", %%%s", register_names[2][SIB_INDEX(sib)]);
#endif
						result.gp_regs_read |= gp_register_bits[2][SIB_INDEX(sib)];
						//*uses |= (EAX_BITS<<(SIB_INDEX(sib)*4));

						switch (SIB_SCALE(sib)) {
						/* sib byte used to scale index */
						case 0:
							break;
						case 1:
#ifdef DEBUG
							if (print) fllprintf(f, "*2");
#endif
							break;
						case 2:
#ifdef DEBUG
							if (print) fllprintf(f, "*4");
#endif
							break;
						case 3:
#ifdef DEBUG
							if (print) fllprintf(f, "*8");
#endif
							break;
						}
					}
				} else {
#ifdef DEBUG
					if (print) fllprintf(f, "%%%s", register_names[table_select][MODRM_RM(modrm)]);
#endif
					if (table_select < 3){
					result.gp_regs_read |= gp_register_bits[table_select][MODRM_RM(modrm)];
					}
					//*uses |= (1<<MODRM_RM(modrm));
				}
#ifdef DEBUG
				if (print) fllprintf(f, ")");
#endif
				result.optype = OPTYPE_MEMORY;
				return result;
			}
		} else if (ModRMparseREG(operandFlags)) {
			/* we parse the REG part of the ModRM byte, this is the more restricted
         option (the top column of the ModR/M table)
			 */
			int table_select=0;
			switch (operandFlags&OP_ADDRM_MASK) {
			case ADDRM_G:
			case ADDRM_S:
				table_select = 2;
				break; /* unsure about S */
			case ADDRM_N:
			case ADDRM_V:
				table_select = 4;
				break;
				/* these two need ring 0 privs: */
			case ADDRM_C:
				table_select = 8;
				break;
			case ADDRM_D:
				table_select = 9;
				break;
			}
			unsigned char modrm = *(ts->first_byte_after_opcode);
#ifdef DEBUG
			if (print) fllprintf(f, "%%%s", register_names[table_select][MODRM_REG(modrm)]);
#endif
			if (table_select == 0 ||
					table_select == 1 ||
					table_select == 2){
				if ((operandFlags & OP_USE_MASK) == READ){
					result.gp_regs_read |= gp_register_bits[table_select][MODRM_REG(modrm)];
				}
				if ((operandFlags & OP_USE_MASK) == WRITE){
					result.gp_regs_written |= gp_register_bits[table_select][MODRM_REG(modrm)];
				}
				result.optype = OPTYPE_REGISTER;
				return result;
			} else {
				result.valid = 0;
				return result;
			}
		} else {
			/* although this instructions has a ModR/M byte,
         this argument (either dst, src, aux) does not use it*/
			result.valid = 0;
			return result;
		}
	} else if ((operandFlags&ADDRM_X) == ADDRM_X) {
#ifdef DEBUG
		if (print) fllprintf(f, "%ds:(%esi)");
#endif
		result.valid = 0;
		return result;
	} else if ((operandFlags&ADDRM_Y) == ADDRM_Y) {
#ifdef DEBUG
		if (print) fllprintf(f, "%es:(%edi)");
#endif
		result.valid = 0;
		return result;
	}
	result.valid = 0;
	return result;
}

/**
 * @param translate translation struct containing the instruction
 *                  that we want to 'decode'
 * @param instr the fields of instr concerning registers are filled
 *              by this function.
 */
void compute_instruction_register_usage(struct translate* ts,
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 struct lmem_instruction* instr) {

	int length = ts->next_instr - ts->cur_instr;
	int print = 1;

	instr->gp_registers_read=0;
	instr->gp_registers_written=0;

	struct OperandInfo srcinfo;
	struct OperandInfo dstinfo;
	struct OperandInfo auxinfo;
	int si = 0;
	int di = 0;
	int ai = 0;

	if (ts->cur_instr_info->srcFlags) {
		srcinfo = computeOperandInfo(ts->cur_instr_info->srcFlags,
				ts->cur_instr_info->implSrcFlags,
				ts->cur_instr_info->tableFlags,
				ts->src_operand_size, ts, length, print);
		si = 1;
		PRINT_DEBUG("has src");
	}
	if (ts->cur_instr_info->destFlags) {
		dstinfo = computeOperandInfo(ts->cur_instr_info->destFlags,
				ts->cur_instr_info->implDestFlags,
				ts->cur_instr_info->tableFlags,
				ts->dest_operand_size, ts, length, print);
		di = 1;
		PRINT_DEBUG("has dest");
	}
	if (ts->cur_instr_info->auxFlags) {
		auxinfo = computeOperandInfo(ts->cur_instr_info->auxFlags,
				ts->cur_instr_info->implAuxFlags,
				ts->cur_instr_info->tableFlags,
				ts->aux_operand_size, ts, length, print);
		ai = 1;
		PRINT_DEBUG("has aux");
	}

	if ((si && !srcinfo.valid) ||
			(di && !dstinfo.valid) ||
			(ai && !auxinfo.valid)) {
		instr->supported = 0;
		instr->gp_registers_read = 0xFFFFFFFF;
		instr->gp_registers_written = 0xFFFFFFFF;
	} else {
		instr->gp_registers_read = 0;
		if (si) instr->gp_registers_read |= srcinfo.gp_regs_read;
		if (di) instr->gp_registers_read |= dstinfo.gp_regs_read;
		if (ai) instr->gp_registers_read |= auxinfo.gp_regs_read;

		instr->gp_registers_written = 0;
		if (si) instr->gp_registers_written |= srcinfo.gp_regs_written;
		if (di) instr->gp_registers_written |= dstinfo.gp_regs_written;
		if (ai) instr->gp_registers_written |= auxinfo.gp_regs_written;
	}

	// Handle special cases:

	// Stack instructions
	unsigned int ocf = ts->cur_instr_info->opcodeFlags;
	if ((ocf & OPCODEFL_INS_GROUP_MASK) == OPCODEFL_STACK){
		instr->gp_registers_read |= ESP_BITS;
		instr->gp_registers_written |= ESP_BITS;
	}
	if (ocf == (OPCODEFL_DIV)){

	}
}

void compute_memory_accesses(struct translate* ts,
																		struct lmem_instruction* result)
{
	////////////////////////////////////////////////////////////////
	// maybe it can be considered hackish just to ignore ignore jumps...
	// todo: WE HAVE TO BE MUCH MORE SOPHISTICATED HERE!
	////////////////////////////////////////////////////////////////

	const int LEAL = 0x8D;

	result->reads_memory = 0;
	result->writes_memory = 0;

	if ((ts->cur_instr_info->opcodeFlags & OPCODEFL_INS_GROUP_MASK) == OPCODEFL_EXEC){
		return;
	}

	if (*(ts->cur_instr + ts->num_prefixes) == LEAL){
		// although lea looks like a memory access instruction
		// it doesn't access memory
		return;
	}
	if (MODRM_MOD(*ts->first_byte_after_opcode) == 0x3){
		// the 'effective address' is in reality a register
		// see intel manual modrm table
		return;
	}
	if (hasMemOp(ts->cur_instr_info->srcFlags)){
		result->reads_memory = 1;
	}
  if (hasMemOp(ts->cur_instr_info->destFlags)){
  	result->writes_memory = 1;
  }
}

void compute_lmem_instruction(struct translate* ts,
		                          struct lmem_instruction* result)
{
	fbt_memset(result, sizeof(*result), 0);
	result->supported = 1; // may be set to false further down
	result->arithmetic_flag_info = compute_flags_info(ts);
	compute_instruction_register_usage(ts, result);
	compute_memory_accesses(ts, result);
}



