/**
 * @file fbt_patching.c
 * Implementation file for the online patching extension for libdetox, which
 * allows changing a program during run time.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
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

#ifdef ONLINE_PATCHING

#if !defined(TRACK_CFTX)
#error Enable TRACK_CFTX to use ONLINE_PATCHING
#endif /* TRACK_CFTX */

#include <assert.h>

#include "../libfastbt.h"
#include "../fbt_asm_macros.h"
#include "../fbt_algorithms.h"
#include "../fbt_actions.h"
#include "../patching/fbt_patching.h"
#include "../fbt_mem_mgmt.h"
#include "../fbt_mem_pool.h"
#include "../fbt_llio.h"
#include "../fbt_translate.h"
#include "../fbt_debug.h"
#include "../fbt_syscalls_impl.h"
#include "../fbt_libc.h"
#include "../fbt_translate.h"
#include "../fbt_code_cache.h"
#include "../fbt_datatypes.h"
#include "../fbt_x86_opcode.h"
#include "../fbt_mem_mgmt.h"
#include "../fbt_syscalls_impl.h"
#include "../fbt_trampoline.h"
#include "../fbt_restart_transaction.h"

#define _GNU_SOURCE
#include <sched.h>
#include <sys/mman.h>
#include <errno.h>
#include <asm/ldt.h>

#ifdef ONLINE_PATCHING_LOADER
#include <dlfcn.h>
#endif

#if defined(DEBUG)
#include <pthread.h>
#endif /* DEBUG */

/* Default to binary search for patch lookup */
#if !defined(ONLINE_PATCHING_HASH_SEARCH) && !defined(ONLINE_PATCHING_LINEAR_SEARCH)
#define ONLINE_PATCHING_BINARY_SEARCH
#warning No patch searching algorithm specified: defaulting to ONLINE_PATCHING_BINARY_SEARCH
#endif

#if defined(ONLINE_PATCHING_HASH_SEARCH)
  #define INV_LOAD_FACTOR 2
#endif

#include "fbt_patching_filesystem.h"
#include "fbt_patching_socket.h"

/* -----------------------------------------------------------------------------
   Utility functions
   ---------------------------------------------------------------------------*/
#include "fbt_patching_util.h"

#if defined(ONLINE_PATCHING_BINARY_SEARCH)
static int compare_int(int a, int b) {
  if (a == b) {
    return 0;
  } else if (a < b) {
    return -1;
  } else {
    return +1;
  }
}

static int int_comparator(const void *a, const void *b) {
  return compare_int(*(int *)a, *(int *)b);
}

static int int_direct_comparator(const void *a, const void *b) {
  return compare_int((int)a, (int)b);
}

static int change_address_comparator(void *first, void *second) {
  return int_comparator(
    &((struct change *)first)->offset,
    &((struct change *)second)->offset
  );
}

static int query_change_by_offset(void *first, void *second) {
  return -int_direct_comparator(
    (void*)((struct change *)first)->offset,
    second
  );
}
#endif /* ONLINE_PATCHING_BINARY_SEARCH */

/* -----------------------------------------------------------------------------
   Patch application
   ---------------------------------------------------------------------------*/
void fbt_apply_local_patch(struct thread_local_data *tld, long online) {
  PRINT_DEBUG_FUNCTION_START("fbt_apply_local_patch\n");
  char patch_path[1024];
  
  int len = fbt_get_patch_path(patch_path, 1024);
  if (len > 0) {       
    long file;
    #define O_RDONLY 0
    fbt_openE(patch_path, O_RDONLY, 0, file);
    
    if (file > 0) {
      PRINT_DEBUG("Found a patch %s, will try to apply now\n", patch_path);                
      fbt_apply_patch_from_file(tld, file, online);
      long status;
      fbt_close(file, status, "Failed to close local patch");
    } else {
      PRINT_DEBUG("No patch found at %s (fd=%d).\n", patch_path, file);        
    }
  }
  PRINT_DEBUG_FUNCTION_END("");
}

void fbt_apply_patch(struct thread_local_data *tld, struct patching_information *pi, long online) {
  PRINT_DEBUG_FUNCTION_START("fbt_apply_patch(tld=%x, pi=%x, online=%d)", tld, pi, online);

  /* If the patch comes from outside libdetox, there might
   * not be a thread pool associated with it, so we copy the 
   * patching_information into a newly allocated thread pool */
  if (pi->mem_pool == NULL) {
    struct mem_pool init_pool = {
      .allocation_size = 1024,
      .flags = PROT_READ | PROT_WRITE
    };

    pi->mem_pool = fbt_mem_pool_bootstrap(&init_pool);

    struct patching_information *old_pi = pi;
    pi = (struct patching_information *)fbt_mem_pool_allocate(pi->mem_pool, sizeof(struct patching_information));
    *pi = *old_pi;
  }

  /* Only one thread (the patching thread) currently references the patching information */
  pi->ref_count = 1;

  #if defined(ONLINE_PATCHING_BINARY_SEARCH)

  /* Sort changes so we can use binary search for lookup */
  PRINT_DEBUG("Sorting changes for use with binary search.\n");
  fbt_qsort(pi->changes, pi->num_changes, sizeof(struct change), &change_address_comparator);

  #elif defined(ONLINE_PATCHING_HASH_SEARCH)
  
  PRINT_DEBUG("Building hash table.\n");
  
  struct change *original_changes = pi->changes;
  
  PRINT_DEBUG("num_changes = %d\n", pi->num_changes);

  size_t hashtable_size = pi->num_changes * sizeof(struct change) * INV_LOAD_FACTOR;
  
  PRINT_DEBUG("hashtable_size = %d\n");
  if (pi->num_changes) {
    pi->changes = fbt_mem_pool_allocate(pi->mem_pool, hashtable_size);
    fbt_memset(pi->changes, 0, hashtable_size);
  } else {
    pi->changes = NULL;
  }
  
  int i;
  for (i = 0; i < pi->num_changes; ++i) {
    struct change *change = &original_changes[i];
    
    assert(change->offset != 0);
        
    uint32_t pos = fbt_hash(&change->offset, sizeof(change->offset) / sizeof(uint32_t), 0);
    pos = pos % hashtable_size;
    
    /* Collision handling: if there is already an entry at this position, we
       have to increase the counter until we hit an empty cell (offset=0) */
    while (pi->changes[pos].offset != 0) {
      pos = (pos + 1) % hashtable_size;
    }
    
    PRINT_DEBUG("%x -> hash[%d]\n", change->offset, pos);
    
    pi->changes[pos] = *change;
  }  
  
  //fbt_free(original_changes);
  
  #endif

  pi->version = tld->shared_data->patching_information == NULL ?
                1 :
                tld->shared_data->patching_information->version + 1;


  PRINT_DEBUG("patching_information: %x\n", tld->shared_data->patching_information);
  PRINT_DEBUG("fbt_patching_information: version %d, num_changes: %d\n", pi->version, pi->num_changes);
  
  /* We will not access old information anymore */
  fbt_mutex_lock(&tld->shared_data->patching_information_lock);
  struct patching_information *old_pi = tld->shared_data->patching_information;
  if (old_pi != NULL) {
    long refs = __sync_sub_and_fetch(&old_pi->ref_count, 1);
    PRINT_DEBUG("%d refs remaining.\n");
    if (refs == 0) {
      PRINT_DEBUG("Freeing memory!\n");
      fbt_mem_pool_free(old_pi->mem_pool);
    }
  }
  fbt_mutex_unlock(&tld->shared_data->patching_information_lock);

  /* Swap in changes */
  tld->shared_data->patching_information = pi;

  /* lock threads list (no thread can terminate or start) and
     restart each thread's transaction */
  if (online) {
    PRINT_DEBUG("Acquiring mutex.\n");
    fbt_mutex_lock(&tld->shared_data->threads_mutex);
    struct thread_entry *current = tld->shared_data->threads;
    while (current != NULL) {
      PRINT_DEBUG("Stopping thread %x\n", current->tld);
      fbt_restart_transaction(tld, current->tld);
      current = current->next;
    }  
    fbt_mutex_unlock(&tld->shared_data->threads_mutex);
  }
  PRINT_DEBUG_FUNCTION_END("");
}

void fbt_apply_patch_from_file(struct thread_local_data *tld, long fd, long online) {
  PRINT_DEBUG_FUNCTION_START("fbt_apply_patch\n");
  struct header_message header;

  read_struct(fd, &header, sizeof(header));
  PRINT_DEBUG("num_changes: %x\n", header.num_changes);

  PRINT_DEBUG("Allocating %d bytes for changes..\n", header.num_changes * sizeof(struct change));
  
  /* Allocate memory pool for this patch */
  struct mem_pool init_pool;
  init_pool.allocation_size = 10 * PAGESIZE;
  init_pool.flags = PROT_READ | PROT_WRITE;

  struct mem_pool *pool = fbt_mem_pool_bootstrap(&init_pool);

  /* From here on, the memory of the patch is self contained, i.e.
   * we can discard the local copy of the pool descriptor */

  struct patching_information* pi = (struct patching_information *)fbt_mem_pool_allocate(pool, sizeof(struct patching_information));

  pi->mem_pool = pool;
  

  #if defined(ONLINE_PATCHING_LOADER)
  pi->num_changes = header.num_changes + header.num_replacements + 2 * header.num_keeps;
  #else
  pi->num_changes = header.num_changes;
  #endif
  
  if (pi->num_changes > 0) {
    pi->changes = (struct change *)fbt_mem_pool_allocate(pool, pi->num_changes * sizeof(struct change));
  } else {
    pi->changes = NULL;
  }

  unsigned i;
  for (i = 0; i < header.num_changes; ++i) {
    struct change_message change;
    read_struct(fd, &change, sizeof(change));

    // Read all the bytes for the change
    PRINT_DEBUG_FUNCTION_START("change(%x)", change.offset);
    PRINT_DEBUG("num bytes: %d\n", change.num_bytes);
    PRINT_DEBUG("original_instr_len: %d\n", change.original_instr_len);    

    char *buf = NULL;
    if (change.num_bytes > 0) {
      buf = fbt_mem_pool_allocate(pool, change.num_bytes);
      long bytes_read;
      fbt_read(fd, buf, change.num_bytes, bytes_read);
      assert(bytes_read == (long)change.num_bytes);
    } 
    
    unsigned j;
    for (j = 0; j < change.num_bytes; ++j) {
        PRINT_DEBUG("%x ", buf[j]);
    }
    PRINT_DEBUG("\n");

    pi->changes[i].offset = change.offset;
    pi->changes[i].generation_addr = change.generation_addr;
    pi->changes[i].original_instr_len = change.original_instr_len;
    pi->changes[i].num_bytes = change.num_bytes;
    pi->changes[i].machine_code = buf;
    
    PRINT_DEBUG_FUNCTION_END("");    
  }



  /* Read new DSOs */
  #if defined(ONLINE_PATCHING_LOADER)
  PRINT_DEBUG("[Loader]\n");
  PRINT_DEBUG("num_dsos = %d\n", header.num_dsos);
  struct change *current_change = pi->changes + header.num_changes;
  for (i = 0; i < header.num_dsos; ++i) {
    PRINT_DEBUG("DSO %d\n", i);
    struct dso_entry dso;
    read_struct(fd, &dso, sizeof(dso));

    PRINT_DEBUG("name = %s\n", dso.path);
    PRINT_DEBUG("num_repl = %d / %d\n", dso.num_replacements, header.num_replacements);

    PRINT_DEBUG("Loading dso [%s]\n", dso.path);
    //llprintf("Loading dso [%s]\n", dso.path);
    void *handle = dlopen(dso.path, RTLD_NOW | RTLD_NODELETE | RTLD_LOCAL | RTLD_DEEPBIND);
    //llprintf("handle = %x\n", handle);

    PRINT_DEBUG("handle = %x\n", handle);
    if (handle == NULL) {
      llprintf("** ERROR ** Could not load patching DSO '%s'.\n", dso.path);
      return;
    }

    /* Load all keep declarations */
    unsigned long j;
    for (j = 0; j < dso.num_keeps; ++j) {
      struct dso_keep keep;
      read_struct(fd, &keep, sizeof(keep));

      PRINT_DEBUG("keep %x at %s\n", keep.address, keep.symbol);
      void *sym = dlsym(handle, keep.symbol);
      PRINT_DEBUG("sym = %x\n", sym);

      /* disassemble instruction */
      struct translate ts;
      ts.next_instr = (unsigned char *)keep.address;
      fbt_disasm_instr(&ts);

      /* Replace the target address with a copy of the old instruction */
      long instr_length = ts.next_instr - ts.cur_instr;

      /* Copy first instruction */
      char *buf = fbt_mem_pool_allocate(pool, instr_length);
      fbt_memcpy(buf, ts.cur_instr, instr_length);

      struct change c1 = {
        .offset = (address_t)sym,
        .generation_addr = (address_t)ts.cur_instr,
        .original_instr_len = 1,
        .num_bytes = instr_length,
        .machine_code = buf
      };

      *current_change = c1;

      current_change += 1;


      /* Insert instruction that jumps back to the 2nd instruction of the original */
      long after = keep.address + instr_length;

      buf = fbt_mem_pool_allocate(pool, 5);
      char *target = buf;
      BEGIN_ASM(target)
        jmp_abs {after}
      END_ASM

      struct change c2 = {
        .offset = (address_t)(sym + 1),
        .generation_addr = (address_t)buf,
        .original_instr_len = instr_length,
        .num_bytes = 5,
        .machine_code = buf
      };

      *current_change = c2;

      current_change += 1;

    }
    

    /* Load all replacements */
    for (j = 0; j < dso.num_replacements; ++j) {
      PRINT_DEBUG("repl %d\n", j);
      struct dso_replacement replacement;
      read_struct(fd, &replacement, sizeof(replacement));

      void *sym = dlsym(handle, replacement.symbol);
      if (sym == NULL) {
        PRINT_DEBUG("*** ERROR *** Could not find symbol.\n");
      }
      PRINT_DEBUG("map %s = %x\n", replacement.symbol, sym);

      /* Replace the target address with a jump to the new symbol */
      char *buf = fbt_mem_pool_allocate(pool, 5);
      char *target = buf;
      BEGIN_ASM(target)
        jmp_abs {sym}
      END_ASM

      struct change c = {
        .offset = replacement.address,
        .generation_addr = (address_t)buf,
        .original_instr_len = 0,
        .num_bytes = 5,
        .machine_code = buf
      };

      *current_change = c;

      current_change += 1;

    }
  }
  #endif /* ONLINE_PATCHING_LOADER */
  

  fbt_apply_patch(tld, pi, online);

  
  PRINT_DEBUG_FUNCTION_END(" ");
}


/* -----------------------------------------------------------------------------
   Initialization
   ---------------------------------------------------------------------------*/
void fbt_online_patching_refresh(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("fbt_online_patching_refresh\n");
  
  struct patching_information *old_pi = tld->patching_information;
  if (old_pi != NULL) {
    long refs = __sync_sub_and_fetch(&old_pi->ref_count, 1);    
    PRINT_DEBUG("decreased refs to %d\n", refs);
    if (refs == 0) {
      PRINT_DEBUG("Freeing memory!\n");
      fbt_mem_pool_free(old_pi->mem_pool);
    }
  }

  /* Acquire newest patching information and increment reference count */
  fbt_mutex_lock(&tld->shared_data->patching_information_lock);
  struct patching_information *new_pi = tld->shared_data->patching_information;
  
  if (new_pi != NULL) {
    #if defined(DEBUG)
    long refs = __sync_add_and_fetch(&new_pi->ref_count, 1);
    PRINT_DEBUG("increased refs to %d\n", refs);
    #else
    __sync_add_and_fetch(&new_pi->ref_count, 1);
    #endif /* DEBUG */
    
    PRINT_DEBUG("%x is now using patch %x with %d changes\n", tld, new_pi, new_pi->num_changes);
  }
  tld->patching_information = new_pi;
  fbt_mutex_unlock(&tld->shared_data->patching_information_lock);

  PRINT_DEBUG_FUNCTION_END("");
}

void fbt_online_patching_start(struct thread_local_data *parent_tld) {    
  struct thread_local_data *tld = parent_tld->shared_data->patching_tld;

  /* If we are starting the patching thread as the result of a fork call,
   * we have to reset the reference count on the patching information,
   * as threads will not be copied to the new process */
  struct patching_information *pi = tld->shared_data->patching_information;
  if (pi != NULL) {
    #if defined(DEBUG)
    long old_ref_count = pi->ref_count;
    #endif /* DEBUG */

    /* We have at least one reference from the patching thread */
    pi->ref_count = 1; 

    /* Is the new process also referencing our information? */
    if (pi == parent_tld->patching_information) {
      pi->ref_count += 1;
    }

    PRINT_DEBUG("Reset ref_count to %d (was: %d)\n", pi->ref_count, old_ref_count);
  }

  #ifdef fbt_online_patching_thread

  #if defined(DEBUG)
  /* When debugging is enabled, we have to use pthread_create,
   * as PRINT_DEBUG uses pthread features */
  typedef void *(pthread_func)(void *);
  pthread_t thread;
  pthread_create(&thread, NULL, (pthread_func *)fbt_online_patching_thread, (void *)tld->shared_data);

  #else
  /* In production mode, we use a direct system call */

  char *stack_mem = fbt_lalloc(tld, NRPAGES(PATCHING_THREAD_STACK_SIZE), MT_ONLINE_PATCHING_STACK);
  stack_mem = &stack_mem[PATCHING_THREAD_STACK_SIZE];
  
  char *mem = fbt_lalloc(tld, NRPAGES(PATCHING_THREAD_TLS_SIZE), MT_ONLINE_PATCHING_STACK);

  // TODO: we could reuse the stack if online patching is already initialized

  long flags = CLONE_VM | CLONE_SETTLS | CLONE_THREAD | CLONE_SIGHAND;

  struct user_desc info;
  info.entry_number = -1;
  info.base_addr = (int)mem;
  info.limit = PATCHING_THREAD_TLS_SIZE;

  long res;
  fbt_set_thread_area(&info, res);

  long ptid;
  long ctid;
  long retval = clone(fbt_online_patching_thread, stack_mem, flags, (void*)tld->shared_data, &ptid, &info, &ctid);

  PRINT_DEBUG("Online patching thread has PID=%d\n", retval);
  if (retval == -1) {
    PRINT_DEBUG("** ERROR ** failed to spawn online patching thread (error: %d).\n", errno);
    llprintf("** ERROR ** failed to spawn online patching thread (error: %d).\n", errno);
  }

  #endif /* DEBUG */

  #endif
}

void fbt_online_patching_init(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("fbt_online_patching_init\n");

  /* Initialize tld for patching thread so when forking (where we lose the
   * patching thread), we can start a new thread working on that data */
  tld->shared_data->patching_tld = fbt_init_tls();
  tld->shared_data->patching_tld->shared_data = tld->shared_data;

  #ifdef ONLINE_PATCHING_STARTUP
  // TODO: do also when _start'ing
  /* First, check for a local patch */
  fbt_apply_local_patch(tld, 0);
  tld->patching_information = tld->shared_data->patching_information;
  #endif

  fbt_online_patching_start(tld);
  
  PRINT_DEBUG_FUNCTION_END(" ");    
};

struct change *fbt_online_patching_find_change(struct patching_information *pi, void* p) {
  int i;
  if (pi == NULL) {
    return NULL;
  }
  
  if (pi->num_changes == 0) {
    return NULL;
  }

  struct change *changes = pi->changes;


  #if defined(ONLINE_PATCHING_LINEAR_SEARCH)
  
  for (i = 0; i < pi->num_changes; ++i) {
    struct change *c = &changes[i];
    if (c->offset == (unsigned long)p) {
      PRINT_DEBUG_FUNCTION_START("fbt_online_patching_find_change\n");    
      PRINT_DEBUG("Found a patch for %x\n", p);
      PRINT_DEBUG_FUNCTION_END(" ");        
      return c;
    }
  }

  #elif defined(ONLINE_PATCHING_HASH_SEARCH)

  size_t hashtable_size = pi->num_changes * sizeof(struct change) * INV_LOAD_FACTOR;
          
  uint32_t pos = fbt_hash(&p, sizeof(pi->changes[0].offset) / sizeof(uint32_t), 0);
  pos = pos % hashtable_size;
  
  /* Collision handling: if there is already an entry at this position, we
     have to increase the counter until we hit an empty cell (offset=0) */
  while (pi->changes[pos].offset != 0) {
    if (pi->changes[pos].offset == (unsigned)p) {
      return &pi->changes[pos];
    }
    pos = (pos + 1) % hashtable_size;
  }

  #else

  struct change *result = fbt_binary_search(
    changes,
    pi->num_changes,
    sizeof(struct change),
    &query_change_by_offset,
    p
  );

  if (result != changes + pi->num_changes && 
      result->offset == p)
  {
    return result;
  }

  #endif
  
  return NULL;
}

#endif /* ONLINE_PATCHING */

