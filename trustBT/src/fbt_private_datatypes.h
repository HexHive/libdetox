/**
 * This file defines all datatypes that are used throughout the bt
 * This header file must be included in all bt-c files so that the
 * relevant datatypes can be used
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
 *   Antonio Barresi <abarresi@student.ethz.ch>
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

#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "fbt_datatypes.h"
#include "fbt_tcache.h"
#include "fbt_rbtree.h"

#endif

/*
 * hashtable size: we use 20bit out of the 32bit of an address to perform a lookup in the hashtable.
 * 2^20 * 8 bytes (space for 2 32bit addresses) = 8MiB.
 * Has to be a power of 2.
 */
#define HASHTABLE_NRBITS 23
//#define HASHTABLE_SIZE 0x00800000
#define HASHTABLE_SIZE (0x1<<HASHTABLE_NRBITS)
#define HASHTABLE_MAXENTRIES (HASHTABLE_SIZE>>3)
/*
 * (i % HASHTABLE_SIZE) should be the same as (i & (HASHTABLE_SIZE - 1)), and the compiler is good at
 * precomputing constant expressions such as (HASHTABLE_SIZE - 1)
 */


/*
 * We take bits with exponents 2 to 21 (inclusive) of the source address to determine the position in
 * the hash table. Because an entry in the table is 8 bytes long, we can only address
 * locations in the table with 8 bytes granularity.
 * To achieve that, the address is shifted one bit to the left and ANDed with this pattern. This gives
 * us the offset into the table.
 * WARNING: code in fbt_asm_function.S depends on the layout of this definition!
 */
//#define HASH_PATTERN 0x007FFFF8
//#define HASH_PATTERN 0x000FFFF8
#define HASH_PATTERN ((HASHTABLE_SIZE-1)^0x7)

#define C_HASH_FUNCTION(addr)  ((addr<<3) & HASH_PATTERN)
#define C_HASH_LOWER_COL(addr) ((((addr<<3) & ~(HASH_PATTERN))>>3)&7)
#define C_HASH_UPPER_COL(addr) ((((addr<<3) & ~(HASH_PATTERN))>>20)&(-1^HASH_PATTERN))

#if !defined(_ASSEMBLER_)

/**
 * The p_thread_key used to store the thread local data
 */
pthread_key_t tld_key;

/**
 * The offset of the tld pointers pushed onto the stack from the
 * real address of the tld.
 * Is defined in libfastbt.c.
 */
extern void *fbt_random_tld_offset;

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
    void *cache_end;
    struct cache_free_list *next;
};
typedef struct cache_free_list cache_free_list_t;

#ifdef FBT_RET_CACHE
#define RET_CACHE_NR_ENTRIES 256
struct ret_cache_entry {
    void *ret_addr;
    void *backpatch;
    void *src;
    void *retprelude;
    struct ret_cache_entry *next;
};
typedef struct ret_cache_entry ret_cache_entry_t;
#endif

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

struct alloc_chunk {
    struct rb_node node;
    unsigned char flags;
    struct alloc_chunk *next;
    struct alloc_chunk *next_lockdown;
};

struct mem_alloc_data {
    int free_bytes;
    void *mem_ptr;
    int free_pers_bytes;
    void *pers_mem_ptr;

    /**
     * linked list; holds non-persistent allocated memory that will be freed
     * at the end of the thread.
     */
    struct alloc_chunk *chunks;
    /**
     * red-black tree; holds all chunks of allocated memory for fast lookup. It
     * actually contains elements of type struct alloc_chunk, which "inherits"
     * from struct rb_node by having one as its first field.
     */
    struct rb_node *chunks_tree;

#ifdef SECU_MPROTECT_IDS
    /**
     * chunks that need to be write protected before returning control to the
     * guest code.
     */
    struct alloc_chunk *lockdown_list;
#endif /* SECU_MPROTECT_IDS */
};

#ifdef FBT_IND_JUMP_MULTIPLE
#define JUMPTABLE_SIZE 512
/**
 * Linked list of jumptable base addresses and their jumptables in memory.
 */
struct jumptable_list {
	unsigned int orig_base;
	unsigned char **table_base;
	char index_reg;
	struct jumptable_list *next;
	unsigned char *fixup;
};
typedef struct jumptable_list jumptable_list_t;
#endif

#ifdef TRUST_MEMPROTECT
#define LOCKLIST_SIZE 10
/**
 * Linked list element for pages to be locked.
 */
struct lock_list {
	void* page;
        int nr_pages;
	unsigned char execflag;
	//struct lock_list *next;
};
typedef struct lock_list lock_list_t;
#endif

/**
 * This struct keeps all thread local information (with a circular dependency to
 * the corresponding translate_struct)
 * WARNING: code in fbt_asm_function.S depends on the layout of this structure!
 */
struct thread_local_data {
    void *hashtable;                    /* association between addresses in TU and addresses in CCF */
    void *tcache_end;                   /* pointer to byte after allocated memory. */
    trampoline_entry_t *trampoline;     /* the next free trampoline */
    cache_free_list_t *cache_list;      /* linked list of all allocated cache and trampoline blocks. */
    struct mem_alloc_data mem_alloc;    /* struct that keeps track of allocated memory */
    translate_struct_t ts;              /* the struct which is used to hold the data for translation */
    void *ind_jump_target;              /* variable to hold the address of translated indirect jump and ret targets */
    void *ret_jumpback_tramp;           /* address of a jump-back trampoline that is jumped to by overwriting the RIP */
    // remove or implement
    // void *translate_stack;              /* stack for all bt internal functions */
    // void *rip;                          /* return instruction pointer used for fbt -> prog transition */
    unsigned int rand_state;            /* state struct of the reentrant random function rand_r(3) */
    unsigned int *ind_jump_trampoline;  /* fast version of indirect jumps (target is pushed before, remaining code is here) */
    unsigned int *ind_call_trampoline;  /* fast version of indirect calls/rets (target is pushed before, remaining code is here) */
    void *next_map;                     /* address where we try next to allocate memory */
#ifdef FBT_RET_CACHE
    unsigned int *retcache;             /* cache of return addresses, set by the corresponding call instructions */
    unsigned int *retcache_jump;        /* small trampoline that defaults to an ind call dispatch */
    ret_cache_entry_t *untrans_calls;   /* call locations without a matching return target */
    ret_cache_entry_t *trans_calls;   /* call locations without a matching return target */
#endif
    
#ifdef FBT_RET_STACK
    ret_stack_entry_t *translated_call_stack;
    ret_stack_entry_t *translated_call_stack_end;
    ret_stack_entry_t *translated_call_stack_tos;
#endif
#ifdef FBT_IND_JUMP_MULTIPLE
    jumptable_list_t *jumptable_list; /* the jumptable linked list for the switch jumptable optimization */
#endif
#ifdef TRUST_MEMPROTECT
    struct lock_list *locklist;
#endif
};

#endif // language c

/*
 * Define structure offset constants for use in assembler.
 */
// #define OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR 0
// #define OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR 4
//
// #define OFFSETOF_TS_TRANSL_INSTR 16
//
// /* do not change this value, asm depends on it! */
// #define OFFSETOF_TLD_HASHTABLE 0
//
// #define OFFSETOF_TLD_TS 16
// #define OFFSETOF_TLD_TS_TRANSL_INSTR (OFFSETOF_TLD_TS + OFFSETOF_TS_TRANSL_INSTR)
//
// #if  defined(FBT_INLINE_CALLS)
// #define OFFSETOF_TLD_TRANSLATED_CALL_STACK_END 56
// #define OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS 60
// #else
// #define OFFSETOF_TLD_TRANSLATED_CALL_STACK_END 52
// #define OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS 56
// #endif
//
// #define OFFSETOF_TCACHE_ENTRY_SRC 0
// #define OFFSETOF_TCACHE_ENTRY_DST 4
//
// #define SIZEOF_HASHTABLE_ENTRY 8

// #if !defined(_ASSEMBLER_)

/*
 * Check the correctness of the structure offset constants at compile time by the C compiler.
 */
// #define static_assert(cond)  do { enum { static_assert_failed = 1/(cond) }; } while (0)

// inline static void static_assert_structure_offsets() {
// #ifdef FBT_RET_STACK
// 	static_assert(OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR == offsetof(struct ret_stack_entry, transl_addr));
// 	static_assert(OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR == offsetof(struct ret_stack_entry, orig_addr));
// 	static_assert(OFFSETOF_TLD_TRANSLATED_CALL_STACK_END == offsetof(struct thread_local_data,
// 	                           translated_call_stack_end));
// 	static_assert(OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS == offsetof(struct thread_local_data,
//                                    translated_call_stack_tos));
// #endif
// 	static_assert(OFFSETOF_TLD_TS == offsetof(struct thread_local_data, ts));
// 	static_assert(OFFSETOF_TLD_HASHTABLE == offsetof(struct thread_local_data, hashtable));
//
// 	static_assert(OFFSETOF_TS_TRANSL_INSTR == offsetof(translate_struct_t, transl_instr));
//
// 	static_assert(OFFSETOF_TCACHE_ENTRY_SRC == offsetof(struct tcache_entry, src));
// 	static_assert(OFFSETOF_TCACHE_ENTRY_DST == offsetof(struct tcache_entry, dst));
// }
//
// #endif // language c
#endif
