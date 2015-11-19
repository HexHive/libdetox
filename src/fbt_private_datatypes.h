/**
 * This file defines all datatypes that are used throughout the bt
 * This header file must be included in all bt-c files so that the
 * relevant datatypes can be used
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
#ifndef FBT_PRIVATE_DATATYPES_H
#define FBT_PRIVATE_DATATYPES_H

#if !defined(_ASSEMBLER_)

#include <stddef.h>
#include "fbt_datatypes.h"
#include "fbt_tcache.h"

#endif

/*
 * hashtable size: we use 20bit out of the 32bit of an address to perform a lookup in the hashtable.
 * 2^20 * 8 bytes (space for 2 32bit addresses) = 8MiB.
 * Has to be a power of 2.
 */
#define HASHTABLE_SIZE 0x00800000
//#define HASHTABLE_SIZE 0x00100000
/*
 * (i % HASHTABLE_SIZE) should be the same as (i & (HASHTABLE_SIZE - 1)), and the compiler is good at
 * precomputing constant expressions such as (HASHTABLE_SIZE - 1)
 */


/*
 * We take bits with exponents 2 to 21 (inclusive) of the source address to determine the position in
 * the hash table. Because an entry in the table is 8 bytes long, we can should only be able to address
 * locations in the table with 8 bytes granularity.
 * To achieve that, the address is shifted one bit to the left and ANDed with this pattern. This gives
 * us the offset into the table.
 * WARNING: code in fbt_asm_function.S depends on the layout of this definition!
 */
#define HASH_PATTERN 0x007FFFF8
//#define HASH_PATTERN 0x000FFFF8

#define C_HASH_FUNCTION(addr)  ((addr<<2) & HASH_PATTERN)

#if !defined(_ASSEMBLER_)

/**
 * The p_thread_key used to store the thread local data
 */
pthread_key_t tld_key;

/**
 * This is a free entry in the trampoline table
 * Trampolines are used for jump-conditionals and are recycled after the
 * target has been translated.
 * All free trampolines are kept in this trampoline-free-list
 */
struct trampoline_entry {
	struct trampoline_entry *next;
	uint32_t allocMem[4];
};
typedef struct trampoline_entry trampoline_entry_t;

/**
 * Linked list of allocated memory locations that should be freed when
 * we flush the translation cache
 */
struct cache_free_list {
	void *cache_block;
	struct cache_free_list *next;
};
typedef struct cache_free_list cache_free_list_t;

#ifdef FBT_RET_STACK
/**
 * Shadow stack entry: suggests the translated return address
 *  also provides the original return address to allow checking for correctness
 *  and synchronizing with the real return address stack.
 * WARNING: code in fbt_asm_function.S depends on the layout of this structure!
 */
struct ret_stack_entry {
	void *transl_addr;
	void *orig_addr;
};
typedef struct ret_stack_entry ret_stack_entry_t;
#endif

/**
 * This struct keeps all thread local information (with a circular dependency to
 * the corresponding translate_struct)
 * WARNING: code in fbt_asm_function.S depends on the layout of this structure!
 */
struct thread_local_data {
	void *hashtable;                   /* association between addresses in TU and addresses in CCF */
	void *tcache_end;                  /* pointer to byte after allocated memory. */
	trampoline_entry_t *trampoline;    /* the next free trampoline */
	cache_free_list_t *cache_list;     /* linked list of all allocated cache and trampoline blocks. */
	translate_struct_t ts;             /* the struct which is used to hold the data for translation */

#ifdef FBT_RET_STACK
	ret_stack_entry_t *translated_call_stack;
	ret_stack_entry_t *translated_call_stack_end;
	ret_stack_entry_t *translated_call_stack_tos;	
#endif
};

#endif // language c

/*
 * Define structure offset constants for use in assembler.
 */
#define OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR 0
#define OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR 4

#define OFFSETOF_TS_TRANSL_INSTR 16

/* do not change this value, asm depends on it! */
#define OFFSETOF_TLD_HASHTABLE 0

#define OFFSETOF_TLD_TS 16
#define OFFSETOF_TLD_TS_TRANSL_INSTR (OFFSETOF_TLD_TS + OFFSETOF_TS_TRANSL_INSTR)
#define OFFSETOF_TLD_TRANSLATED_CALL_STACK_END 52
#define OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS 56

#define OFFSETOF_TCACHE_ENTRY_SRC 0
#define OFFSETOF_TCACHE_ENTRY_DST 4

#define SIZEOF_HASHTABLE_ENTRY 8

#if !defined(_ASSEMBLER_)

/*
 * Check the correctness of the structure offset constants at compile time by the C compiler.
 */
#define static_assert(cond)  do { enum { static_assert_failed = 1/(cond) }; } while (0)

inline static void static_assert_structure_offsets() {
#ifdef FBT_RET_STACK
	static_assert(OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR == offsetof(struct ret_stack_entry, transl_addr));
	static_assert(OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR == offsetof(struct ret_stack_entry, orig_addr));
	static_assert(OFFSETOF_TLD_TRANSLATED_CALL_STACK_END == offsetof(struct thread_local_data,
	                           translated_call_stack_end));
	static_assert(OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS == offsetof(struct thread_local_data,
                                   translated_call_stack_tos));
#endif
	static_assert(OFFSETOF_TLD_TS == offsetof(struct thread_local_data, ts));
	static_assert(OFFSETOF_TLD_HASHTABLE == offsetof(struct thread_local_data, hashtable));

	static_assert(OFFSETOF_TS_TRANSL_INSTR == offsetof(translate_struct_t, transl_instr));

	static_assert(OFFSETOF_TCACHE_ENTRY_SRC == offsetof(struct tcache_entry, src));
	static_assert(OFFSETOF_TCACHE_ENTRY_DST == offsetof(struct tcache_entry, dst));
}

#endif // language c
#endif
