/**
 * This is the tcache module, where all translated code snipplets are put
 * including the hashtable which maps between translated and untranslated
 * code (e.g. to find the already translated block in the code cache)
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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "fbt_private_datatypes.h"
#include "fbt_tcache.h"
#include "fbt_debug.h"
#include "fbt_statistic.h"
#include "fbt_asm_macros.h"
#include "fbt_mem_alloc.h"
#include "fbt_mem_protection.h"
#include "fbt_trampoline.h"

/**
 * allocates one chunk of memory for the translation cache.
 * Allocates one chunk of memory of size ALLOC_SIZE and sets the transl_instr pointer to the beginning of the chunk.
 * @param tld state of current translator thread
 */
inline void allocate_tcache(thread_local_data_t *tld)
{
    PRINT_DEBUG_FUNCTION_START("tcache_alloc(tld=%p)", tld);
    
    /* allocate tcache (page aligned) */
    tld->ts.transl_instr = fbt_lalloc(tld, ALLOC_PAGES);
    tld->tcache_end = tld->ts.transl_instr + ALLOC_SIZE - CCF_GUARD;
    
    if (tld->ts.transl_instr == NULL) {
	perror("tcache alloc failed");
		exit(1);
    }
    //     INFO_LLPRINTF("allocated tcache at %p\n", tld->ts.transl_instr);
    
    PRINT_INFO("allocated tcache: %p -> %p", tld->ts.transl_instr, tld->tcache_end);
    
    /* fill tcache with illegal HLT instructions */
#if defined(DEBUG) || !defined(NDEBUG)
    memset(tld->ts.transl_instr, 0xF4, ALLOC_SIZE);
#endif
    /* lets make our code-buffer executable */
    // 	if (mprotect(tld->ts.transl_instr, ALLOC_SIZE, (PROT_READ | PROT_WRITE | PROT_EXEC))!=0) {
    // 		perror("Could not setup code buffer");
    // 		exit(1);
    // 	}
    fbt_ids_setexec(tld, tld->ts.transl_instr);

    PRINT_DEBUG_FUNCTION_END(" ");
}


/**
 * initialize translation cache and index.
 * Initialize the translation cache index (hashtable) and allocate the first
 * chunk of memory for the translation cache.
 * @return state of current translator thread
 */
thread_local_data_t* tcache_init()
{
    thread_local_data_t *tld;
    
    PRINT_DEBUG_FUNCTION_START("tcache_init()");
    
    /* allocate memory for thread local data struct */
    if ((tld = (thread_local_data_t*) malloc(sizeof(thread_local_data_t))) == NULL) {
	perror("malloc: no free memory (tcache_init)!");
	exit(1);
    }
    
    tld->trampoline = NULL;
    tld->cache_list = NULL;
    tld->ts.tld = tld;
    tld->ts.app_data = NULL;
#if defined(FBT_INLINE_CALLS)
    tld->ts.inlined_frames = NULL;
#endif
    tld->mem_alloc.free_bytes = 0;
    tld->mem_alloc.free_pers_bytes = 0;
    tld->mem_alloc.chunks = NULL;
    tld->mem_alloc.chunks_tree = NULL;
#ifdef SECU_MPROTECT_IDS
    tld->mem_alloc.lockdown_list = NULL;
#endif
#ifdef FBT_IND_JUMP_MULTIPLE
    tld->jumptable_list = NULL;
#endif

    /* initialize state for rand_r */
    struct timeval tv;
    if (-1 == gettimeofday(&tv, NULL)) {
        perror("Could not use time as rand seed");
    }
    tld->rand_state = (unsigned int) tv.tv_usec;
    tld->next_map = (void*) (rand_r(&(tld->rand_state)) & (~(sysconf(_SC_PAGESIZE) - 1)));

    /* allocate memory for hashtable */
    tld->hashtable = fbt_lalloc(tld, HASHTABLE_SIZE / sysconf(_SC_PAGESIZE) + 1); /* +4 for 0x1 guard for tcache_find_fast asm function */

    /* initialize hashtable as empty */
    PRINT_INFO("allocated hashtable: %p -> %p", tld->hashtable, tld->hashtable+HASHTABLE_SIZE);
    memset(tld->hashtable, 0x0, HASHTABLE_SIZE+4); /* +4 for NULL guard for tcache_find_fast asm function */
    *(int*)((int)(tld->hashtable)+HASHTABLE_SIZE) = 0x1; /* guard for find_fast-wraparound */
    
    /* allocate first chunk of memory */
    allocate_tcache(tld);
    
#ifdef FBT_RET_STACK
    tld->translated_call_stack = fbt_smalloc(tld,
					     (FBT_RET_STACK_SIZE + 1) * sizeof(cache_free_list_t));
    tld->translated_call_stack_end = tld->translated_call_stack + FBT_RET_STACK_SIZE;
    tld->translated_call_stack_tos = tld->translated_call_stack_end;
    /* empty stack contains guard element */
    tld->translated_call_stack_tos->orig_addr = NULL;
    tld->translated_call_stack_tos->transl_addr = NULL;
#endif

    
    /* TODO: REMOVE? (or implement)
       4k stack for all fastbt functions */
    // tld->translate_stack = fbt_lalloc(tld, 1);

    /* initialize ind jump trampoline */
    create_indjump_fast_trampoline(tld);
    create_indcall_fast_trampoline(tld);
#ifdef FBT_RET_CACHE
    create_ret_cache(tld);
    tld->untrans_calls = NULL;
    tld->trans_calls = NULL;
#endif
    PRINT_DEBUG_FUNCTION_END("-> %p", tld);
    return tld;
}

/**
 * find the translation cache index entry for the given TU address.
 * @param tu_address the address of the translation unit
 * @return the address of the translated version or 0 if not found
 */
void *tcache_find(thread_local_data_t *tld, void *tu_address)
{
	PRINT_DEBUG_FUNCTION_START("tcache_find(*tld=%p, *tu_address=%p)", tld, tu_address);

	assert(tld == (thread_local_data_t*) pthread_getspecific(tld_key));

	/* calculate offset into hashtable (so this instruction is our hash function) */
	int32_t offset = C_HASH_FUNCTION((int32_t) tu_address);
	
	int32_t pos = 0;
	
	tcache_entry_t *entry = tld->hashtable + offset;

#if defined(FBT_STATISTIC)
	fbt_nr_tcache_slow_lookups++;
#endif

	/* check entry if src address equals tu_address */
	while (entry->src != 0) {
		if (tu_address == entry->src) {
			/* return corresponding dest address */
			PRINT_DEBUG_FUNCTION_END("-> %p", entry->dst);
			assert(entry->dst!=NULL);
			if (pos!=0) {
			    /* not optimal entry! replace! */
			    void *tmp;
			    tcache_entry_t *firstentry = tld->hashtable + C_HASH_FUNCTION((int32_t) tu_address);
			    tmp = firstentry->src;
			    firstentry->src = entry->src;
			    entry->src = tmp;
			    tmp = firstentry->dst;
			    firstentry->dst = entry->dst;
			    entry->dst = tmp;
			    entry = firstentry;
			}
			return entry->dst;
		}
		/*
		* We mustn't access memory beyond the hashtable!!
		* Bitwise AND with (HASHTABLE_SIZE - 1) is the same as
		* modulo HASHTABLE_SIZE.
		*/
		offset = (offset + sizeof(tcache_entry_t)) & (HASHTABLE_SIZE-1);
		pos ++;
		entry = tld->hashtable + offset;
	}

	PRINT_DEBUG_FUNCTION_END("-> %p", NULL);
	return NULL;

}


/**
 * Adds a translation cache index entry to the index and allocates memory for the
 * compiled code fragment if required.
 * @param tld state of current translator thread
 * @param tu_address the address of the translation unit
 */
void tcache_put(thread_local_data_t *tld, void *tu_address)
{
    PRINT_DEBUG_FUNCTION_START("tcache_put(*tld=%p, *tu_address=%p)", tld, tu_address);
    /* check if more memory needs to be allocated for tcache */
    if ((tld->tcache_end - (void*)tld->ts.transl_instr) < MAX_CCF_SIZE) {
	PRINT_DEBUG("Not enough memory for tcache index - allocating more!");
	unsigned char *prev_transl_instr = tld->ts.transl_instr;

	allocate_tcache(tld);

	/* add a jmp connect old and new tcache memory blocks */
	JMP_REL32(prev_transl_instr, (int32_t)tld->ts.transl_instr);
    }
    PRINT_DEBUG("tld->ts.transl_instr: %p", tld->ts.transl_instr);

    /* add entry to tcache index */
    tcache_add_entry(tld, tu_address, tld->ts.transl_instr);

    PRINT_DEBUG_FUNCTION_END(" ");
}

/**
 * adds an entry to the translation cache index.
 * This simply adds an entry tu_address -> ccf_address to the tcache index
 * @param tld state of current translator thread
 * @param tu_address the source address of the entry
 * @param ccf_address the destination address of the entry
 */
void tcache_add_entry(thread_local_data_t *tld, void *tu_address, void *ccf_address)
{
	PRINT_DEBUG_FUNCTION_START("tcache_add_entry(*tld=%p, *tu_address=%p, *ccf_address=%p)", tld, tu_address, ccf_address);
	/* calculate offset into hashtable that corresponds to this tu_address*/
	int32_t offset = C_HASH_FUNCTION((int32_t) tu_address);
	tcache_entry_t *entry = tld->hashtable + offset;

	int count = 0;
#ifdef FBT_INLINE_CALLS
	if (tld->ts.inlined_frames!=NULL) {
	    PRINT_DEBUG("We are currently inlining, it would be a bad idea to add this location to the lookup-table");
	    PRINT_DEBUG_FUNCTION_END(" ");
	    return;
	}
#endif /* FBT_INLINE_CALLS */
#if defined(FBT_STATISTIC)
	fbt_nr_ccf++;
#endif
	/* search the hastable for a free position, beginning at offset */
	while (entry->src != 0) {
	        /*if (offset > HASHTABLE_SIZE-1-sizeof(tcache_entry_t)) {
		    printf("Last entry - wrap around %p -> %p is: (0x%x)\n", tu_address, ccf_address, *(int*)(tld->hashtable+offset));
		}*/
		offset = (offset + sizeof(tcache_entry_t)) & (HASHTABLE_SIZE - 1);
		entry = tld->hashtable + offset;
		count++;
		if (count>=HASHTABLE_MAXENTRIES/10) {
		  printf("ERROR: no more space in hashtable (fbt_tcache.c)\n");
		  exit(1);
		}
	}

#if defined(FBT_STATISTIC)
	switch (count) {
	case 0: fbt_nr_tcache_direct++; break;
	case 1: fbt_nr_tcache_1++; break;
	case 2: fbt_nr_tcache_2++; break;
	case 3:
	case 4: fbt_nr_tcache_4++; break;
	case 5:
	case 6:
	case 7:
	case 8: fbt_nr_tcache_8++; break;
	default: fbt_nr_tcache_8p++; printf("Target is far away in hashtable: %d (%p)\n", count, tu_address);

	}
#endif
	/* insert entry into hashtable */
	entry->src = tu_address;
	entry->dst = ccf_address;
	DUMP_JMP_TABLE_ENTRY(tu_address, ccf_address);
	PRINT_DEBUG_FUNCTION_END(" ");
}

/* frees all tcache memory */
void tcache_free(thread_local_data_t *tld)
{
  assert(tld!=NULL);
	cache_free_list_t *blocks = tld->cache_list;

  /* iterates through all allocated blocks */
  /* blocks include tcache blocks and trampoline blocks */
	while (blocks!=NULL) {
	  cache_free_list_t *cur = blocks;
	  free(blocks->cache_block);
	  blocks = blocks->next;
	  free(cur);
	}

	/* free the hashtable as well */
	free(tld->hashtable);
  tld->hashtable = NULL;

  free(tld);
}


