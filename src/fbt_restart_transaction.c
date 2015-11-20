/**
 * @file fbt_restart_transaction.c
 * Code dealing with restarting a transaction, which resets the 
 * thread state such that translation starts anew.
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

#include "fbt_restart_transaction.h"

#if defined(TRACK_CFTX)

#include "fbt_code_cache.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "fbt_mem_pool.h"
#include "fbt_translate.h"
#include "libfastbt.h"

#if defined(ONLINE_PATCHING)
#include "patching/fbt_patching.h" /* fbt_online_patching_refresh */
#endif /* ONLINE_PATCHING */

struct ccache_entry {
  ulong_t *src;
  ulong_t *dst;
};

static void cftx_handler(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("cftx_handler(tld=%p)", tld);
  PRINT_DEBUG("Interrupted %x at %x\n", tld, (tld->interrupted_cft ? tld->interrupted_cft->original : 0));

  /* Wait for calling thread to finish overwriting control flow transfers */
  fbt_mutex_lock(&tld->interrupted_cft_mutex);

  // Fix all ind->target's in trampolines
  fbt_fix_trampolines_after_interrupt_cftx(tld);

  #if defined(TRACK_CFTX)
  /* Clean thread tracking: we can still free memory by clearing memory
   * pool directly */
  tld->control_flow_transfers = NULL;
  #endif /* TRACK_CFTX */

  fbt_mutex_unlock(&tld->interrupted_cft_mutex);

  /* Full memory barrier */
  __sync_synchronize();

  
  fbt_mutex_unlock(&tld->interrupt_cft_mutex);

  /* Starting from this point, the patching has to request a fresh restart
   * of this thread if it wants to guarantee that it has the most recent 
   * patching information */

  /* Retranslate ind_target */
  void *original = NULL;
  if (tld->interrupted_cft == NULL) {
    PRINT_DEBUG("ind_target = %x\n", tld->ind_target);
    original = fbt_ccache_find_reverse(tld, tld->ind_target);
    PRINT_DEBUG("original = %x\n", original);
  }


  #if defined(ONLINE_PATCHING)
  /* Copy current patching information */
  fbt_online_patching_refresh(tld);
  #endif /* ONLINE_PATCHING */

  /* Clear the mapping table */
  int pos;
  for (pos = 0; pos < MAPPINGTABLE_SIZE; ++pos) {
    struct ccache_entry *entry = tld->mappingtable + pos;
    entry->src = 0;
    entry->dst = 0;
  }  

  /* Release pages allocated for code cache */
  /* we could walk through the tld pages and remove all that are of type
   * MT_CODE_CACHE, but this would be kind of messy. By switching to 
   * using mem pools for the code cache, this could be simplified */

  fbt_allocate_new_code_cache(tld);

  /* Reinitialize transaction */
  fbt_transaction_init(tld, tld->shared_data->commit_function);

  if (tld->interrupted_cft != NULL) {
    original = tld->interrupted_cft->original;
  } 

  #if defined(TRACK_CFTX)
  /* Copy settings of old memory pool and free it */
  struct mem_pool settings = *tld->cft_trampoline_mem_pool;
  fbt_mem_pool_free(tld->cft_trampoline_mem_pool);

  /* Boostrap new memory pool with same settings */  
  tld->cft_trampoline_mem_pool = fbt_mem_pool_bootstrap(&settings);
  #endif /* TRACK_CFTX */
  
  /* Now we can translate the new target */
  tld->ind_target = fbt_translate_noexecute(tld, original);
  PRINT_DEBUG("ind_target + %x\n", tld->ind_target);
  

  PRINT_DEBUG_FUNCTION_END("");
}

int fbt_restart_transaction(struct thread_local_data *tld, struct thread_local_data *target) {
  if (fbt_mutex_lock(&target->interrupt_cft_mutex) != 0) {
    PRINT_DEBUG("*** 1\n");
    PRINT_DEBUG("Thread %x is already being flushed. Skipping.\n", (ulong_t)tld);
    PRINT_DEBUG_FUNCTION_END("");
    return 2;
  }

  /* Target thread is not yet allowed to flush cache */
  fbt_mutex_lock(&target->interrupted_cft_mutex);

  /* Overwrite control flow transfers of target */
  fbt_interrupt_cftx(target, &cftx_handler);

  /* Target thread can now flush cache */
  fbt_mutex_unlock(&target->interrupted_cft_mutex);
   
  return 0;
}


void fbt_interrupt_cftx(struct thread_local_data *tld, void *interruption_target) {
  tld->interruption_target = interruption_target;

  /* Interrupt all control flow transfers in user code */
  struct control_flow_transfer *cft = tld->control_flow_transfers;
  while (cft != NULL) {
    char *offset = (char *)cft->location;

    *((ulong_t *)offset) = (ulong_t)cft->trampoline - ((ulong_t)offset + 4);
    cft = cft->next;
  }

  /* Full memory barrier, so writes to trampolines are not reordered before
   * writes to direct jumps */
  __sync_synchronize();


  /* Interrupt all control flow transfers in trampolines (indirect jumps through ind_target) */
  struct trampoline_cft *tcft = tld->trampoline_cfts_begin;
  while (tcft != tld->trampoline_cfts_end) {
    *((ulong_t *)((ulong_t)tcft->addr)) = (ulong_t)&tld->interrupt_cft_trampoline;
    ++tcft;
  }
}

void fbt_fix_trampolines_after_interrupt_cftx(struct thread_local_data *tld) {
  struct trampoline_cft *tcft = tld->trampoline_cfts_begin;
  while (tcft != tld->trampoline_cfts_end) {
    *((ulong_t *)tcft->addr) = (ulong_t)&tld->ind_target;
    ++tcft;
  }  
}

void fbt_store_cftx(struct thread_local_data *tld, struct control_flow_transfer *cft)
{
  /* Allocate space for entry */
  struct control_flow_transfer *next = tld->control_flow_transfers;
  tld->control_flow_transfers = fbt_mem_pool_allocate(tld->cft_trampoline_mem_pool, sizeof(struct control_flow_transfer));

  ulong_t trampoline_size = 10 + 5;

  /* Create a trampoline for this cft */
  cft->trampoline = fbt_mem_pool_allocate(tld->cft_trampoline_mem_pool, trampoline_size);

  char *transl = cft->trampoline;
  BEGIN_ASM(transl)
    movl ${tld->control_flow_transfers}, {&tld->interrupted_cft} // 10 bytes
    jmp_abs {tld->interrupt_cft_from_user_code_trampoline}       // 5 bytes
  END_ASM

  /* Store control flow transfer */
  *tld->control_flow_transfers = *cft;
  tld->control_flow_transfers->next = next;

}

#endif /* TRACK_CFTX */