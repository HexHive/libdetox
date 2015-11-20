/**
 * @file fbt_mem_mgmt.c
 * Implementation of memory pools for fine-grained memory management,
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

#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "fbt_mem_pool.h"
#include "fbt_libc.h"

#include <assert.h>
#include <asm-generic/mman.h>


static void mem_pool_map(struct mem_pool *mp, long size);
static void mem_pool_add_info_node(struct mem_pool *mp, char *mem, long size);

void fbt_mem_pool_init(struct mem_pool *mp) {
  mp->size = 0;
  mp->mem = NULL;
  mp->info_size = 0;
  mp->info_mem = 0;
  mp->info = NULL;
  
  #if defined(DEBUG)
  mp->allocated = 0;
  #endif /* DEBUG */

  mem_pool_map(mp, mp->allocation_size);
}

struct mem_pool *fbt_mem_pool_bootstrap(struct mem_pool *init_pool) {
  fbt_mem_pool_init(init_pool);

  struct mem_pool *result = (struct mem_pool *)fbt_mem_pool_allocate(
    init_pool,
    sizeof(struct mem_pool)
  );

  *result = *init_pool;  
  return result;
}

static void mem_pool_add_info_node(struct mem_pool *mp, char *mem, long size) {
  assert(mp->info_size >= sizeof(struct mem_pool_info));

  struct mem_pool_info *new_node = (struct mem_pool_info *)mp->info_mem;
  new_node->next = mp->info;
  new_node->mem = mem;
  new_node->size = size;

  mp->info_mem += sizeof(struct mem_pool_info);
  mp->info_size -= sizeof(struct mem_pool_info);

  mp->info = new_node;  

  #if defined(DEBUG)
  mp->allocated += size;
  #endif /* DEBUG */
}

static void mem_pool_map(struct mem_pool *mp, long size) {
  /* Allocate enough storage */
  long alloc_size = NRPAGES(size)*PAGESIZE;
  void *retval;
  fbt_mmap(NULL, alloc_size, mp->flags, MAP_PRIVATE|MAP_ANONYMOUS,  \
           -1, 0, retval, "BT failed to allocate memory (fbt_mem_pool_init: "
           "fbt_mem_mgmt.c)\n");  
  mp->mem = retval;
  mp->size = alloc_size;

  /* We also need to store the current memory info node, so we might to have to
   * allocate some more memory */
  if (mp->info_size < 2 * sizeof(struct mem_pool_info)) {
    mp->info_size = 16 * sizeof(struct mem_pool_info);
    mp->info_size = NRPAGES(mp->info_size) * PAGESIZE;
    fbt_mmap(NULL, mp->info_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,  \
             -1, 0, retval, "BT failed to allocate memory (fbt_mem_pool_init: "
             "fbt_mem_mgmt.c)\n");      
    mp->info_mem = retval;

    /* store information about the newly acquired information node */
     mem_pool_add_info_node(mp, mp->info_mem, mp->info_size);
  }

  /* Store the newly allocated pages */
  mem_pool_add_info_node(mp, mp->mem, mp->size);
}

void *fbt_mem_pool_allocate(struct mem_pool *mp, unsigned long size) {
  /* Make sure there still is enough memory */
  if (mp->size < size) {
    mem_pool_map(mp, size > mp->allocation_size ? size : mp->allocation_size);
  }
  void *result = mp->mem;
  mp->size -= size;
  mp->mem += size;
  return result;
}

void fbt_mem_pool_free(struct mem_pool *mp) {
  #if defined(DEBUG)
  long remaining_bytes = mp->allocated;
  #endif /* DEBUG */

  mp->size = 0;
  mp->mem = NULL;
  mp->info_size = 0;
  mp->info_mem = NULL;

  /* We have to start at the last item so we do not free memory we are using
   * for our iteration */
  struct mem_pool_info *current = mp->info;
  while (current != NULL) {
    /* Store next pointer, as we might be deallocating the current node */
    struct mem_pool_info *next = current->next;

    #if defined(DEBUG)
    remaining_bytes -= current->size;
    #endif /* DEBUG */

    long ret;
    fbt_munmap(current->mem,
               current->size,
               ret,
               "BT failed to deallocate memory "
               "(fbt_mem_pool_free: fbt_mem_mgmt.c)\n");
    current = next;
  }

  #if defined(DEBUG)
  assert(remaining_bytes == 0);
  #endif /* DEBUG */
}
