/**
* Interface to the binary translator which can be used from the outside
*
* Copyright (c) 2008 ETH Zurich
*   Mathias Payer <mathias.payer@inf.ethz.ch>
*   Marcel Wirth <mawirth@student.ethz.ch>
*   Stephan Classen <scl@soft-eng.ch>
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
#define _GNU_SOURCE
#include <pthread.h>
#include <dlfcn.h>
#include <signal.h>

#include "libfastbt.h"
#include "fbt_private_datatypes.h"
#include "fbt_translate.h"
#include "fbt_asm_functions.h"
#include "fbt_tcache.h"
#include "fbt_signals.h"
#include "fbt_trampoline.h"
#include "fbt_libc.h"
//TODO sec syscalls trustVM
//#include "fbt_sec_syscalls.h"
#include "fbt_debug.h"

#if defined(FBT_STATISTIC)
#include "fbt_statistic.h"
#endif

#include "fbt_mem_protection.h"

/* is located in "fbt_opcode_tables.h" can not be included because of multiple
   occurences */
extern ia32_opcode_t opcode_table_onebyte[];

/* allows us not to push the real tld address onto the stack */
void *fbt_random_tld_offset;

#pragma GCC visibility push(default)

inline thread_local_data_t* get_tld()
{
  /* get the thread local data */
  thread_local_data_t *tld = (thread_local_data_t*)pthread_getspecific(tld_key);

  /* if there has not been set up a tld for this thread then do it now */
  if (tld == NULL) {
    tld = tcache_init();
    pthread_setspecific(tld_key, tld);

    /* create the special, persistent jump-back trampoline */
    create_jumpback_trampoline(tld);
  }

  return tld;
}

/* this function is called in fbt_asm_functions.S, .section .init */
void fbt_init(ia32_opcode_t *opcode_table)
{
  /* secuBT: init the memory protection */
  fbt_memprotect_init();

  /* secuBT: call init function for the secure system call mechanism */
  //TODO trustVM
  //fbt_init_sec_syscalls();

  /* init the pthread key for the thread local data */
  if (tld_key!=0 && pthread_getspecific(tld_key)!=NULL) {
    llprintf("TLD already exists - seems as if we want to run a transaction "
             "inside a transaction (%p - %p)!\n",
             (void*)tld_key, pthread_getspecific(tld_key));
    ffflush();
    fbt_suicide();
  }

  DUMP_START;
  DEBUG_START;

#if defined(LDPRELOAD)
  initSignals();
#endif
  /*
  * if the user program specifies a custom opcode table we overwrite the default
  * table overwriting is done because of performance reasons. this way the
  * position of the table_onebyte does not change in memory and therefore the
  * address can be hardcoded at compile time
  */
  if (opcode_table != NULL) {
    fbt_memcpy(opcode_table_onebyte, opcode_table, 0x100*sizeof(ia32_opcode_t));
  }

  /*
  * The fbt_commit_transaction argument is there to catch exiting threads.
  * If we don't catch this we could run into problems, because whenever a thread
  * exits it's thread local storage get's cleaned up and we loose our thread
  * local data (the struct with our tcache pointers and so on), then we either
  * run into exceptions or just crash if assertions are turned
  * off. fbt_commit_transaction is only needed if we use LD_PRELOAD, otherwise
  * we can skip it (although it acts as a last guard against a bt bug)
  */
  if (tld_key==0) {
#if defined(HIJACKCONTROL)
    if (pthread_key_create(&tld_key, end_transaction_clean) != 0) {
#else
    if (pthread_key_create(&tld_key, NULL) != 0) {
#endif
      fbt_suicide_str("Error creating thread local key\n");
    }
  } else {
    PRINT_DEBUG("TLD key already exists\n");
  }
}

void fbt_exit()
{
  thread_local_data_t* tld = get_tld();
  PRINT_DEBUG_FUNCTION_START("fbt_exit(tld=%p)\n",tld);
  assert(tld!=NULL);
  
  tcache_free(tld);
  pthread_key_delete(tld_key);

  PRINT_DEBUG_FUNCTION_END(" ");
}

void fbt_start_transaction(void (*commit_function)())
{
  PRINT_DEBUG_FUNCTION_START("fbt_start_transaction(commit_function = %p)",
                             (void*)commit_function);

  /* get thread local data */
  thread_local_data_t *tld = get_tld();

  /* TODO create jumpback trampoline */
  // create_jumpback_trampoline(tld);

  /* set memory address of the stub of fbt_commit_transaction in the client
     program */
  tcache_add_entry(tld, (void*)commit_function, (void*)end_transaction);

  /*
  * These functions are not transaction safe, therefore we catch them inside of
  * transactions and execute our own cover-functions that take care to return to
  * translated code
  */
#if defined(LDPRELOAD)
  tcache_add_entry(tld, (void*)signal, (void*)fbt_signal);
  tcache_add_entry(tld, (void*)sigaction, (void*)fbt_sigaction);
  tcache_add_entry(tld, (void*)pthread_create, (void*)fbt_pthread_create);
  tcache_add_entry(tld, (void*)dlsym, (void*)dlsym_handler);
  tcache_add_entry(tld, (void*)dlvsym, (void*)dlvsym_handler);
  tcache_add_entry(tld, (void*)end_transaction_clean,
                   (void*)end_transaction_clean);
#ifdef SECU_BLOCK_DL_ITERATE_PHDR
  tcache_add_entry(tld, (void*)dl_iterate_phdr, (void*)fbt_do_nothing);
#endif /* SECU_BLOCK_DL_ITERATE_PHDR */
#endif  /* LD_PRELOAD */

  /* find out return instruction pointer (=beginning of first TU)*/
  void *tu_begin = read_rip();

  /* check if this code has already been translated */
  void *ccf_begin = tcache_find(tld, tu_begin);

  /* if not then do it now */
  if (ccf_begin == NULL) {
    /* translate the TU if not already in tcache */
    ccf_begin = translate_noexecute(tld, tu_begin);
  }

  PRINT_DEBUG("starting transaction at 0x%x (orig: 0x%x)\n", (int32_t)ccf_begin, (int32_t)tu_begin);

  /* use a jump-back trampoline to jump to the first instruction in the CCF */
  tld->ind_jump_target = ccf_begin;

  FBT_IDS_LOCKDOWN(tld);
  change_rip(tld->ret_jumpback_tramp);    /* overwrite RIP to jump to tramp */

  /* avoid tail-call optimization (-foptimize-sibling-calls) for change_rip() */
  __asm__ volatile("");

  PRINT_DEBUG_FUNCTION_END(" ");
}

void fbt_add_translated_function(void *original_address,
                                  void *translated_address)
{
  PRINT_DEBUG_FUNCTION_START("fbt_add_translated_function(original_address = %p, translated_address = %p)", original_address, translated_address);

  /* get thread local data */
  thread_local_data_t *tld = get_tld();

  tcache_add_entry(tld, original_address, translated_address);

  PRINT_DEBUG_FUNCTION_END(" ");
}

void fbt_set_thread_specific_app_data(void* data)
{
  PRINT_DEBUG_FUNCTION_START("fbt_set_thread_specific_app_data(data = %p)",
                             data);

  /* get thread local data */
  thread_local_data_t *tld = get_tld();

  tld->ts.app_data = data;

  PRINT_DEBUG_FUNCTION_END(" ");
}


void* fbt_get_thread_specific_app_data()
{
  PRINT_DEBUG_FUNCTION_START("fbt_get_thread_specific_app_data()");

  /* get thread local data */
  thread_local_data_t *tld = get_tld();

  PRINT_DEBUG_FUNCTION_END("-> %p", tld->ts.app_data);
  return tld->ts.app_data;
}



void fbt_commit_transaction ()
{
  llprintf("WARNING: Guest code managed to break out of secuBT sandbox!\n"
           "If you can read this message, this means that untranslated "
           "code was executed between fbt_start_transaction() and "
           "fbt_commit_transaction().\n");

  DUMP_END;
  DEBUG_END;
}


#pragma GCC visibility pop
