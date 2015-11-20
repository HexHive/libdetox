/**
 * @file fbt_lmem.c
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

#ifdef LMEM

#include "fbt_lmem.h"

#include "fbt_lmem_util.h"
#include "fbt_lmem_spilling.h"

#include "fbt_translate.h"
#include "fbt_actions.h"
#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "fbt_x86_opcode.h"
#include "fbt_syscall.h"
#include "sdbg_insert_lea.h"

#include "fbt_lmem_api.h"
#include "fbt_lmem_instruction.h"

#include <assert.h>
#include <unistd.h> // for size_t
#include <asm-generic/mman.h>

/*
 * Since we rewrote the translation function, we
 * don't support some fancy functions that would
 * never be used together with lmem anyway
 */
#ifdef TRACK_BASIC_BLOCKS
#error lmem does not support TRACK_BASIC_BLOCKS
#endif
#ifdef SECU_ENFORCE_NX
#error lmem does not support SECU_ENFORCE_NX
#endif

/* bit test masks for ModR/M byte */
#define MASK_MOD 0xC0
#define MASK_REG 0x38
#define SHIFT_REG 3
#define MASK_RM  0x07

#define SAVE_FLAGS \
        movl %eax, {&tld->saved_eax}; \
				lahf; \
				seto %al; \
        movl %eax, {&tld->saved_arith_flags}; \
        movl {&tld->saved_eax}, %eax;

#define RESTORE_FLAGS \
        movl %eax, {&tld->saved_eax}; \
			  movl {&tld->saved_arith_flags}, %eax; \
				addb $0x7f, %al;  \
				sahf; \
      	movl {&tld->saved_eax}, %eax;

/**
 * Allocates memory for all lmem specific data structures.
 * For now it is just the 'shadow memory'.
 */
void lmem_init(){
	int retval;
	int i;
	int siz = 0x2000000;
	fbt_mmap( 0x40000000, siz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,  \
			-1, 0, retval, "BT failed to allocate memory (fbt_mem_pool_init: "
			"fbt_mem_mgmt.c)\n");
	for (i=0; i<siz; i++){
		*((char*)(0x40000000+i))=1;
	}
	for (i=0; i<siz; i++){
		*((char*)(0x40000000+i))=0;
	}
}

unsigned get_jump_target(const unsigned char *addr, int length){
	assert(!HAS_PREFIX(*addr)); /* no prefixes allowed */
	int32_t jump_target=0;
	if (*addr == 0xE9) {  /* 32bit offset */
		jump_target = *((int32_t*)(addr + 1)) + (int32_t)addr + length;
	} else { /* our argument is only an 8bit offset */
		jump_target = (int32_t)(*((char*)(addr + 1)) + addr + length);
	}
	return jump_target;
}

/**
 * @param condjmp a disassembled conditional jump instruction
 * @param result an array of size 2 which is filled with the two target addresses
 */
static void get_conditional_jump_targets(const struct translate* condjmp, unsigned* result){
	unsigned char *addr = condjmp->cur_instr;
	int length = condjmp->next_instr - condjmp->cur_instr;

	ulong_t jump_target;
	if (condjmp->num_prefixes != 0) {
		llprintf("Instruction at %p uses prefixes (len: %d)!\n", addr, length);
		if (*addr == PREFIX_FS_SEG_OVR) {
			addr++;
			length--;
		} else
			fbt_suicide_str("No prefixes handled in conditional jump!\n");
	}
	assert(((*addr == 0x0F && length == 6) || (length == 2)));
	/* no prefixes handled */

	/* check if we have jecxz (jump if ecx register is zero) */
	if (*addr == 0xE3) {
		PRINT_DEBUG("... processing jecxz");
		jump_target = *((char*) (addr + 1)) + ((ulong_t)addr) + length;
	} else {
		if (*(unsigned char*) addr != 0x0F) {
			PRINT_DEBUG("... processing one-byte jcc");
			jump_target = *((char*) (addr + 1)) + ((ulong_t)addr) + length;
		} else {
			PRINT_DEBUG("... processing two-byte jcc");
			jump_target = *((ulong_t*) (addr + 2)) + ((ulong_t)addr) + length;
		}
	}

	result[0] = jump_target;
	result[1] = (unsigned)(condjmp->next_instr);
}

// returns 1 if it doesn't need them
// returns 0 if it can't prove that it doesn't need them

static void lmem_check_address(){
	unsigned eaxreg, ecxreg;
	asm __volatile__("": "=a"(eaxreg), "=c"(ecxreg) : );
	/* internal debugging check */
	if ((eaxreg&0xFFFFFF00) != ((ecxreg&0x1FFFF00)|0x40000000)) {
		llprintf("fail %p %p, %p, %p\n", eaxreg, ecxreg, eaxreg&0xFFFFFF00, (ecxreg&0x1FFFF00)|0x40000000);
		fbt_suicide_str("FIAL\n");
	}
	//llprintf("lmem: invalid memory access\n");
	llprintf("lmem: detected possibly invalid memory access at %p\n", ecxreg);
	//llprintf("TODO: here we would check if it is really invalid\n");
	asm __volatile__("hlt");
}

#ifdef DEBUG
extern int debugStream;

void print_disasm_inst(int f, struct translate* ts,
                              unsigned int instr_len);
#endif

enum FlagNeeds{S_DONTNEEDFLAGS, S_NEED_FLAGS_IN_EFLAGS, S_NEED_FLAGS_IN_MEMORY};
enum FlagsAction{EF_SAV=1, EF_RST=2};

static void analyze_flags_save_restore_needs(struct lmem_instruction* instructions,
																						 struct translate* transl,
																						 int* needs_memcheck,
																						 int num_instructions,
																						 int* saverest,
																						 int* dbg_flagsneeds)
{
	if (num_instructions == 0){
		llprintf("warning... bb with 0 instructions");
		return;
	}

	PRINT_DEBUG("anfl beg");
	int i;
	for (i=0; i<num_instructions; i++){
		saverest[i] = 0;
		dbg_flagsneeds[i] = 0;
	}

	/**
	 * The protocols for passing the flags between one basic block
	 * and another says that if the next basic block might need the flags
	 * then they need to be put in the eflags register by the preceding
	 * basic block.
	 */
	int s = S_NEED_FLAGS_IN_EFLAGS;  // this holds what the next instruction needs

#ifdef ENABLE_UNSOUND_ASSUMPTIONS
	if (fbt_strncmp(transl[num_instructions-1].cur_instr_info->mnemonic, "ret", 256)==0){
		s = S_DONTNEEDFLAGS;
	}
	if (transl[num_instructions-1].cur_instr_info->opcode.handler == action_call){
		s = S_DONTNEEDFLAGS;
	}
	if (transl[num_instructions-1].cur_instr_info->opcode.handler == action_jmp_indirect){
		s = S_DONTNEEDFLAGS;
	}
#endif

	i=num_instructions-1; // last instr. of basic block
	while (i>=0){
		if (i+1 < num_instructions){
			dbg_flagsneeds[i+1] = s;
		}
		PRINT_DEBUG("i = %d", i);
		if (s == S_DONTNEEDFLAGS){
			if (instructions[i].arithmetic_flag_info & FL_USES &&
					instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("f");
				if (needs_memcheck[i]){
					s = S_NEED_FLAGS_IN_MEMORY;
					saverest[i] |= EF_RST;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS;
				}
			}
			if (instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("a");
				// we don't care about flags
				// next instructions don't care about flags
				// so we're ok doing nothing
			} else if (instructions[i].arithmetic_flag_info & FL_CHANGES_SOME) {
				PRINT_DEBUG("b");
				// we don't care about flags
				// next instructions don't care about flags
				// so we're ok doing nothing
			} else if (instructions[i].arithmetic_flag_info & FL_USES){
				PRINT_DEBUG("c");
				if (needs_memcheck[i]){
					saverest[i] |= EF_RST;
					s = S_NEED_FLAGS_IN_MEMORY;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS;
				}
			} else {
				PRINT_DEBUG("d");
			}
		} else if (s == S_NEED_FLAGS_IN_EFLAGS) {
			if (instructions[i].arithmetic_flag_info & FL_USES &&
					instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("F");
				if (needs_memcheck[i]){
					saverest[i] |= EF_RST;
					s = S_NEED_FLAGS_IN_MEMORY;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS; // just to be explicit
				}
			} else if (instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("A");
				// since this instruction changes all flags, they are automatically in eflags
				// also, this instruction doesn't care about flags
				s = S_DONTNEEDFLAGS;
			} else if (instructions[i].arithmetic_flag_info & FL_CHANGES_SOME) {
				PRINT_DEBUG("B");
				if (needs_memcheck[i]){
					saverest[i] |= EF_RST;
					s = S_NEED_FLAGS_IN_MEMORY;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS; // just to be explicit
				}
			} else if (instructions[i].arithmetic_flag_info & FL_USES){
				PRINT_DEBUG("C");
				if (needs_memcheck[i]){
					saverest[i] |= EF_RST;
					s = S_NEED_FLAGS_IN_MEMORY;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS; // just to be explicit
				}
			} else {
				PRINT_DEBUG("D");
				if (needs_memcheck[i]){
					s = S_NEED_FLAGS_IN_MEMORY;
					saverest[i] |= EF_RST;
				}
			}
		} else if (s == S_NEED_FLAGS_IN_MEMORY) {
			if (instructions[i].arithmetic_flag_info & FL_USES &&
					instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("6.");
				if (needs_memcheck[i]){
					s = S_NEED_FLAGS_IN_MEMORY;
					saverest[i] |= EF_RST;
					saverest[i+1] |= EF_SAV;
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS;
					saverest[i+1] |= EF_SAV; // next instruction will have to put them in memory
				}
			} else if (instructions[i].arithmetic_flag_info & FL_CHANGES_ALL){
				PRINT_DEBUG("1.");
				saverest[i+1] |= EF_SAV; // the next instruction will need to save them
				s = S_DONTNEEDFLAGS; // because this instruction would change them anyway
			} else if (instructions[i].arithmetic_flag_info & FL_CHANGES_SOME) {
				PRINT_DEBUG("2.");
				if (needs_memcheck[i]){
					s = S_NEED_FLAGS_IN_MEMORY;  // ??????????
					saverest[i] |= EF_RST;
					saverest[i+1] |= EF_SAV;  // LOOK AGAIN LATER HERE!!!!!!!!!!!!!!!!! TODO CHECK
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS;
					saverest[i+1] |= EF_SAV;  // next instruction will have to put them in memory
				}
			} else if (instructions[i].arithmetic_flag_info & FL_USES){
				PRINT_DEBUG("3.");
				// since this instruction uses eflags, we need the flags  to be in
				// eflags. So...
				if (needs_memcheck[i]){
					s = S_NEED_FLAGS_IN_MEMORY;
					saverest[i] |= EF_RST;
					saverest[i] |= EF_SAV; // TODO:  THIS CAN BE DELETED I THINK next instruction will have to put them in memor???
				} else {
					s = S_NEED_FLAGS_IN_EFLAGS;
					saverest[i] |= EF_SAV; // next instruction will have to put them in memory
				}
			} else {
				PRINT_DEBUG("4.");
			}
		}
		i--;
	}

	/*
	 * If the first instruction needs flags in memory then
	 * it needs to save them, since it is passed from the
	 * previous block in the eflags register
	 */
	if (s == S_NEED_FLAGS_IN_MEMORY){
		assert(num_instructions>0);
		saverest[0] |= EF_SAV;
	}

	PRINT_DEBUG("anfl end");
}

/**
 * Often it is the case that the compiler generates a 'cmp' or 'test'
 * followed by some 'mov's, 'lea's etc. finally followed by a 'jcc'.
 * It is inefficient to save and restore the flags
 */
void optimize_savefl_restorefl_pairs()
{


#if 0

	PRINT_DEBUG("\nOPTIMIZING SAVE/RESTORE PAIRS");

	int suppress[MAX_LOOKAHEAD];
	for (i=0; i<lookahead_nr+1; i++){
		suppress[i] = 0;
	}

	// experimental code
	int insert_from = -1;
	int insert_at = -1;
	int insert_len = -1;
	char insert_what[16];

	// search for a save/restore pair
	int j;
	for (i=0; i<lookahead_nr; i++){
		if (saverest[i] & EF_SAV){
			for (j=i; j<lookahead_nr; j++){
				if (saverest[j] & EF_RST){
					PRINT_DEBUG("sr pair %d %d", i, j);
					// find instruction that sets all flags
					int ins_that_sets = -1;
					int q;
					for (q=i-1; q>=0; q--){
						if (flag_infos[q] & FL_CHANGES_ALL){
							ins_that_sets = q;
							break;
						} else if (flag_infos[q] & FL_CHANGES_SOME){
							break;
						}
					}
					if (ins_that_sets != (-1)){
						// now we try to see if the instruction that
						// did set all flags is one that we are able
						// to handle nicely
						PRINT_DEBUG("flag setting instr: %s", lookahead[ins_that_sets].cur_instr_info->mnemonic);
						if (strcmp(lookahead[ins_that_sets].cur_instr_info->mnemonic, "test") == 0 ||
								strcmp(lookahead[ins_that_sets].cur_instr_info->mnemonic, "cmp") == 0) {
							struct opsinfo oi = get_instruction_register_usage(&lookahead[ins_that_sets], lookahead[ins_that_sets].next_instr - lookahead[ins_that_sets].cur_instr, 0);

							// cmp reg, reg
							// test reg, reg
							if (oi.valid && oi.dest_info < 8 && oi.source_info < 8){

								int ok = 1;

								int k;
								for (k=ins_that_sets+1; k<j; k++){
									PRINT_DEBUG("the k=%d", k);
									if (!inlist(lookahead[k].cur_instr_info->mnemonic, &safelist[0])){
										PRINT_DEBUG("'%s' not in safe list", lookahead[k].cur_instr_info->mnemonic);
										ok = 0;
										break;
									}
									struct opsinfo gg = get_instruction_register_usage(&lookahead[k], lookahead[k].next_instr - lookahead[k].cur_instr, 0);
									if (!gg.valid){
										PRINT_DEBUG("invalid");
										ok = 0;
										break;
									}
									if (gg.dest_info < 8 &&
											(gg.dest_info == oi.dest_info || gg.dest_info == oi.source_info)){
										PRINT_DEBUG("ggdi %d oidi %d oisi %d", gg.dest_info, oi.dest_info, oi.source_info);
										ok = 0;
										break;
									}
								}

								if (ok){
									PRINT_DEBUG("CAN OPTIMIZE REGREG");

									insert_from = ins_that_sets;
									insert_at = j;
									insert_len = lookahead[ins_that_sets].next_instr - lookahead[ins_that_sets].cur_instr;
									int r;
									for (r=0; r<insert_len; r++){
										insert_what[r] = *(char*)(lookahead[ins_that_sets].cur_instr+r);
									}

									// no need to save and restore explicitly any more
									saverest[i] &= (~EF_SAV);
									saverest[j] &= (~EF_RST);
									suppress[ins_that_sets] = 1;

									goto end_optimization;
								} else {
									PRINT_DEBUG("NOT OK rr");
								}

							} else if (oi.valid &&
												((oi.dest_info == RG_IMMEDIATE && oi.source_info < 8) ||
												 (oi.dest_info < 8 && oi.source_info == RG_IMMEDIATE))) {
								int regnr;
								if (oi.source_info < 8){
									regnr = oi.source_info;
								} else {
									assert(oi.dest_info < 8);
									regnr = oi.dest_info;
								}

								int ok = 1;
								int k;
								for (k=ins_that_sets+1; k<j; k++){
									PRINT_DEBUG("the k=%d", k);
									struct opsinfo gg = get_instruction_register_usage(&lookahead[k], lookahead[k].next_instr - lookahead[k].cur_instr, 0);
									if (!gg.valid){
										PRINT_DEBUG("invalid");
										ok = 0;
										break;
									}
									if (gg.dest_info == regnr){
										PRINT_DEBUG("ggdi %d, regnr %d", gg.dest_info, regnr);
										ok = 0;
										break;
									}
								}

								if (ok){
									PRINT_DEBUG("CAN OPTIMIZE REGIMM");

									insert_from = ins_that_sets;
									insert_at = j;
									insert_len = lookahead[ins_that_sets].next_instr - lookahead[ins_that_sets].cur_instr;
									int r;
									for (r=0; r<insert_len; r++){
										insert_what[r] = *(char*)(lookahead[ins_that_sets].cur_instr+r);
									}

									// no need to save and restore explicitly any more
									saverest[i] &= (~EF_SAV);
									saverest[j] &= (~EF_RST);
									suppress[ins_that_sets] = 1;

									goto end_optimization;
								} else {
									PRINT_DEBUG("NOT OK ri");
								}
							}	else if (oi.valid &&
												 ((oi.dest_info == RG_IMMEDIATE && oi.source_info == RG_MEMORY) ||
												  (oi.dest_info == RG_MEMORY && oi.source_info == RG_IMMEDIATE))) {

								/*int mem_base_reg;
								if (oi.source_info == RG_MEMORY){

								} else {
									assert(oi.dest_info == RG_MEMORY);
									...
								}*/

								int ok = 1;
								int k;
								for (k=ins_that_sets+1; k<j; k++){
									PRINT_DEBUG("the k=%d", k);
									struct opsinfo gg = get_instruction_register_usage(&lookahead[k], lookahead[k].next_instr - lookahead[k].cur_instr, 0);
									if (!gg.valid){
										PRINT_DEBUG("invalid");
										ok = 0;
										break;
									}
									if (gg.dest_info == RG_MEMORY){
										PRINT_DEBUG("memory is written... we don't do further analysis for now");
										ok = 0;
										break;
									}
									if (gg.dest_info < 8 &&
											((1 << gg.dest_info) & oi.uses)){
										PRINT_DEBUG("writes to a register that is used!");
										ok = 0;
										break;
									}
								}

								if (ok){
									PRINT_DEBUG("CAN OPTIMIZE MEMIMM");

									insert_from = ins_that_sets;
									insert_at = j;
									insert_len = lookahead[ins_that_sets].next_instr - lookahead[ins_that_sets].cur_instr;
									int r;
									for (r=0; r<insert_len; r++){
										insert_what[r] = *(char*)(lookahead[ins_that_sets].cur_instr+r);
									}

									// no need to save and restore explicitly any more
									saverest[i] &= (~EF_SAV);
									saverest[j] &= (~EF_RST);
									suppress[ins_that_sets] = 1;

									goto end_optimization;
								} else {
									PRINT_DEBUG("NOT OK mi!");
								}
							} else if (oi.valid &&
												 ((oi.dest_info < 8 && oi.source_info == RG_MEMORY) ||
												  (oi.dest_info == RG_MEMORY && oi.source_info < 8))) {
								int regnr;
								if (oi.source_info < 8){
									regnr = oi.source_info;
								} else {
									assert(oi.dest_info < 8);
									regnr = oi.dest_info;
								}

								int ok = 1;
								int k;
								for (k=ins_that_sets+1; k<j; k++){
									PRINT_DEBUG("the k=%d", k);
									struct opsinfo gg = get_instruction_register_usage(&lookahead[k], lookahead[k].next_instr - lookahead[k].cur_instr, 0);
									if (!gg.valid){
										PRINT_DEBUG("invalid");
										ok = 0;
										break;
									}
									if (gg.dest_info == RG_MEMORY){
										PRINT_DEBUG("memory is written... we don't do further analysis for now");
										ok = 0;
										break;
									}
									if (gg.dest_info < 8 &&
											((1 << gg.dest_info) & oi.uses)){
										PRINT_DEBUG("writes to a register that is used!");
										ok = 0;
										break;
									}
									if (gg.dest_info == regnr){
										PRINT_DEBUG("register is overwritten");
										ok = 0;
										break;
									}
								}

								if (ok){
									PRINT_DEBUG("CAN OPTIMIZE MEMREG");

									insert_from = ins_that_sets;
									insert_at = j;
									insert_len = lookahead[ins_that_sets].next_instr - lookahead[ins_that_sets].cur_instr;
									int r;
									for (r=0; r<insert_len; r++){
										insert_what[r] = *(char*)(lookahead[ins_that_sets].cur_instr+r);
									}

									// no need to save and restore explicitly any more
									saverest[i] &= (~EF_SAV);
									saverest[j] &= (~EF_RST);
									suppress[ins_that_sets] = 1;

									goto end_optimization;
								} else {
									PRINT_DEBUG("NOT OK mr!");
								}
							} else {
								PRINT_DEBUG("COULDNT OPTIMIZE val %d, di %d si %d", oi.valid, oi.dest_info, oi.source_info);
							}
						} else {
							PRINT_DEBUG("COULDNT OPTIMIZE BECAUSE NOT CMP OR TEST");
						}
					} else {
						PRINT_DEBUG("not found...");
					}
				} else if (saverest[j] & EF_SAV){
					i = j; // advance
				}
			}
		}
		//print_disasm_instr(2 /* stderr */, &lookahead[i], );
	}
	PRINT_DEBUG("DONE OPTIMIZING SAVE/RESTORE PAIRS: insert_at=%d insert_len=%d\n", insert_at, insert_len);

#endif
}




/**
 * Starts disassembling at 'start' until the end of the basic block
 * is reached. The result is written in the array 'result' and the
 * number of translated instructions is written in 'resultsize'.
 * If the basic block is longer than maxresultsize then the disassembling
 * stops to avoid a buffer overflow.
 */
static void disassemble_basic_block(const struct translate* start,
																		int maxresultsize,
																		struct translate* result,
																		int* result_size)
{
	result[0] = *start;
	(*result_size) = 1;
	int end_lookahead = 0;
	while ((*result_size) < maxresultsize-1 && !end_lookahead) {
		PRINT_DEBUG("diasa");
		fbt_disasm_instr(&result[(*result_size)-1]);
		PRINT_DEBUG("dias done");

		int length = result[(*result_size)-1].next_instr - result[(*result_size)-1].cur_instr;
		PRINT_DEBUG("length is %d", length);

		//llprintf("disas done\n");
		// these are the instructions where one can continue in straight line...
		if (result[(*result_size)-1].cur_instr_info->opcode.handler == action_copy ||
				result[(*result_size)-1].cur_instr_info->opcode.handler == action_warn /* is just a special action_copy */ ||
				result[(*result_size)-1].cur_instr_info->opcode.handler == action_call ||
				result[(*result_size)-1].cur_instr_info->opcode.handler == action_jmp){

			result[(*result_size)] = result[(*result_size)-1];

			// unconditional jump...
			if (result[(*result_size)].cur_instr_info->opcode.handler == action_jmp){
				PRINT_DEBUG("its a jmp");
				unsigned jump_target = get_jump_target(result[(*result_size)].cur_instr, length);
			  void *transl_target = fbt_ccache_find(result[(*result_size)].tld, (void*)jump_target);
				if (transl_target == NULL){
					PRINT_DEBUG("not alr trans");
					result[(*result_size)].next_instr = (unsigned char*)jump_target;
				} else {
					PRINT_DEBUG("alr trans");
					end_lookahead = 1;
					(*result_size)--;
				}

			} else if (result[(*result_size)].cur_instr_info->opcode.handler == action_call){
				PRINT_DEBUG("its a call");

				const unsigned char *addr = result[(*result_size)].cur_instr;
				assert(length == 5 && !HAS_PREFIX(*addr));
				ulong_t call_target = *((ulong_t*) (addr + 1)) + (ulong_t) addr + length;
				unsigned char *next_instr = result[(*result_size)].next_instr;

				PRINT_DEBUG("original call_target: %p", (void*)call_target);
				// call only wants to get eip
				if (*((ulong_t*) (addr + 1)) == 0x0 && *(next_instr) >= 0x58 && *(next_instr) <= 0x5F) {
					// next instruction will be just the instruction after 'call'
					PRINT_DEBUG("just wants eip");
				} else {
					void *transl_target = fbt_ccache_find(result[(*result_size)].tld, (void*)call_target);
					if (transl_target == NULL){
						PRINT_DEBUG("not alr trans");
						result[(*result_size)].next_instr = (unsigned char*)call_target;
					} else {
						PRINT_DEBUG("alr trans");
						end_lookahead = 1;
						(*result_size)--;
					}
				}
			} else {
				PRINT_DEBUG("normally...");
			}

			(*result_size)++;
		} else {
			break;
		}
	}
	//(*result_size)--;
	PRINT_DEBUG("end... result size is %d ", (*result_size));
}

void examine_following_basic_blocks(const struct translate* last_instr_of_bb){
	void* following_bbs[2];
	int num_following_bbs; // will 1 or 2

	if (last_instr_of_bb->cur_instr_info->opcode.handler == action_jmp){
		int jmpinstrlen = last_instr_of_bb->next_instr - last_instr_of_bb->cur_instr;
		unsigned jmptarget = get_jump_target(last_instr_of_bb->cur_instr, jmpinstrlen);
		following_bbs[0] = (void*)jmptarget;
		num_following_bbs = 1;
	} else if (last_instr_of_bb->cur_instr_info->opcode.handler == action_jcc){
		unsigned jcctargets[2];
		get_conditional_jump_targets(last_instr_of_bb, &jcctargets[0]);

		following_bbs[0] = (void*)jcctargets[0];
		following_bbs[1] = (void*)jcctargets[1];
		num_following_bbs = 2;
	}

/*
  #ifdef ENABLE_UNSOUND_ASSUMPTIONS
	if (strcmp(lookahead[lookahead_nr].cur_instr_info->mnemonic, "ret") == 0){
		s = S_DONTNEEDFLAGS; // reasonable assumption, but not guaranteed !!!
	}
	if (strcmp(lookahead[lookahead_nr].cur_instr_info->mnemonic, "call") == 0){
		s = S_DONTNEEDFLAGS; // reasonable assumption, but not guaranteed !!!
	}
	if (lookahead[lookahead_nr].cur_instr_info->opcode.handler == action_jmp_indirect){
		s = S_DONTNEEDFLAGS; // reasonable assumption, but not guaranteed !!!
	}
#endif
*/
}


#define MAIN_BB_MAX_INSTRNS 256

void *fbt_lmem_translate_noexecute(struct thread_local_data *tld,	void *orig_address) {
	static char* allowed[] = {
				"mov"

				"add", "or",
				"adc", "sbb",
				"and",
				"daa",
				"sub",

				"das", "xor", "aaa", "cmp", "aas", "imul", "test",
				"sahf", "cmps", "cmpsb", "cmpsw", "cmpsd", "scas",
				"scasb", "scasw", "scasd", "rol", "ror", "rcl", "rcr",
				"shl", "sal", "sar", "shr", "aam", "amx", "aad", "adx",
				"ng", "div", "idiv", "vmread", "vmwrite", "bt", "shld",
				"shrd", "cmpxchg", "btr", "popcnt", "bt", "bts", "btr",
				"btc", "bsf", "bsr", "xadd", "vmptrld", "vmclear", "vmxon",
				"vmptrst", "vmcall", "vmlaunch", "vmresume", "vmxoff",

				"bound", "arpl", "sahf", "fucomip", "fcomip", "fucomi", "fcomi",
				"icebp", "cmc", "clc", "stc",
				"dec", "verr", "verw", "lar", "lsl", "ucomisd",
				"comiss", "comisd", "sysenter", "cmpxchg8b",

				"adc", "sbb", "daa",
				"das", "aaa",


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

	PRINT_DEBUG_FUNCTION_START("lmem_translate_noexecute(*tld=%p, *orig_address=%p)",
			tld, orig_address);

	assert(tld != NULL);

	/* if the target is already translated then we return the cached version  */
	void *already_translated = fbt_ccache_find(tld, orig_address);
	if (already_translated != NULL) {
		PRINT_DEBUG_FUNCTION_END("already translated -> %p",
				already_translated);
		return already_translated;
	}

	struct mem_info *code_block = tld->chunk;
	/* make sure that we don't translate translated code */
	while (code_block != NULL) {
		if ((orig_address >= code_block->ptr) &&
				(orig_address < (code_block->ptr + code_block->size))) {
			llprintf("Translating translated code: %p (%p len: 0x%x (%p) type: %d (syscall=%d))\n",
					orig_address, code_block->ptr, code_block->size, code_block,
					code_block->type, MT_SYSCALL_TABLE);
			fbt_suicide(255);
		}
		code_block = code_block->next;
	}

	/* we need to translate TU, add to ccache index,
     jump to the translated code */
	enum translation_state tu_state = NEUTRAL;

	int bytes_translated = 0;
	struct translate *ts = &(tld->trans);
	ts->next_instr = (unsigned char*)orig_address;

	/* check if more memory needs to be allocated for tcache */
	if ((long)(ts->code_cache_end - ts->transl_instr) < MAX_BLOCK_SIZE) {
		PRINT_DEBUG("Not enough memory for new code block - allocating more!");
		unsigned char *prev_transl_instr = ts->transl_instr;

		fbt_allocate_new_code_cache(tld);

		/* add a jmp connect old and new tcache memory blocks */
		if (prev_transl_instr != NULL) {
			JMP_REL32(prev_transl_instr, ts->transl_instr);
		}
	}
	PRINT_DEBUG("tld->ts.transl_instr: %p", ts->transl_instr);

	/* add entry to ccache index */
	fbt_ccache_add_entry(tld, orig_address, ts->transl_instr);

	/* look up address in translation cache index */
	void *transl_address = ts->transl_instr;

	int i;

	/*
	 * Disassemble the main basic block, where by main basic block
	 * we mean the basic block that we are going to translate in this
	 * function execution.
	 */
	int main_bb_ninstr = (-1);
	struct translate main_bb[MAIN_BB_MAX_INSTRNS];
	disassemble_basic_block(ts, MAIN_BB_MAX_INSTRNS, &main_bb[0], &main_bb_ninstr);
	//assert(main_bb_ninstr > 0);
	assert(main_bb_ninstr <= MAIN_BB_MAX_INSTRNS);


	/**
	 * To aid the analysis of the main basic block, it is helpful to
	 * know what state (flags, registers) the following basic blocks
	 * will need.
	 * If the last instruction of the main basic block is some sort of
	 * indirect jump we get no useful information.
	 * If the last instruction is a conditional jump then the info we
	 * get is the least common denominator of the two branches.
	 */
	unsigned next_bb_available_regs;
	int next_bb_flags_info;
	examine_following_basic_blocks(&main_bb[main_bb_ninstr-1]);

	/**
	 * Decode instructions in a friendly format
	 */
	struct lmem_instruction instructions[MAIN_BB_MAX_INSTRNS];
	assert(main_bb_ninstr < MAIN_BB_MAX_INSTRNS);
	for (i=0; i<main_bb_ninstr; i++){
		compute_lmem_instruction(&main_bb[i], &instructions[i]);
	}

	/**
	 * Compute which registers are available before instruction i.
	 * This helps us to insturment the code without having to save
	 * and restore registers all the time.
	 */
	unsigned available_regs[MAIN_BB_MAX_INSTRNS];
	assert(main_bb_ninstr < MAIN_BB_MAX_INSTRNS);
	lmem_compute_spilling(&instructions[0], main_bb_ninstr, &available_regs[0]);

	/**
	 * Find which instructions need instrumentation
	 */
	int needs_memcheck[MAIN_BB_MAX_INSTRNS];
	assert(main_bb_ninstr < MAIN_BB_MAX_INSTRNS);
	for (i=0; i<main_bb_ninstr; i++){
		needs_memcheck[i] = instructions[i].writes_memory || instructions[i].reads_memory;
	}

	/**
	 * For each instruction see if before the instruction is
	 * executed flags need to be saved and/or restored
	 */
	int saverest[MAIN_BB_MAX_INSTRNS];
	int dbg_flagsneeds[MAIN_BB_MAX_INSTRNS];
	assert(main_bb_ninstr < MAIN_BB_MAX_INSTRNS);
	analyze_flags_save_restore_needs(&instructions[0], &main_bb[0], &needs_memcheck[0], main_bb_ninstr, &saverest[0], &dbg_flagsneeds[0]);
	/**
	 * Debug output
	 */
	PRINT_DEBUG("\nBEIGN OF ANALYSYS OUTPUT\n");
	for (i=0; i<main_bb_ninstr; i++){
		//PRINT_DEBUG("available regs: %x", available_regs[i]);
#ifdef DEBUG
		print_disasm_inst(debugStream, &main_bb[i], main_bb[i].next_instr - main_bb[i].cur_instr);
#endif
		PRINT_DEBUG("");
		PRINT_DEBUG("isr "BYTETOBINARYPATTERN" ids "BYTETOBINARYPATTERN" iau "BYTETOBINARYPATTERN,
				BYTETOBINARY(main_bb[i].cur_instr_info->implSrcFlags),
				BYTETOBINARY(main_bb[i].cur_instr_info->implDestFlags),
				BYTETOBINARY(main_bb[i].cur_instr_info->implAuxFlags));
		//PRINT_DEBUG("memcheck %d, save %d, restore %d",	needs_memcheck[i],	saverest[i]&1, (saverest[i]&2)>>1);
#ifdef DEBUG
		char* fin[] = {"-", "use", "wall", "use&wall", "wsome", "...", ".....", "......."};
		assert(instructions[i].arithmetic_flag_info >=  0);
		assert(instructions[i].arithmetic_flag_info <= 7);
		PRINT_DEBUG("nmch=%d flgneeds=%d saverest=%d supported=%d flaginfo=%s gpread=%x gpwrite=%x rmem=%d wmem=%d",
				needs_memcheck[i],
				dbg_flagsneeds[i],
				saverest[i],
				instructions[i].supported,
				fin[instructions[i].arithmetic_flag_info],
				instructions[i].gp_registers_read,
				instructions[i].gp_registers_written,
				instructions[i].reads_memory,
				instructions[i].writes_memory);
#endif
	}
	PRINT_DEBUG("\nEND OF ANALYSYS OUTPUT\n");

	int instrnr = -1;

	/* we translate as long as we
			 - stay in the limit (MAX_BLOCK_SIZE)
			 - or if we have an open TU (could happen if we are translating a call or
				 so) */
	while (((bytes_translated < MAX_BLOCK_SIZE) && (tu_state == NEUTRAL)) ||
			(tu_state == OPEN)) {
		instrnr++;

		//assert(instrnr <= main_bb_ninstr);

		/* translate an instruction */

		/* Check if we are still within the boundaries of the current section.
				 Otherwise, do a complete check of the address to translate and
				 update the current section information. */

		fbt_disasm_instr(ts);

		unsigned char *old_transl_instr = ts->transl_instr;

	#ifdef ENABLE_MEMCHECKS
		if (saverest[instrnr] & EF_SAV){
			unsigned char *transl_instr = ts->transl_instr;
			BEGIN_ASM(transl_instr)
				SAVE_FLAGS
			END_ASM
			ts->transl_instr = transl_instr;
			PRINT_DEBUG("SAV");
		}

		int checkmem = 1;

		if (!checkmem && needs_memcheck[instrnr] && inlist(ts->cur_instr_info->mnemonic, allowed)){
			unsigned char *transl_instr = ts->transl_instr;
			BEGIN_ASM(transl_instr)
			//testl %ecx, %ecx // just mess around with flags
			END_ASM
			ts->transl_instr = transl_instr;
		} else if (needs_memcheck[instrnr] && inlist(ts->cur_instr_info->mnemonic, allowed)) {

			/* Switch to secure stack. Throughout, we follow the principle that we
			 * don't mess with the stack of the application as this could possibly
			 * interfere with exception management, etc. */
			unsigned char *transl_instr = ts->transl_instr;
			BEGIN_ASM(transl_instr)
			movl %esp, {tld->stack-1}
			movl ${tld->stack-1}, %esp
			END_ASM
			int no_need = 0;

			BEGIN_ASM(transl_instr)
			pushl %eax
			pushl %ecx
			END_ASM
			/*
			 * At this point we do the leal computation. It is important
			 * that the registers that leal uses for its address computation
			 * have still the original values.
			 * This is the case because the only register we modified is %esp,
			 * but %esp cannot be used (I looked at the modrm table of the
			 * intel doc) in address computations.
			 */
			no_need = sdbg_insert_leal(ts, &transl_instr); // puts address in eax
			BEGIN_ASM(transl_instr)
			movl %eax, %ecx // copy full address in ecx
			andl $0x1FFFFFF, %eax
			orl  $0x40000000, %eax
			movb (%eax), %al

			/* pusha
						call_abs {&dbgchk}
						popa */

			test %al, %al
			jz cont
			/*
			 * If we got here it is possible address has been marked as forbidden.
			 *
			 */
			pusha
			call_abs {&lmem_check_address}
			popa
			cont:
			popl %ecx
			popl %eax
			END_ASM

			/* go back to application stack */
			BEGIN_ASM(transl_instr)
			popl %esp
			END_ASM

			if (!no_need){
				PRINT_DEBUG("Memcheck inserted");
				ts->transl_instr = transl_instr;
			} else {
				//assert(0);
			}
		}

		if (saverest[instrnr] & EF_RST){
			unsigned char *transl_instr = ts->transl_instr;
			BEGIN_ASM(transl_instr)
				RESTORE_FLAGS
			END_ASM
			ts->transl_instr = transl_instr;
			PRINT_DEBUG("RST");
		}

	#endif

		tu_state = ts->cur_instr_info->opcode.handler(ts);
		PRINT_DEBUG("%d: '%s'", instrnr, ts->cur_instr_info->mnemonic);

		bytes_translated += (ts->transl_instr - old_transl_instr);
	}

	/*
	if (insert_at != -1){
		if (insert_from < instrnr && instrnr < insert_at){
			// the translation ended before we expected it!
			// let's fix it.

			PRINT_DEBUG("\n____ INSERTING HERE _______\n");
			int k;
			for (k=0; k<insert_len; k++){
				*ts->transl_instr = insert_what[k];
				ts->transl_instr++;
			}
		}
	}
	*/

	//if (instrnr>0)
	/*if (ss[instrnr] == S_NEED_FLAGS_IN_EFLAGS){
		// put them in memory, as the protocol requires

		unsigned char *transl_instr = ts->transl_instr;
		BEGIN_ASM(transl_instr)
			SAVE_FLAGS
		END_ASM
		ts->transl_instr = transl_instr;
		PRINT_DEBUG("SAVED FROM EFLAGS TO SAVED EFLAGS");
	}*/

	/* if the TU was finished because the number of instructions hit the limit, we
			 have to add glue code to the TU */
	if (tu_state != CLOSE) {
		if (tu_state != CLOSE_GLUE) {
			PRINT_DEBUG("finishing TU because instruction limit was hit, invoking"
					"translation function on %p", ts->next_instr);
		} else {
			PRINT_DEBUG("closing TU upon request, invoking translation function on"
					" %p", ts->next_instr);
		}
		/* takes care of the flag register! */
		/* write: jump to trampoline that does chaining if next block needs to be
				 translated or does fallthrough otherwise */
		struct trampoline *trampo = fbt_create_trampoline(tld,
				(void*)ts->next_instr,
				ts->transl_instr+1,
				ORIGIN_RELATIVE);
		JMP_REL32(ts->transl_instr, trampo->code);
	}

	/* make sure that we always stay in the limits, even if we overwrite the
			 MAX_BLOCK_SIZE due to some optimizations */
	assert(bytes_translated < TRANSL_GUARD);
	assert((void*)(ts->transl_instr) < (void*)(ts->code_cache_end +
			TRANSL_GUARD));

	PRINT_DEBUG_FUNCTION_END("-> %p,   next_tu=%p (len: %d)", transl_address,	ts->next_instr, bytes_translated);

	return transl_address;
}

enum syscall_auth_response lmem_syscall(struct thread_local_data *tld,
		ulong_t syscall_nr,
		ulong_t arg1,
		ulong_t arg2,
		ulong_t arg3 __attribute__((unused)),
		ulong_t arg4 __attribute__((unused)),
		ulong_t arg5 __attribute__((unused)),
		ulong_t *arg6 __attribute__((unused)),
		ulong_t is_sysenter __attribute__((unused)),
		ulong_t *retval __attribute__((unused))) {
	if (syscall_nr != 500){
		fbt_suicide_str("fbt_sdbg.c: wrong syscall number in lmem_syscall\n");
	}
	int ad = (int) arg2;
	ad &= 0x1FFFFFF;
	ad |= 0x40000000;
	if (arg1 == 0){
		//llprintf("Protecting memory at %p! (%p)\n", (void*)arg2, ((char*)ad));
		//*((char*)ad)=1;
	} else if (arg1 == 1){
		//llprintf("Unprotecting memory at %p! (%p)\n", (void*)arg2, ((char*)ad));
		//*((char*)ad)=0;
	} else {
		fbt_suicide_str("fbt_sdbg.c: unknown command in lmem_syscall\n");
	}
	return SYSCALL_AUTH_FAKE;
}


#endif // LMEM

