/**
 * @file fbt_mem_mgmt.c
 * Implementation of the internal memory management for the BT (code cache,
 * trampolines, mapping table, internal memory)
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

#include "fbt_mem_mgmt.h"

#include <assert.h>
#include <asm-generic/mman.h>

#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "fbt_datatypes.h"
#include "fbt_debug.h"
#if defined(VERIFY_CFTX)
#include "fbt_dso.h"
#endif  /* VERIFY_CFTX */
#if defined(DYNARACE)
#include "fbt_dynarace.h"
#endif  /* DYNARACE */
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_pool.h"
#include "fbt_syscall.h"
#if defined(SEL_DEBUG)
#include "fbt_sdbg.h"
#endif
#if defined(TRACK_BASIC_BLOCKS)
#include "fbt_mutex.h"
#endif /* TRACK_BASIC_BLOCKS */

#if defined(DEBUG)
#include <sys/stat.h>
#include <stdarg.h>
#endif /* DEBUG */

struct thread_local_data *fbt_init_tls() {
  return fbt_reinit_tls(NULL);
}

struct thread_local_data *fbt_reinit_tls(struct thread_local_data *tld) {
  /* allocate (bootstrapping) memory */
  void *mem;
  if (tld == NULL) {
    fbt_mmap(NULL, SMALLOC_PAGES * PAGESIZE, PROT_READ|PROT_WRITE,
             MAP_PRIVATE|MAP_ANONYMOUS, -1, 0, mem,
             "BT failed to allocate memory (fbt_reinit_tls: "
             "fbt_mem_mgmt.c)\n");
  } else {
    /* Free all the dynamic memory we have allocated. Note that this will
       leave us with the one single chunk that we initially allocated that
       contains stack and tld.
       This last chunk will be reset so that it can be used like the mmap
       above. */
    fbt_mem_free(tld);
    mem = tld->chunk->ptr;
  }

  /* stack grows BT_STACK_SIZE pages down */
  void *stack = mem + (BT_STACK_SIZE * PAGESIZE);

  assert(tld == NULL || (tld != NULL && tld == stack));
  tld = (struct thread_local_data*)(stack);
  tld->ind_target = NULL;
  tld->stack = stack;

  /* initialize memory allocation */
  tld->chunk = (struct mem_info*)(tld + 1);
  tld->chunk->next = NULL;
  tld->chunk->type = MT_INTERNAL;
  tld->chunk->ptr = mem;
  tld->chunk->size = SMALLOC_PAGES * PAGESIZE;

  /* initialize translate struct */
  tld->trans.tld = tld;
  tld->trans.transl_instr = NULL;
  tld->trans.code_cache_end = NULL;
  tld->trans.trampos = NULL;
  tld->trans.cur_instr = NULL;
  tld->trans.cur_instr_info = NULL;
  tld->trans.first_byte_after_opcode = NULL;
  tld->trans.num_prefixes = 0;
  tld->trans.next_instr = NULL;

  tld->smalloc = (void*)(tld->chunk + 1);
  tld->smalloc_size = (SMALLOC_PAGES * PAGESIZE) - ((ulong_t)(tld->smalloc) -
                                                    (ulong_t)(mem));

  assert(tld->smalloc_size > 0);

  /* starting from this point we can use our internal memory allocation */

  /* allocate memory for hashtable.
     lalloc uses mmap and map_anonymous, so the table is initialized with 0x0
     therefore we don't need to memset the whole table+4 for 0x1 guard for
     tcache_find_fast asm function */
  tld->mappingtable = fbt_lalloc(tld, (MAPPINGTABLE_SIZE / PAGESIZE) + 1,
                                 MT_MAPPING_TABLE);
  /* guard for find_fast-wraparound used in optimizations */
  *(long*)((long)(tld->mappingtable)+MAPPINGTABLE_SIZE) = 0x1; 

  PRINT_DEBUG("allocated mappingtable: %p -> %p",
              tld->mappingtable, tld->mappingtable + MAPPINGTABLE_SIZE);

  
  /* initialize trampolines */
  tld->ret2app_trampoline = NULL;
  tld->opt_ijump_trampoline = NULL;
  tld->opt_icall_trampoline = NULL;
  tld->unmanaged_code_trampoline = NULL;
  tld->opt_ret_trampoline = NULL;
  tld->opt_ret_remove_trampoline = NULL;

#if defined(ICF_PREDICT)
  tld->opt_ijump_predict_fixup = NULL;
  tld->opt_icall_predict_fixup = NULL;
  tld->icf_predict = NULL;
#endif  /* ICF_PREDICT */
  
#if defined(SHADOWSTACK)
  /* allocate shadow stack */
  /* enough for 1024 or 512 recursive stack frames - on 32/64 bit */
  tld->top_of_shadowstack = fbt_lalloc(tld, 2, MT_SHADOWSTACK);
  tld->shadowstack = tld->top_of_shadowstack;
  
  /* We do not know the location of the loader yet */
  tld->shadowstack_loader_begin = 0;
  tld->shadowstack_loader_end = 0;

  /* Create a dummy entry on the shadow stack */
  fbt_memset(tld->top_of_shadowstack, 0, sizeof(struct shadowstack_entry));
  /* If libdetox is secure, this value is not accessible to an attacker and therefore
   * cannot be guessed to cause a segmentation fault */
  tld->top_of_shadowstack->return_address = &tld->shadowstack_end;
  tld->top_of_shadowstack += 1;
#endif

#if defined(VERIFY_CFTX)
  tld->dso_objects = NULL;
  tld->symb_cache = fbt_smalloc(tld, SYMBOL_CACHE_SIZE*sizeof(struct symbol_cache));
#if defined(DUMP_CFTX)
  char path_name[128];
  fbt_memset(path_name, 0, sizeof(path_name));
  fbt_memcpy(path_name, "dump_cftx_", 10);
  ulong_t pos = 10;
  ulong_t rem = (ulong_t)tld;
  char transl[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  /* Add tld address to path */
  while (rem != 0) {
    ulong_t i = rem % 16;
    path_name[pos] = transl[i];
    pos += 1;
    rem /= 16;
  }
  path_name[pos] = '\0';

  long dump_cftx_file;
  fbt_open(
      path_name,
      O_CREAT | O_TRUNC | O_WRONLY,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
      dump_cftx_file,
      "Could not open file (fbt_mem_mgmt.c)"
  );
  tld->dump_cftx_file = dump_cftx_file;
#endif /* DUMP_CFTX */  
#endif  /* VERIFY_CFTX */
  
#if defined(AUTHORIZE_SYSCALLS)
  tld->syscall_location = NULL;
  ulong_t table_size = (((MAX_SYSCALLS_TABLE*sizeof(void*)) + (PAGESIZE-1)) &
                        (~(PAGESIZE-1))) / PAGESIZE;

  tld->syscall_table = \
    (enum syscall_auth_response (**)(struct thread_local_data*, ulong_t,
                                     ulong_t, ulong_t, ulong_t, ulong_t,
                                     ulong_t, ulong_t*, ulong_t, ulong_t*))
    fbt_lalloc(tld, table_size, MT_SYSCALL_TABLE);
  assert(table_size == 1);
#if defined(SYSCALL_POLICY_FILE)
  tld->policy_entries = (struct syscall_policy_entry**) \
    fbt_lalloc(tld, table_size, MT_SYSCALL_POLICY);
#endif  /* SYSCALL_POLICY_FILE */
#endif  /* AUTHORIZE_SYSCALLS */
#if defined(SEL_DEBUG)
  if (tld->sdbg == NULL) {
    tld->sdbg = sdbg_init(tld);
  }
#endif

#if defined(DYNARACE)
  tld->dynfile_free = NULL;
  tld->dynfile_cache = NULL;
#endif  /* DYNARACE */


#if defined(TRACK_BASIC_BLOCKS)
  tld->basic_blocks = NULL; 
#endif /* TRACK_BASIC_BLOCKS */

#if defined(TRACK_INSTRUCTIONS)
  tld->instructions = fbt_lalloc(tld, TRACK_INSTRUCTIONS_PAGES_PER_LEVEL, MT_INTERNAL);
  fbt_memset(tld->instructions, 0, sizeof(ulong_t) * 256);
#endif /* TRACK_INSTRUCTIONS */

#if defined(TRACK_CFTX)

  tld->trampoline_cfts_begin = fbt_lalloc(tld, 1, MT_INTERNAL);
  tld->trampoline_cfts_end = tld->trampoline_cfts_begin;

  fbt_mutex_init(&tld->interrupt_cft_mutex);
  fbt_mutex_init(&tld->interrupted_cft_mutex);

  /* Bootstrap memory pool for CFTs trampolines */
  struct mem_pool init_pool = {
    .allocation_size = 2048,
    .flags = PROT_READ | PROT_WRITE | PROT_EXEC
  };
  tld->cft_trampoline_mem_pool = fbt_mem_pool_bootstrap(&init_pool);

#endif /* TRACK_CFTX */

  /* add code cache */
  fbt_allocate_new_code_cache(tld);
  
  return tld;
}

void fbt_reinit_new_process(struct thread_local_data *tld) {
#if defined(TRACK_CFTX)
  fbt_mutex_init(&tld->interrupt_cft_mutex);
  fbt_mutex_init(&tld->interrupted_cft_mutex);
#endif /* TRACK_CFTX */  
  
  #if defined(SHARED_DATA)
  /* Reinitialize thread list */
  // TODO: we leak old thread list
  struct shared_data *sd = tld->shared_data;
  sd->threads = fbt_smalloc(tld, sizeof(struct thread_entry));
  sd->threads->tld = tld; 
  sd->threads->next = NULL;

  #if defined(ONLINE_PATCHING)
  fbt_mutex_init(&sd->patching_information_lock);
  #endif /* ONLINE_PATCHING */
  
  fbt_mutex_init(&sd->threads_mutex);
  tld->shared_data = sd;
  #endif /* SHARED_DATA */

}


void fbt_allocate_new_code_cache(struct thread_local_data *tld) {
  void *mem = fbt_lalloc(tld, CODE_CACHE_ALLOC_PAGES, MT_CODE_CACHE);
  tld->trans.transl_instr = mem;
  tld->trans.code_cache_end = mem + (CODE_CACHE_ALLOC_PAGES * PAGESIZE) -
    TRANSL_GUARD;
}

void fbt_allocate_new_trampolines(struct thread_local_data *tld) {
  ulong_t trampo_size = (((ALLOC_TRAMPOLINES * sizeof(struct trampoline)) +
                          (PAGESIZE-1)) & (~(PAGESIZE-1))) / PAGESIZE;

  void *mem = fbt_lalloc(tld, trampo_size,
MT_TRAMPOLINE);
  struct trampoline *trampos = (struct trampoline*)mem;

  /* initialize linked list */
  long i;
  for (i=0; i<ALLOC_TRAMPOLINES-1; ++i) {
    trampos->next = trampos+1;
    trampos = trampos->next;
  }
  trampos->next = tld->trans.trampos;

  tld->trans.trampos = (struct trampoline*)mem;  
}

void fbt_trampoline_free(struct thread_local_data *tld,
                         struct trampoline *trampo) {
  trampo->next = tld->trans.trampos;
  tld->trans.trampos = trampo;
}

#if defined(DYNARACE)
void fbt_allocate_new_dynarace_files(struct thread_local_data *tld) {
  ulong_t alloc_size = (((ALLOC_DYNARACE * sizeof(struct dynarace_file)) +
                         (PAGESIZE-1)) & (~(PAGESIZE-1))) / PAGESIZE;
  
  void *mem = fbt_lalloc(tld, alloc_size, MT_INTERNAL);
  struct dynarace_file *dynfile = (struct dynarace_file*)mem;

  /* initialize linked list */
  unsigned long i;
  for (i = 0; i < (ALLOC_DYNARACE-1); ++i) {
    dynfile->next = dynfile+1;
    dynfile = dynfile->next;
  }
  dynfile->next = tld->dynfile_free;

  tld->dynfile_free = (struct dynarace_file*)mem;  
}  
#endif  /* DYNARACE */

#if defined(ICF_PREDICT)
void fbt_allocate_new_icf_predictors(struct thread_local_data *tld) {
  ulong_t predict_size = (((ALLOC_PREDICTIONS * sizeof(struct icf_prediction)) +
                           (PAGESIZE-1)) & (~(PAGESIZE-1))) / PAGESIZE;

  void *mem = fbt_lalloc(tld, predict_size, MT_ICF_PREDICT);
  struct icf_prediction *icf_preds = (struct icf_prediction*)mem;

  /* initialize linked list */
  long i;
  for (i = 0; i < (long)(ALLOC_PREDICTIONS - 1); ++i) {
    icf_preds->pred.next = icf_preds+1;
    icf_preds = icf_preds->pred.next;
  }
  icf_preds->pred.next = tld->icf_predict;

  tld->icf_predict = (struct icf_prediction*)mem;  
}

void fbt_icf_predictor_free(struct thread_local_data *tld,
                            struct icf_prediction *icf_predict) {
  icf_predict->pred.next = tld->icf_predict;
  icf_predict->nrmispredict = 0;
  icf_predict->origin1 = NULL;
  icf_predict->dst1 = NULL;
  tld->icf_predict = icf_predict;
}
#endif  /* ICF_PREDICT */

void fbt_mem_free(struct thread_local_data *tld) {
  assert(tld != NULL);
  long kbfreed = 0;
  struct mem_info *chunk = tld->chunk;
  while (chunk->next != NULL) {
    long ret;
    /* we need to save the next pointer. munmap could unmap the last allocated
       data and chunk itself would no longer be valid. this is a bootstrapping
       problem and takes care of the last allocated chunk. */
    struct mem_info *next = chunk->next;
    kbfreed += chunk->size >> 10;
    fbt_munmap(chunk->ptr, chunk->size, ret, "BT failed to deallocate memory "
               "(fbt_mem_free: fbt_mem_mgmt.c)\n");
    chunk = next;
  }
  tld->chunk = chunk;
  PRINT_DEBUG("%d KB freed on fbt_mem_free", kbfreed);
}

void *fbt_lalloc(struct thread_local_data *tld, int pages,
                 enum mem_type type) {
  assert(pages > 0);
  if (pages <= 0)
    fbt_suicide_str("Trying to allocate 0 pages (fbt_lalloc: fbt_mem_mgmt.c)\n");
  
  /* TODO: add guard pages for stack, mapping table, code cache */
  int alloc_size = pages * PAGESIZE;

  struct mem_info *chunk = fbt_smalloc(tld, sizeof(struct mem_info));

  /* what flags should we use for the current alloc? */
  long flags = 0;
  switch (type) {
    case MT_INTERNAL:
    case MT_MAPPING_TABLE:
#if defined(SHARED_DATA)
    case MT_SHARED_DATA:
#endif /* SHARED_DATA */
#if defined(ONLINE_PATCHING)
    case MT_ONLINE_PATCHING_STACK:
#endif /* ONLINE_PATCHING */
#if defined(SHADOWSTACK)
    case MT_SHADOWSTACK:
#endif  /* SHADOWSTACK */
#if defined(AUTHORIZE_SYSCALLS)
    case MT_SYSCALL_TABLE:
#if defined(SYSCALL_POLICY_FILE)
    case MT_SYSCALL_POLICY:
#endif  /* SYSCALL_POLICY_FILE */
#endif  /* AUTHORIZE_SYSCALLS */
#if defined(ICF_PREDICT)
    case MT_ICF_PREDICT:
#endif  /* ICF_PREDICT */
      flags = PROT_READ|PROT_WRITE;
      break;
      
    case MT_CODE_CACHE:
    case MT_TRAMPOLINE:
      flags = PROT_READ|PROT_WRITE|PROT_EXEC;
      break;
  }
  
  void *retval;
  fbt_mmap(NULL, alloc_size, flags, MAP_PRIVATE|MAP_ANONYMOUS,  \
           -1, 0, retval, "BT failed to allocate memory (fbt_lalloc: "
           "fbt_mem_mgmt.c)\n");

  /* we do not track shared data, as it should never be freed */
  int track_chunk = 1;
  switch(type) {
#if defined(ONLINE_PATCHING)
    case MT_ONLINE_PATCHING_STACK:
#endif /* ONLINE_PATCHING */
#if defined(SHARED_DATA)
    case MT_SHARED_DATA:
#endif /* SHARED_DATA */
      track_chunk = 0;
    default:
      break;
  }

  /* fill in the memory chunk information and store it in the list */
  if (track_chunk)  {
    chunk->ptr = retval;
    chunk->size = alloc_size;
    chunk->type = type;
    chunk->next = tld->chunk;
    tld->chunk = chunk;
  }
  return retval;
}

void *fbt_smalloc(struct thread_local_data *tld, long size) {
  /* ensure that we use smalloc only for small stuff */
  if (size > SMALLOC_MAX || size <= 0) {
    fbt_suicide_str("Too much memory requested (fbt_smalloc: fbt_mem_mgmt.c)\n");
  }
  /* do we need to allocate additional small memory space? */
  if (size > tld->smalloc_size) {
    void *mem;
    fbt_mmap(NULL, SMALLOC_PAGES * PAGESIZE, PROT_READ|PROT_WRITE,
             MAP_PRIVATE|MAP_ANONYMOUS, -1, 0, mem,
             "BT failed to allocate memory (fbt_smalloc: fbt_mem_mgmt.c)\n");
    tld->smalloc_size = SMALLOC_PAGES * PAGESIZE;
    tld->smalloc = mem;

    struct mem_info *chunk = (struct mem_info*) \
      fbt_smalloc(tld, sizeof(struct mem_info));

    chunk->type = MT_INTERNAL;
    chunk->ptr = mem;
    chunk->size = SMALLOC_PAGES * PAGESIZE;

    chunk->next = tld->chunk;
    tld->chunk = chunk;
  }
  /* let's hand that chunk of memory back to the caller */
  void *mem = tld->smalloc;
  tld->smalloc += size;
  tld->smalloc_size -= size;
  
  assert(((long)tld->smalloc) == ((long)mem)+size);
  
  return mem;
}

#ifdef SHARED_DATA
void fbt_init_shared_data(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("fbt_init_shared_data(%x)", tld);

  fbt_gettid(tld->tid);

  struct shared_data *sd = fbt_lalloc(tld, 
                                      NRPAGES(sizeof(struct shared_data)),
                                      MT_SHARED_DATA);
  
  sd->threads = fbt_smalloc(tld, sizeof(struct thread_entry));
  sd->threads->tld = tld; 
  sd->threads->next = NULL;

  #if defined(ONLINE_PATCHING)
  sd->patching_information = NULL;
  #endif /* ONLINE_PATCHING */
  
  fbt_mutex_init(&sd->threads_mutex);

  tld->shared_data = sd;
  
  PRINT_DEBUG_FUNCTION_END("");
}
#endif /* SHARED_DATA */
