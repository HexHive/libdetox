/**
 * @file sl_tls.c
 * Implementation of methods needed for static and dynamic Thread Local Storage
 * (TLS). Static tls is composed of all TLS blocks that are available at
 * process startup. For DSOs loaded after process startup, only dynamic tls
 * is allowed. 
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
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

#include "sl_libc.h"
#include "sl_macros.h"
#include "sl_tls.h"
#include "sl_datatypes.h"
#include "sl_so_chain.h"
#include "sl_macros.h"

extern struct rtld_global *rtld_glob;
extern dso *so_chain;


/**
 * Syscall trampoline to int80.
 */
asm ("sysinfo_int80:;"
     "int $0x80;"
     "ret");
 

/**
 * Get a random value.
 * @return random value
 */
static long random_value() {
  long ret = 0;
  /* Open /dev/urandom */
  long fd = sl_open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    sl_printf("Error random_value: could not open /dev/urandom.");
    sl_exit(1);
  }
  
  /* Read a random long value */
  if ((long)sl_read(fd, &ret, sizeof(ret)) == -1) {
    sl_printf("Error random_value: could not read /dev/urandom.");
    sl_exit(1);
  }
 
  sl_close(fd);
  return ret;
}


/**
 * Set the thread pointer to the thread control block.
 * @param tcb The thread control block
 */
static void set_thread_pointer(thread_control_block *tcb) {
  struct user_desc tls_descr;
  
  /* Initialize tcb */
  tcb->tcb = tcb;
  tcb->self = tcb;
  tcb->sysinfo = (uintptr_t) &sysinfo_int80;
  
  /* Set random values for the GNU stack canary and pointer guard */
  tcb->stack_guard = random_value();
  tcb->point_guard = random_value();

  /* Set up descriptor for the tls segment */
  tls_descr.entry_number = -1; /* Use a free entry */
  tls_descr.base_addr = (unsigned long) tcb;    
  tls_descr.limit = 0xfffff;                    
  tls_descr.seg_32bit = 1;
  tls_descr.contents = 0;
  tls_descr.read_exec_only = 0;
  tls_descr.limit_in_pages = 1;
  tls_descr.seg_not_present = 0;
  tls_descr.useable = 0;
  tls_descr.empty = 0;

  /* Set_thread_area syscall to obtain entry in the GDT for our tls segment*/
  if (sl_set_thread_area(&tls_descr) == -1) {
    sl_printf("Error: set thread area syscall\n");
    sl_exit(1);
  }

  /* Segment selector format 
   * | 15      3 | 2 | 1 0 |
   * |   index   | T |  P  |
   *
   * T = Table indicator (0 = GDT; 1 = LDT)
   * P = Privilege level (0 = kernel; 3 = user)
   */

  /* Write segment register. The index points to an entry in the GDT and is in
   * user privilege level, so we shift it 3 bits left (* 8) and set the lower
   * 3 bits to 011 (+ 3). */   
  asm volatile ("movl %0, %%gs"
                :
                : "g" (tls_descr.entry_number * 8 + 3));
}


/**
 * Allocates memory for the dynamic thread vector.
 * @param tcb The thread control block
 */
/* REFERENCED BY EXPORTED FUNCTION */
static void allocate_dtv(void *tcb) {
  /* Allocate memory for dtv */
  unsigned long dtv_length = GL(_dl_tls_max_dtv_idx) + DTV_SURPLUS;
  dynamic_thread_vector *dtv = sl_calloc (dtv_length + 2,
                                          sizeof(dynamic_thread_vector));
  if ((long)dtv == -1) {
    sl_printf("Error allocate_dtv: failed to allocate dtv.\n");
    sl_exit(1);
  }

  /* The first entry (tcb->dtv[-1]) contains the size of the dtv */
  dtv[0].counter = dtv_length;
    
  /* Install DTV */
  ((thread_control_block *) (tcb))->dtv = (dtv) + 1;
}


thread_control_block *setup_tls() {
#ifdef D_TLS  
  sl_printf ("\nSetting up TLS datastructures... ");
#endif
  
  /* Set number of elements in static TLS block */
  GL(_dl_tls_static_nelem) = GL(_dl_tls_max_dtv_idx);

  /* Allocate memory for dtv slotinfo list. Each DSO which uses TLS has a
     corresponding entry in the slotinfo list. */
  unsigned long num_slots = GL(_dl_tls_max_dtv_idx) + TLS_SLOTINFO_SURPLUS;
  GL(_dl_tls_dtv_slotinfo_list)
    = sl_calloc(1, (sizeof(struct dtv_slotinfo_list)
                    + num_slots * sizeof(struct dtv_slotinfo)));

  /* Check allocation */
  if ((long)GL(_dl_tls_dtv_slotinfo_list) == -1) {
    sl_printf("Error tls_init: failed to allocate slotinfo list.\n");
    sl_exit(1);
  }
  
  /* Initialize slotinfo list */
  GL(_dl_tls_dtv_slotinfo_list)->len = num_slots;
  GL(_dl_tls_dtv_slotinfo_list)->next = 0;
  struct dtv_slotinfo *slotinfo = GL(_dl_tls_dtv_slotinfo_list)->slotinfo;

  unsigned long offset = 0;
  unsigned long max_align = TLS_TCB_ALIGN;
  
  /* Iterate over shared objects, calculate tls offset for each and initialize 
     slotinfo list entry. */
  long i = 0;
  dso *so;
  for (so = so_chain; so != 0; so = so->next) {
     /* Check if object uses tls */
     if (so->tls_blocksize != 0) {
       /* Set slotinfo and get maximum alignment */
       slotinfo[i].map = so;
       max_align = MAX(max_align, so->tls_align);

       /* Compute offset (tcb-offset = tls) */
       unsigned long off = ROUND(offset + so->tls_blocksize, so->tls_align);

       offset = off;
       so->tls_offset = off;
       i++;
     }        
   }

   /* Size of the static tls block */
   unsigned long size = ROUND(offset + TLS_STATIC_SURPLUS, max_align) 
                        + TLS_TCB_SIZE;

   GL(_dl_tls_static_used) = offset; 
   GL(_dl_tls_static_size) = size;
   GL(_dl_tls_static_align) = max_align;

   /*
    * Allocate storage for the static tls data. It looks like this:
    * [TLS3] [TLS2] [TLS1] [TCB]
    * The TCB has a pointer to the dynamic thread vector (dtv) which contains
    * an entry for each loaded object which points to its tls block.
    */
   void *tcb = sl_malloc(size);
   if ((long)tcb == -1) {
     sl_printf("Error tls_init: failed to allocate memory for static tls.\n");
     sl_exit(1);    
   }

   /* Get address of thread control block (tcb) and set it to zero */
   tcb = (char *)tcb + size - TLS_TCB_SIZE;  
   sl_memset(tcb, '\0', TLS_TCB_SIZE);

   /* Allocate dtv */
   allocate_dtv(tcb);

   /* Set thread pointer to the tcb */
   set_thread_pointer(tcb);

#ifdef D_TLS  
   sl_printf("done\n");
#endif
   return (thread_control_block *)tcb;
}


/**
 * Update the dtv of the current thread according to the generation of the
 * module with id "modid".
 *
 * @param modid Id of the module
 * @return dso of the module with "modid"
 */
static dso *update_dtv (unsigned long modid) {
  /* Shared object and generation of the module */
  dso *mod_so = 0;
  unsigned long mod_gen = 0;  
  dynamic_thread_vector *dtv;  

  /* Get the dtv of the current thread */
  GET_DTV(dtv);

  /* Get slotinfo list */
  struct dtv_slotinfo_list *list = GL(_dl_tls_dtv_slotinfo_list);

  /* Go to list containing module */
  unsigned long i = modid;
  while (i >= list->len) {
    i -= list->len;
    list = list->next;
  }

  mod_gen = list->slotinfo[i].gen;
  
  if (dtv[0].counter < mod_gen) {
    /* The generation of the module is younger than what our dtv implements,
       we have to update some slots */
    list = GL(_dl_tls_dtv_slotinfo_list);

    /* Iterate over slotinfo lists */
    do {
      /* Iterate over list entries */
      for (i = 0; i < list->len; ++i) {
        unsigned long gen = list->slotinfo[i].gen;

        /* If generation is younger it may not yet been initialized, if its
           older we already have it in the dtv so we continue */
        if (gen > mod_gen || gen <= dtv[0].counter)
          continue;

        /* Get the corresponding shared_object */
        dso *curr_map = list->slotinfo[i].map;
        if (curr_map == 0) 
          continue;        
        
        /* Check if current dtv is large enough */
        unsigned long curr_modid = curr_map->tls_modid;

        if (dtv[-1].counter < curr_modid) {
          /* Not enough memory in dtv, allocate more */
          thread_control_block *tcb;
          GET_TCB(tcb);
          
          /* Allocate memory */
          allocate_dtv(tcb);

          /* Copy old dtv */
          sl_memcpy(tcb->dtv, dtv, 
                    dtv[-1].counter * sizeof(dynamic_thread_vector));

          /* Free old dtv and set pointer to new dtv */
          sl_free(dtv, dtv[-1].counter);
          dtv = tcb->dtv;
        }

        /* Set dtv entry */
        dtv[modid].pointer.is_static = 0;
        dtv[modid].pointer.val = TLS_DTV_UNALLOCATED;

        /* Save to return at the end */
        if (curr_modid == modid) {
          mod_so = curr_map;
        }
      }
      
    } while ((list = list->next) != 0);

    /* Update generation counter */
    dtv[0].counter = mod_gen;
  }

  return mod_so;
}


void add_to_slotinfo (dso *so) {
  /* Get slotinfo list */
  struct dtv_slotinfo_list *list = GL(_dl_tls_dtv_slotinfo_list);
  struct dtv_slotinfo_list *prev = 0;
  unsigned long i = so->tls_modid;

  /* Got to slot i */
  do {
    if (i < list->len)
      break;
    i -= list->len;
    prev = list;
    list = list->next;
  } while (list != 0);

  /* End of list? */
  if (list == 0) {
    /* No more space in slotinfo list, allocate new list */
    list = prev->next = sl_calloc(1, (sizeof(struct dtv_slotinfo_list)
                + TLS_SLOTINFO_SURPLUS * sizeof(struct dtv_slotinfo)));
    if ((long)list == -1) {
      sl_printf("Error add_to_slotinfo: failed to allocate memory for list\n");
      sl_exit(1);
    }
    
    list->len = TLS_SLOTINFO_SURPLUS;
    list->next = 0;
  }

  /* Set slotinfo and increment generation */
  list->slotinfo[i].map = so;
  list->slotinfo[i].gen = GL(_dl_tls_generation) + 1;
}


/* EXPORTED FUNCTION */
void * REG_ARGS(1) allocate_tls(void *tcb) {
#ifdef D_TLS  
  sl_printf("Allocating TLS (tcb: %p)\n", tcb);
#endif
  /* Allocate memory for the Dynamic Thread Vector */  
  allocate_dtv(tcb);
  /* Map in the tls init image of all loaded objects and set dtv entries */
  return allocate_tls_init(tcb);
}


/* EXPORTED FUNCTION */
void * REG_ARGS(1) allocate_tls_init(void *tcb) {
  /* Get dynamic thread vector of the current thread and the slotinfo list */
  dynamic_thread_vector *dtv = ((thread_control_block *)tcb)->dtv;
  struct dtv_slotinfo_list *list = GL(_dl_tls_dtv_slotinfo_list);
  unsigned long idx = 0, i = 0, max_gen = 0;

  /* Iterate over slotinfo lists */
  while (idx < GL(_dl_tls_max_dtv_idx)) {    

    /* Iterate over list entries */
    for (i = 0; i < list->len; ++i) {
      if (idx + i > GL(_dl_tls_max_dtv_idx)) 
        break;

      /* Get corresponding shared object */
      dso *so = list->slotinfo[i].map;
      if(so == 0)
        continue;

      /* Maximum generation number */
      max_gen = MAX(max_gen, list->slotinfo[i].gen);
      
      if (so->tls_offset == 0) {
        /* This is a dynamically loaded object, mark for lazy allocation */
        dtv[so->tls_modid].pointer.val = TLS_DTV_UNALLOCATED;
        dtv[so->tls_modid].pointer.is_static = 0;
        continue;
      }

      /* Compute address of data and set dtv slot */
      void *dest = (char *) tcb - so->tls_offset;
      dtv[so->tls_modid].pointer.val = dest;
      dtv[so->tls_modid].pointer.is_static = 1;
      
      /* Copy initialization image */
      sl_memcpy(dest, so->tls_initimage, so->tls_initimage_size);
      
      /* Clear .bss section */
      sl_memset(dest + so->tls_initimage_size, '\0',
                so->tls_blocksize - so->tls_initimage_size);
    }
    
    idx += i;
    list = list->next;
  }

  /* Update generation counter */
  dtv[0].counter = max_gen;
  
  return tcb;
}


/* EXPORTED FUNCTION */
void REG_ARGS(2) get_tls_static_info (unsigned long *sizep,
                                      unsigned long *alignp) {
  *sizep = GL(_dl_tls_static_size);
  *alignp = GL(_dl_tls_static_align);
}


/* EXPORTED FUNCTION */
void * REG_ARGS(1) tls_get_addr(struct tls_index *ti) {
#ifdef D_TLS  
  sl_printf("tls_get_addr, mod: %d off: %d\n", ti->ti_module, ti->ti_offset);
#endif
  dso *so = 0;
  
  /* Get the dtv of the current thread */
  dynamic_thread_vector *dtv;  
  GET_DTV(dtv);
  
  if (dtv[0].counter != GL(_dl_tls_generation)) {
    /* The dtv is out of date -> update */
    so = update_dtv(ti->ti_module);
    GET_DTV(dtv);
  }
  
  /* Get address of thread local data */
  void *p = dtv[ti->ti_module].pointer.val;

  if (p == TLS_DTV_UNALLOCATED) {
    /* Allocation was deferred so we do it now */
    if (so == 0) {
      sl_printf("Error tls_get_addr: no DSO found.\n");
      sl_exit(1);
    }

    /* Allocate memory for tls block and save size */
    p = dtv[ti->ti_module].pointer.val = sl_malloc(so->tls_blocksize);
    dtv[ti->ti_module].pointer.size = so->tls_blocksize;
    if ((long)p == -1) {
      sl_printf("Error tls_get_addr: failed to allocate tls block.\n");
      sl_exit(1);
    }

    /* Copy initialization image */
    sl_memcpy(p, so->tls_initimage, so->tls_initimage_size);

    /* Clear .bss section */
    sl_memset(p + so->tls_initimage_size, '\0',
              so->tls_blocksize - so->tls_initimage_size);

    /* This is dynamic tls */
    dtv[ti->ti_module].pointer.is_static = 0;
  }
  
  /* Calculate address of variable */
  return (char *)p + ti->ti_offset;
}


/* EXPORTED FUNCTION */
void *tls_get_addr_2(struct tls_index *ti) {
  sl_printf("Error tls_get_addr_2: not implemented.\n");
  sl_exit(1);
  return 0;
}


/* EXPORTED FUNCTION */
void REG_ARGS(2) deallocate_tls(void *tcb, unsigned long dealloc_tcb) {
#ifdef D_TLS  
  sl_printf("Deallocating TLS (tcb: %p)\n", tcb);
#endif

  /* Get dynamic thread vector */
  dynamic_thread_vector *dtv = ((thread_control_block *)tcb)->dtv;

  /* We need to free the non-static TLS memory */
  unsigned long i;
  for (i = 1; i < dtv[-1].counter; ++i) {
    if (!dtv[i].pointer.is_static && dtv[i].pointer.val != 0
        && dtv[i].pointer.val != TLS_DTV_UNALLOCATED) {
       /* Free tls block */
       sl_free(dtv[i].pointer.val, dtv[i].pointer.size);
    }
  }

  /* Free dtv array */
  sl_free(dtv-1, dtv[-1].counter * sizeof(dynamic_thread_vector));

  if (dealloc_tcb) {
    /* We have to free the Thread Control Block and static tls */
     tcb -= GL(_dl_tls_static_size);
     sl_free(tcb, GL(_dl_tls_static_size));
  }
}


/* EXPORTED FUNCTION */
long tls_setup(void) {
  sl_printf("Error tls_setup: not implemented.\n");
  sl_exit(1);
  return 0;
}
