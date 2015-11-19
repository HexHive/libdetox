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
#ifndef LIBFASTBT_H
#define LIBFASTBT_H

#include "fbt_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialzes the binary translator
 * Must be called once, before any other call to the binary translator
 * @param opcode_table user defined opcode table to be used instead of the default table. If NULL is passed the default table is used.
 */
void fbt_init (ia32_opcode_t *opcode_table);

/**
 * Shuts the BT down
 * Must be called once for each thread.
 */
void fbt_exit();

/**
 * start a transaction.
 * The tansaction stops when a call 'commit function' is reached
 * @param commit_function a function pointer to the function which indicates the end of the transaction
 */
void fbt_start_transaction(void (*commit_function)());
/**
 * dummy function which can be used to pass to fbt_start_transaction
 */
void fbt_commit_transaction();

/**
 * Starts translating the code.
 * Note: the caller of this function is responsible to add a return function befor calling bt_start otherwise
 * the binary translater will never stop translating
 */
void fbt_start();

/**
 * This function allows to add pre translated functions to the internal cache.
 * Whenever a call to a pre translated function is encountered the code is not translated but executed directly.
 * Note:
 *   - To add a function which will return back to the untranslated code simply pass it in as both original and translated.
 *   - To ensure that when returning from a pretranslated function the execution stays in the translated code
 *     one must make a 'call' to ind_jmp instead of a 'ret'
 * @param original_address a pointer to the function in the untranslated code
 * @param translated_address a pointer to the code which should be executed instead of a translated version of 'original_address'
 */
void fbt_add_translated_function(void *original_address, void *translated_address);

/**
 * This function allows to add pre translated functions to the internal cache, that do not by themselves return to the
 * translated return address. A trampoline is used to translate the return address before calling.
 * @param original_address a pointer to the function in the untranslated code
 * @param translated_address a pointer to the code which should be executed instead of a translated version of 'original_address'
 */
void fbt_add_translated_function_return_to_translated(void *original_address, void *translated_address);

/**
 * Allows sto set a application specific pointer
 * This pointer is accessable in the translation struct which is passed to the actions
 * Note: the pointer is thread local. That means every thread has its own application specfic pointer
 * @param data the pointer to set as the application specific pointer.
 */
void fbt_set_thread_specific_app_data(void* data);

/**
 * Read out the application specific pointer
 * @return the last pointer which was set with fbt_set_thread_specific_app_data or NULL if no pointer has been set yet.
 */
void* fbt_get_thread_specific_app_data();


inline thread_local_data_t* get_tld();

       /**
 * These function can be used in pre-translated functions to return to translated code.
 * If this function is not called from within a pretranslated function it will return to untranslated code.
 * What this function does is to change the return instruction pointer of the function from which it is called
 * The changed return pointer will point to the translated code. Therefore this function MUST be called from
 * the main body of the pre-translated function and not from a sub-function. Also it MUST NOT be called when
 * a thread is not in a transaction. If it is called the thread implizitly moves to the translated code when
 * returning form the caller
 */
  extern void set_return_to_translated_asm();

/* avoid tail-call optimization (-foptimize-sibling-calls) for set_return_to_translated() */
#define set_return_to_translated() \
	do { \
		set_return_to_translated_asm(); \
		__asm__ volatile(""); \
	} while(0)

#ifdef __cplusplus
}
#endif

#endif /* LIBFASTBT_H */
