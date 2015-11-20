/**
 * @file fbt_patching.h
 * Header file for the online patching extension for libdetox, which allows
 * changing a program during run time.
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
#ifndef FBT_ONLINE_PATCHING_H
#define FBT_ONLINE_PATCHING_H

#include <stdio.h>

#define ONLINE_PATCHING_ERROR_MULTIPLE_MODES \
    "Please only enable one of ONLINE_PATCHING_FILE, ONLINE_PATCHING_SOCKET" \
    "at a time."

/* Make sure that only one online patching mode is enabled at a time */
#if defined(ONLINE_PATCHING_FILE)
    #ifdef ONLINE_PATCHING_SOCKET
    #error ONLINE_PATCHING_ERROR_MULTIPLE_MODES
    #endif
#elif defined(ONLINE_PATCHING_SOCKET)    

#else
    /* We will only use offline patching */
#endif

/* Settings */
#define PATCHING_THREAD_TLS_SIZE 40960
#define PATCHING_THREAD_STACK_SIZE 40960

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct thread_local_data;
struct change;

/* Primitive types */
typedef unsigned long address_t;

/** Contains information about the current state of the patching engine */
struct patching_information {
  /** The version number of the current patch. This will be increased every
      time a new patch is loaded, so other threads know whether they have to
      flush the cache and load the new patching information */
  long version;
  
  /** Points to an array containing all the changes sorted by address in
     increasing order */
  struct change *changes;
  
  /** Number of elements in 'changes' */
  int num_changes;

  /** The memory pool that was used to allocated all memory used by this 
    * patch */
  struct mem_pool *mem_pool;

  /** Number of threads that are still referencing this structure */
  long ref_count;
};

/** First message received in patching protocol */
struct header_message {
  unsigned long num_changes;
#if defined(ONLINE_PATCHING_LOADER)
  /** Number of DSO that will be loaded as part of this patch */
  unsigned long num_dsos;
  /** The total number of replacements performed with all dsos */
  unsigned long num_replacements;
  /** The number of instructions that loaded DSOs still want to access through 
    * renamed symbols */
  unsigned long num_keeps;
  
#endif /* ONLINE_PATCHING_LOADER */
};

/** See struct change for description of members */
struct change_message {
  address_t offset;
  address_t generation_addr;
  unsigned long original_instr_len;
  unsigned long num_bytes;
};

#ifdef ONLINE_PATCHING_LOADER
/** Entry describing a dynamic library to be loaded */
struct dso_entry {
  /** Path of the dso to be loaded, relative to the patch location */
  char path[256];

  /** Number of symbols to replace */
  unsigned long num_replacements;

  /** Number of instructions to keep from old version */
  unsigned long num_keeps;
};

/** 
 * A replacement as part of a loaded dynamic library.  It requests
 * the instruction at <code>address</code> to be reidrected to the 
 * symbol with name <code>symbol</code> in the loaded library.
 */
struct dso_replacement {
  /** The address to replace with a new version */
  unsigned long address;
  /** Symbol name from the loaded DSO to use */
  char symbol[256];
};

/** 
 * A 'keep' instruction as part of a loaded dynamic library. It requests
 * to make the function at <code>address</code> to be made available
 * as the symbol with name <code>symbol</code> in the loaded library. The
 * symbol must already exist in the library and be at least 2 bytes in length.
 *
 * This is implemented by keeping the first instruction at <code>address</code>
 * as a patched instruction at the <code>symbol</code>'s address, followed by
 * a jump to the next instruction after <code>address</code>, thus allowing
 * the function to be used at the given symbol even if it is overwritten by 
 * a patch, thus enabling 'wrapping' of functions.
 */
struct dso_keep {
  /** The address of the function to 'keep' */
  unsigned long address;
  /** Name of the symbol that should be used to access the 'kept' function */
  char symbol[256];
};
#endif

/** Internal representation of patch-related changes */
struct change {
  /** The address of the instruction that will be replaced with this change */
  address_t offset;

  /** The original address of the replacement instructions that will be used
  in relative references (e.g. jumps, calls) */
  address_t generation_addr;

  /** The length in bytes of the instruction to replace */  
  unsigned long original_instr_len;

  /** The number of machine code bytes in the patched version */
  unsigned long num_bytes;

  /** New machine code */
  char *machine_code;
};

/** Flags to be used when applying patch */
enum apply_patch_flags {
  OP_OFFLINE = 0,
  OP_ONLINE = 1
};

/** 
 * Initializes the patching state when libdetox is bootstrapped
 * 
 * @param tld Thread-local data of initial libdetox context
 */
void fbt_online_patching_init(struct thread_local_data *tld);

/**
 * Initializes online patching for a new process after a fork
 * 
 * @param tld Thread-local data of the new process
 */
void fbt_online_patching_start(struct thread_local_data *tld);

/**
 * Looks up an address in the patching information to find a patch for it
 *
 * @param pi Patching information to search
 * @param p  Address to find a patch for
 * @return Pointer to the patch for <code>p</code>, or NULL if none was found
 */
struct change *fbt_online_patching_find_change(struct patching_information *pi,
                                               void* p);

/** 
 * Applies a patch a file represented by a file descriptor
 * 
 * @param tld   Thread-local data 
 * @param fd    File descriptor to read patching structure from
 * @param flags A combination of flags(OR'd) in 
 *              <code>enum apply_patch_flags</code>
 */
void fbt_apply_patch_from_file(struct thread_local_data *tld,
                               long fd,
                               long flags);

/** 
 * Applies a patch from provided patching information
 * 
 * Note: if the patch does not have its own memory pool yet, one will be created
 *       with a shallow copy of the patching information in it. 
 * 
 * @param tld   Thread-local data 
 * @param pi    Patching information to apply
 * @param flags A combination of flags(OR'd) in 
 *              <code>enum apply_patch_flags</code>
 */
void fbt_apply_patch(struct thread_local_data *tld, 
                     struct patching_information *pi,
                     long flags);

/**
 * Applies local patch from file system if one exists
 *  
 * @param tld Thread-local data
 * @param online A combination (OR'd) of <code>enum apply_patch_flags</code> values
 */
void fbt_apply_local_patch(struct thread_local_data *tld,
                           long online);

/** 
 * Translation function used when online patching is enabled.
 *
 * TODO: merge with standard translation?
 *
 * @param tld Thread-local data
 * @param orig_address Address of instruction to translate
 * @return Pointer into code cache of translated basic block
 */
void *fbt_online_patching_translate_noexecute(
    struct thread_local_data *tld,
    void *orig_address);

/**
 * Refreshes the patching information for a thread by fetching it from the
 * shared data
 *
 * @param tld Thread-local data for the thread whose patching information
 *            should be updated
 */    
void fbt_online_patching_refresh(struct thread_local_data *tld);


#ifdef __cplusplus
}
#endif

#endif
