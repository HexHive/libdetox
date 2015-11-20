/**
 * @file fbt_mem_pool.h
 * Implementation of memory pools for fine-grained memory management
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 1134 $
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
#ifndef FBT_MEM_POOL_H
#define FBT_MEM_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fbt_datatypes.h"

struct mem_pool_info;
struct mem_pool;

/** Initializes a memory pool at the location pointed to by <code>mp</code>
  * 
  * @param mp The memory pool to initialize. Must have <code>allocation_size</code>
  *           and <code>flags</code> already initialized.
  */
void fbt_mem_pool_init(struct mem_pool *mp);

/** Initializes a memory pool with the settings in <code>mp</code>, but
  * bootstraps the memory pool to use memory from itself to store
  * a copy of the mem_pool structure.
  *
  * This function allows the creation of bootstrapped memory pools from
  * e.g. stack memory that will then be completely removed when being
  * freed using <code>fbt_mem_pool_free</code>
  *
  * @param mp Temporary memory pool settings that will be used for the boostrapped
  *           memory pool
  * @return Pointer to the memory pool descriptor in memory allocated within
  *         the same memory pool
  */
struct mem_pool *fbt_mem_pool_bootstrap(struct mem_pool *mp);

/** Allocates a chunk of memory
  * 
  * @param size Number of bytes to allocate
  * @return Pointer to the start of memory region
  */
void *fbt_mem_pool_allocate(struct mem_pool *mp, unsigned long size);

/** Frees a thread pool with all its allocated pages */
void fbt_mem_pool_free(struct mem_pool *mp);

/** A memory pool allows allocating of memory chunks of arbitrary size. It therefore 
  * can be used like both fbt_smalloc and fbt_malloc. Memory pools group together
  * memory that is used for a specific task and is not stored in a tld's memory 
  * chunk list and instead is freed explicitly by calling mem_pool_free.
  * */
struct mem_pool {
  /** Number of bytes to be allocated when new memory has to be requested */
  unsigned long allocation_size;

  /** Number of bytes remaining */
  unsigned long size;

  /** Pointer to start of remaining memory */
  char *mem;

  /** Flags used when allocating new memory */
  long flags;

  /** Number of bytes remaining in the mem_pool_info storage */
  unsigned long info_size;

  /** Pointer to start of mem_pool_info storage */
  char* info_mem;

  /** Information about allocated memory */
  struct mem_pool_info *info;

#if defined(DEBUG)
  /** In debug mode, we keep track of the number of bytes allocated in total
    * to check that all memory has been correctly released */
  long allocated;
 #endif /* DEBUG */
};

struct mem_pool_info {
  /**Pointer to start of memory region allocated using fbt_mmap */
  void *mem;
  /** Length of memory region */
  long size;
  /** Pointer to next allocated chunk */
  struct mem_pool_info *next;
};

#ifdef __cplusplus
}
#endif
#endif  /* FBT_MEM_MGMT_H */
