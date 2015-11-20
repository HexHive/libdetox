/**
 * @file libfastbt.c
 * Interface to the binary translator which can be used from the outside
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-03-23 10:26:53 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedDate: 2011-03-23 10:26:53 +0100 (Wed, 23 Mar 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 443 $
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

#include "libfastbt.h"

#include <assert.h>

#include "fbt_code_cache.h"
#include "fbt_debug.h"
#include "fbt_dso.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "fbt_syscall.h"
#include "fbt_translate.h"
#include "fbt_trampoline.h"
#include "fbt_x86_opcode.h"

#if defined(FBT_STATISTIC)
#include "fbt_statistic.h"
#endif

/* forward declaration for the default opcode table */
extern struct ia32_opcode opcode_table_onebyte[];

#if defined(HIJACKCONTROL)
static struct thread_local_data *tld;
void _init() {
  llprintf("Starting BT\n");
#if defined(DEBUG)
  llprintf("This is a debug build, so do not expect stellar performance!\n");
#endif
  ffflush();
  //sleep(5);
  tld = fbt_init(NULL);
  fbt_start_transaction(tld, fbt_commit_transaction);
}

void _fini() {
  llprintf("Stopping BT\n");
  ffflush();
  fbt_commit_transaction();
  fbt_exit(tld);
}
#endif

/* this function is called in fbt_asm_functions.S, .section .init */
struct thread_local_data* fbt_init(struct ia32_opcode *opcode_table) {
  /* secuBT: call init function for the secure system call mechanism */
  //TODO trustVM
  //fbt_init_sec_syscalls();

  DUMP_START();
  DEBUG_START();

  struct thread_local_data *tld = fbt_init_tls();

  fbt_initialize_trampolines(tld);
  
  /* if the user program specifies a custom opcode table we overwrite the
   * default table overwriting is done because of performance reasons. this way
   * the position of the table_onebyte does not change in memory and therefore
   * the address can be hardcoded at compile time */ 
  if (opcode_table != NULL) {
    fbt_memcpy(opcode_table_onebyte, opcode_table,
               0x100 * sizeof(struct ia32_opcode));
  }

#if defined(AUTHORIZE_SYSCALLS)
  fbt_init_syscalls(tld);
#endif

#if defined(VERIFY_CFTX)
  /* check if we loaded some new code */
  fbt_rescan_dsos(tld);
#endif  /* VERIFY_CFTX */
  
  return tld;
}

void fbt_exit(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("fbt_exit(tld=%p)\n", tld);
  assert(tld != NULL);
  
  fbt_mem_free(tld);

  PRINT_DEBUG_FUNCTION_END(" ");
  DUMP_END();
  DEBUG_END();
}

void fbt_start_transaction(struct thread_local_data *tld,
                           void (*commit_function)()) {
  PRINT_DEBUG_FUNCTION_START("fbt_start_transaction(commit_function = %p)",
                             (void*)commit_function);

  /* set memory address of the stub of fbt_commit_transaction in the client
     program */
  fbt_ccache_add_entry(tld, (void*)commit_function, (void*)fbt_end_transaction);

  /* These functions are not transaction safe, therefore we catch them inside of
   * transactions and execute our own cover-functions that take care to return
   * to translated code */ 
#if defined(HIJACKCONTROL)
  // CLEANUP -> remove
  // TODO: keep this function as simple as possible!
  //fbt_ccache_add_entry(tld, (void*)dlsym, (void*)dlsym_handler);
  //fbt_ccache_add_entry(tld, (void*)dlvsym, (void*)dlvsym_handler);

  /* if thread fails to exit from the BT then we force-exit it */
  fbt_ccache_add_entry(tld, (void*)fbt_exit, (void*)fbt_exit);
#ifdef SECU_BLOCK_DL_ITERATE_PHDR
  // TODO: add REMOVE
  fbt_ccache_add_entry(tld, (void*)dl_iterate_phdr, (void*)fbt_do_nothing);
#endif /* SECU_BLOCK_DL_ITERATE_PHDR */
#endif  /* HIJACKCONTROL */

  /* find out return instruction pointer (=beginning of first TU)*/
  void *orig_begin = __builtin_return_address(0);
  char *transl_begin = fbt_translate_noexecute(tld, orig_begin);

  PRINT_DEBUG("starting transaction at %p (orig. addr: %p)\n",
              (long)transl_begin, (long)orig_begin);

  /* use a jump-back trampoline to jump to the translated code in the code cache
   */ 
  tld->ind_target = transl_begin;
  /* overwrite RIP to jump to tramp */
  *((ulong_t*)__builtin_frame_address(0)+1) = (ulong_t)tld->ret2app_trampoline;
  PRINT_DEBUG_FUNCTION_END(" ");
  /* avoid tail-call optimization (-foptimize-sibling-calls) for RIP change */
  asm volatile("" : : : "cc", "memory");
}

void fbt_commit_transaction() {
  llprintf("WARNING: Guest code managed to break out of secuBT"
           " sandbox!\nIf you can read this message, this means that "
           "untranslated code was executed between fbt_start_transaction() and "
           "fbt_commit_transaction().\n");
  DUMP_END();
  DEBUG_END();
}

void fbt_end_transaction() {
#if defined(FBT_STATISTIC)
  fbt_print_statistics();
#endif
  __asm__ volatile(""); 
}
