/**
 * This handles the debug output that can be customized in the Makefile
 *
 * Copyright (c) 2010 ETH Zurich
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
#include <string.h>

#include "fbt_datatypes.h"
#include "fbt_disassemble.h"
#include "fbt_debug.h"

#define PRINT__BUF__SIZE 512

char print_buf[PRINT__BUF__SIZE];
char scan_half_byte(unsigned char hb)
{
	switch(hb & 0x0f) {
		case 10:
			return 'a';
		case 11:
			return 'b';
		case 12:
			return 'c';
		case 13:
			return 'd';
		case 14:
			return 'e';
		case 15:
			return 'f';
	}
	return hb + '0';
}

char* printnbytes(unsigned char *addr, unsigned int n)
{
	unsigned int i;
	for(i=0; i<n && i<(PRINT__BUF__SIZE-1)/2; ++i) {
		print_buf[2*i] = scan_half_byte(addr[i] >> 4);
		print_buf[2*i+1] = scan_half_byte(addr[i] & 0x0f);
	}
	print_buf[2*i] = '\0';
	return print_buf;
}



#ifdef DUMP_GENERATED_CODE

/*
int disasm_inst(struct translate_struct* ts, char *buf)
{
    struct instr i;
	int size;
	//size = disassemble_address(addr, &i);

	sprintf(buf, "%-7s", i.mnemonic);
	if (i.src[0] && i.aux[0] && i.dest[0]) {
	    sprintf(buf+7, " %s, %s, %s", i.src, i.aux, i.dest);
	}
	if (i.src[0] && i.aux[0] && !i.dest[0]) {
	    sprintf(buf+7, " %s, %s", i.src, i.aux);
	}
	if (i.src[0] && !i.aux[0] && i.dest[0]) {
	    sprintf(buf+7, " %s, %s", i.src, i.dest);
	}
	if (!i.src[0] && i.aux[0] && i.dest[0]) {
	    sprintf(buf+7, " %s, %s", i.aux, i.dest);
	}
	if (i.src[0] && !i.aux[0] && !i.dest[0]) {
	    sprintf(buf+7, " %s", i.src);
	}
	if (!i.src[0] && i.aux[0] && !i.dest[0]) {
	    sprintf(buf+7, " %s", i.aux);
	}
	if (!i.src[0] && !i.aux[0] && i.dest[0]) {
	    sprintf(buf+7, " %s", i.dest);
	}

	return (size);

}
*/

/* implizit operand masks */
#define		REG_IDX_MASK		0x0000000F
#define		REG_TYPE_MASK		0x000000F0
int printOperandString(FILE* f, const unsigned int operandFlags,
		       const unsigned char implOperandFlags, const unsigned char tableFlags,
		       struct translate_struct* ts, unsigned int instr_len)
{
    /* tables with the names of the registers */
    const char const *register_names[10][8]={{ "al",    "cl",    "dl",    "bl",    "ah",    "ch",    "dh",    "bh"    },
					    { "ax",    "cx",    "dx",    "bx",    "sp",    "bp",    "si",    "di"    },
					    { "eax",   "ecx",   "edx",   "ebx",   "esp",   "ebp",   "esi",   "edi"   },
					    { "mm0",   "mm1",   "mm2",   "mm3",   "mm4",   "mm5",   "mm6",   "mm7"   },
					    { "xmm0",  "xmm1",  "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",  "xmm7"  },
					    { "es",    "cs",    "ss",    "ds",    "fs",    "gs",    "ERR",   "ERR"   },
					    { "tr0",   "tr1",   "tr2",   "tr3",   "tr4",   "tr5",   "tr6",   "tr7"   },
					    { "st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" },
		/* only ring 0 */	    { "cr0",   "ERR",   "cr2",   "cr3",   "cr4",   "ERR",   "ERR",   "ERR"   },
		/* only ring 0 */	    { "dr0",   "dr1",   "dr2",   "dr3",   "dr4",   "dr5",   "dr6",   "dr7"   }};
    unsigned char prefix = 0x0;
    if (ts->num_prefixes!=0) {
	prefix = *(ts->cur_instr);
	int nriters = 0;
	/* look out for a prefix we handle */
	while ((prefix!=PREFIX_ADDR_SZ_OVR) && (prefix!=PREFIX_OP_SZ_OVR)) {
	    nriters++;
	    /* did we use up all prefixes? */
	    if (nriters==ts->num_prefixes) { prefix=0x0; break; }
	    prefix = *(ts->cur_instr+nriters);
	}
    }
    if (implOperandFlags!=NONE) {
	/* implicit operands */
	if (!(implOperandFlags & REG_TYPE_MASK)) {
	    fprintf(f, "$%d", implOperandFlags&REG_IDX_MASK);
	    return 2;
	} else {
	    int table_select = ((implOperandFlags&REG_TYPE_MASK)>>4)-1;
	    if (prefix==PREFIX_OP_SZ_OVR || prefix==PREFIX_ADDR_SZ_OVR) table_select--;
	    fprintf(f, "%%%s", register_names[table_select][(implOperandFlags&REG_IDX_MASK)]);
	    return 1+strnlen(register_names[table_select][(implOperandFlags&REG_IDX_MASK)],6);
	}
    } else if (hasImmOp(operandFlags)) {
	/* immediate operands (after whole instruction) */
	int len = 0;
	if (operandFlags&EXECUTE) {
	    fprintf(f,"+");
	    len+=1;
	} else if (hasMemOp(operandFlags) && !(operandFlags&EXECUTE)) {
	    fprintf(f,"(");
	    len+=1;
	} else {
	    fprintf(f,"$");
	    len+=1;
	}
	len+=2+2*operandSize(operandFlags, prefix);
	fprintf(f,"0x");
	unsigned char *startaddr = (ts->cur_instr+instr_len-operandSize(operandFlags, prefix));
	switch (operandSize(operandFlags, prefix)) {
	case 1: fprintf(f, "%.2x", *(startaddr)); break;
	case 2: fprintf(f, "%.4x", *((unsigned short*)startaddr)); break;
	case 4: fprintf(f, "%.8x", *((unsigned int*)startaddr)); break;
	default: fprintf(f, "not supported");
	}
	if (hasMemOp(operandFlags) && !(operandFlags&EXECUTE)) {
	    fprintf(f,")");
	    len+=1;
	}
	return len;
    } else if (tableFlags&HAS_MODRM) {
	/* modrm byte */
	if (ModRMparseRM(operandFlags)) {
	    /* we read our information from the RM part, this is the regular 'free' option */
	    int table_select=0;
	    switch (operandFlags&OP_ADDRM_MASK) {
	    case ADDRM_E:
	    case ADDRM_M:
	    case ADDRM_R: table_select = 2; break;
	    case ADDRM_Q:
	    case ADDRM_U:
	    case ADDRM_W: table_select = 4; break;
	    }
	    if (prefix==PREFIX_OP_SZ_OVR || prefix==PREFIX_ADDR_SZ_OVR) table_select--;
	    int len=0;
	    unsigned char modrm = *(ts->first_byte_after_opcode);
	    /* decode ModRM byte */
	    if (MODRM_MOD(modrm)==0x3) {
		/* only regs - plain and simple, just print it */
		fprintf(f, "%%%s", register_names[table_select][MODRM_RM(modrm)]);
		len+=1+strnlen(register_names[table_select][MODRM_RM(modrm)],6);
	    } else if (MODRM_MOD(modrm)==0 && MODRM_RM(modrm)==5) {
		/* special disp32 - we just print int */
		fprintf(f, "%.8x", *((unsigned int*)(ts->first_byte_after_opcode+1)));
		len+=8;
	    } else {
		if (prefix==PREFIX_ADDR_SZ_OVR) { fprintf(f, "ModRMERR"); return len+8; };
		/* we have some compination of disp and register and maybe a sib escape */
		int dispstarts = (MODRM_RM(modrm)==4) ? 2 : 1; /* offset for disp value */
		switch (MODRM_MOD(modrm)) {
		case 0: if (MODRM_RM(modrm)==4 && SIB_BASE(*(ts->first_byte_after_opcode+1))==5) {
			/* special case, sib byte includes a backward reference to an disp32 */
			fprintf(f, "0x%.8x", *((unsigned int*)(ts->first_byte_after_opcode+dispstarts))); len+=10;
		    }
		    break;
		case 1: fprintf(f, "0x%.2x", *(ts->first_byte_after_opcode+dispstarts)); len+=4; break;
		case 2: fprintf(f, "0x%.8x", *((unsigned int*)(ts->first_byte_after_opcode+dispstarts))); len+=10; break;
		}
		fprintf(f, "("); len++;
		if (MODRM_RM(modrm)==4) {
		    /* sib byte - we need to decode that as well */
		    unsigned char sib = *(ts->first_byte_after_opcode+1);
		    if (SIB_BASE(sib)!=5) {
			/* sib base register */
			fprintf(f, "%%%s", register_names[2][SIB_BASE(sib)]);
			len+=1+strnlen(register_names[2][SIB_BASE(sib)],6);
		    } else {
			/* special sib position */
			switch (MODRM_MOD(modrm)) {
			case 1:
			case 2:
			    fprintf(f, "%%ebp");
			    len+=4;
			}
		    }
		    if (SIB_INDEX(sib)!=4) {
			/* print scaled index register */
			fprintf(f, ", %%%s", register_names[2][SIB_INDEX(sib)]);
			len+=3+strnlen(register_names[2][SIB_INDEX(sib)],6);
			switch (SIB_SCALE(sib)) {
			/* sib byte used to scale index */
			case 0: break;
			case 1: fprintf(f, "*2"); len+=2; break;
			case 2: fprintf(f, "*4"); len+=2; break;
			case 3: fprintf(f, "*8"); len+=2; break;
			}
		    }
		} else {
		    fprintf(f, "%%%s", register_names[table_select][MODRM_RM(modrm)]);
		    len+=1+strnlen(register_names[table_select][MODRM_RM(modrm)],6);
		}
		fprintf(f, ")"); len++;
	    }
	    return len;
	} else if (ModRMparseREG(operandFlags)) {
	    /* we parse the REG part of the ModRM byte, this is the more restricted option */
	    int table_select=0;
	    switch (operandFlags&OP_ADDRM_MASK) {
	    case ADDRM_G:
	    case ADDRM_S: table_select = 2; break; /* unsure about S */
	    case ADDRM_N:
	    case ADDRM_V: table_select = 4; break;
		/* these two need ring 0 privs: */
	    case ADDRM_C: table_select = 8; break;
	    case ADDRM_D: table_select = 9; break;
	    }
	    unsigned char modrm = *(ts->first_byte_after_opcode);
	    fprintf(f, "%%%s", register_names[table_select][MODRM_REG(modrm)]);
	    return 1+strnlen(register_names[table_select][MODRM_REG(modrm)],6);
	} else {
	    fprintf(f, "ModRMERR");
	    return 8;
	}
    }
    return 0;
}

/**
 * Prints instructions to the file (and disassembles it)
 */
void print_disasm_inst(FILE* f, struct translate_struct* ts, unsigned int instr_len)
/* we need to pass instr_len of the current instruction because we can't trust the ts struct
   the problem is that a call instruction will tether next_instr and we cannot calculate the
   length of the current instruction by subtracting next_instr - cur_instr as next_instr will
   follow the call! so we need to pass the length as well!
 */
{
    int j, plen=0, args=0;
    fprintf(f, "0x%.8x: ", (unsigned int)ts->cur_instr);
    for(j=0; j<instr_len; ++j) {
	fprintf(f, "%.2x", (unsigned char)(*(ts->cur_instr+j)));
    }
    for(j=0; j<24-2*instr_len; ++j) {
	fprintf(f, " ");
    }
    fprintf(f, "%s ", ts->cur_instr_info->mnemonic);
    plen = strnlen(ts->cur_instr_info->mnemonic,16)+1;

    if (ts->cur_instr_info->srcFlags) {
	plen+=printOperandString(f, ts->cur_instr_info->srcFlags, ts->cur_instr_info->implSrcFlags, ts->cur_instr_info->tableFlags, ts, instr_len);
	args=1;
    }
    if (ts->cur_instr_info->auxFlags) {
	if (args==1) { fprintf(f, ", "); plen+=2; }
	plen+=printOperandString(f, ts->cur_instr_info->auxFlags, ts->cur_instr_info->implAuxFlags, ts->cur_instr_info->tableFlags, ts, instr_len);
	args=1;
    }
    if (ts->cur_instr_info->destFlags) {
	if (args==1) { fprintf(f, ", "); plen+=2; }
	plen+=printOperandString(f, ts->cur_instr_info->destFlags, ts->cur_instr_info->implDestFlags, ts->cur_instr_info->tableFlags, ts, instr_len);
	args=1;
    }
    
    for(j=0; j<32-plen; ++j) {
	fprintf(f, " ");
    }
    
/* 	int tot_size=0, size, j; */
/* 	char buf[100]; */

/* 	while (tot_size < len) { */
/* 		size = disasm_inst((unsigned char*)(addr+tot_size), buf); */
/* 		fprintf(f, "%p:   ", (addr+tot_size)); */
/* 		for(j=0; j<size; ++j) { */
/* 			fprintf(f, "%.2x", (unsigned char)(*(addr+tot_size+j))); */
/* 		} */
/* 		for(j=0; j<26-2*size; ++j) { */
/* 			fprintf(f, " "); */
/* 		} */
/* 		fprintf(f, "%s\n", buf); */

/* 		tot_size += size; */
/* 		/\* break out of the loop, if we have an invalid instr. *\/ */
/* 		if (size==0) { */
/* 		  printf("fastBT print_disasm_inst: size=0 (%p, disas started at %p)\n", addr+tot_size, addr); */
/* 		  break; */
/* 		} */
/* 	} */
}

#endif

#ifdef PBREAKGDB

void * break_on_transl_addr = NULL;

void pbreakgdb(void *break_addr) __attribute__((noinline));
void pbreakgdb(void *break_addr) {
	break_on_transl_addr = break_addr;
}

void break_on_transl(void *qorg_addr, void *qtransl_addr) __attribute__((noinline));
void break_on_transl(void *qorg_addr, void *qtransl_addr) {
	//reference pbreakgdb to make sure it's not optimized away
	pbreakgdb(qorg_addr);
}

#endif
