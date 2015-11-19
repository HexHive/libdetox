/*
 * translation cache index and memory allocation
 * 
 * at the moment, no memory is ever freed, so memory consumption continuously rises
 *
 */


#include "stm_settings.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef DEBUG
#include <stdio.h> /* debug */
#endif /* DEBUG */

#include "stm_tcache.h"

/* 
 * hashtable size: we use 20bit out of the 32bit of an address to perform a lookup in the hashtable.
 * 2^20 * 8 bytes (space for 2 32bit addresses) = 8MiB.
 * Has to be a power of 2.
 */
#define HASHTABLE_SIZE 0x800000
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
 */
#define HASH_PATTERN 0x007FFFF8

/* allocate chunks of 1MiB at a time */
#define ALLOC_SIZE 0x100000

/* association between addresses in TU and addresses in CCF */
void *hashtable;

/**
 * pointer to next free memory location in tcache. It is advanced when memory is used for CCFs
 */
void *tcache_ptr;

/**
 * pointer to byte after allocated memory. allocated memory remaining = tcache_end - tcache_ptr
 */
void *tcache_end;

/**
 * initialize the translation cache index (hashtable) and allocate the first chunk of memory for 
 * the translation cache.
 */
void tcache_init()
{
	/* create hashtable */
	hashtable = malloc(HASHTABLE_SIZE);
#ifdef DEBUG
	printf("hashtable begins at %#x\n", hashtable);
#endif /* DEBUG */
	memset(hashtable, 0x0, HASHTABLE_SIZE);
	
	/* allocate first chunk of memory */
	tcache_ptr = malloc(ALLOC_SIZE);	
	tcache_end = tcache_ptr + ALLOC_SIZE;
	
}

/**
 * find the translation cache index entry for the given TU address.
 * @param tu_address the address of the translation unit
 * @return the address of the translated version or 0 if not found
 */
void *tcache_find(void *tu_address)
{
	/* calculate offset into hashtable (so this instruction is our hash function) */
	uint32_t offset = ((uint32_t) tu_address << 1) & HASH_PATTERN;
	
	struct tcache_entry *entry = hashtable + offset;
	
	/* check entry if src address equals tu_address */
	while (entry->src != 0) {
		if (tu_address == entry->src) {
			/* return corresponding dest address */
			return entry->dst;
		}
		
		/* 
		 * We mustn't access memory beyond the hashtable!!
		 * Bitwise AND with (HASHTABLE_SIZE - 1) is the same as
		 * modulo HASHTABLE_SIZE.
		 */
		offset = (offset + sizeof(struct tcache_entry)) & (HASHTABLE_SIZE - 1);
		entry = hashtable + offset;
	}
	return NULL;
	
}


/**
 * Adds a translation cache index entry to the index and allocates memory for the 
 * compiled code fragment if required.
 * @param tu_address the address of the translation unit
 * @return a pointer to the start of the CCF
 */
void *tcache_put(void *tu_address)
{
	/* check if more memory needs to be allocated for tcache */
	if (tcache_end - tcache_ptr < MAX_CCF_SIZE) {
		tcache_ptr = malloc(ALLOC_SIZE);
		if (NULL == tcache_ptr)
			exit(1);
		tcache_end = tcache_ptr + ALLOC_SIZE;
	}
#ifdef DEBUG	
	printf("tcache_put: tcache_ptr: %#x\n", tcache_ptr);	/* debug */
#endif /* DEBUG */
	
	/* add entry to tcache index */
	tcache_add_entry(tu_address, tcache_ptr);
	
	/* update tcache_ptr, assuming maximum CCF size */
	void *result = tcache_ptr;
	tcache_ptr = tcache_ptr + MAX_CCF_SIZE;
	return result;
}

/**
 * adds an entry to the translation cache index.
 * This simply adds an entry tu_address -> ccf_address to the tcache index
 * @param tu_address the source address of the entry
 * @param ccf_address the destination address of the entry
 */
void tcache_add_entry(void *tu_address, void *ccf_address)
{
	/* calculate offset into hashtable that corresponds to this tu_address*/
	uint32_t offset = ((uint32_t) tu_address << 1) & HASH_PATTERN;
	struct tcache_entry *entry = hashtable + offset;
	
	/* search the hastable for a free position, beginning at offset */
	while (entry->src != 0) {
		offset = (offset + sizeof(struct tcache_entry)) & (HASHTABLE_SIZE - 1);
		entry = hashtable + offset;
	}
	
	/* insert entry into hashtable */
	entry->src = tu_address;
	entry->dst = ccf_address;
}


/**
 * reclaim reserved but unused memory after translating a TU.
 * Used to update tcache_ptr after the TU has been translated and we know, how much memory the CCF 
 * really uses.
 * @param ccf_address the address where the CCF begins
 * @param ccf_length the length of the CCF
 */
void tcache_reclaim(void *ccf_address, int ccf_length)
{
	if ((ccf_address + MAX_CCF_SIZE == tcache_ptr) && (ccf_length < MAX_CCF_SIZE)) {
		/* 
		 * CCF was shorter than maximum length, and tcache_ptr has not been advanced in between, 
		 * so we can reclaim the memory that was reserved but not used
		 */
		tcache_ptr -= MAX_CCF_SIZE - ccf_length;
	}
}


