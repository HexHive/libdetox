/**
 * @file libfastbt.h
 * Interface to the binary translator which can be used from the outside.
 *
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
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
#ifndef LIBFASTBT_H
#define LIBFASTBT_H

#include "fbt_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIBNAME "./lib/libfastbt.so.0.3.1"
#define LIBLEN 26 /* length of LIBLEN+1 */

/**
 * Initialzes the binary translator.
 * Must be called once, before any other call to the binary translator
 * @param opcode_table user defined opcode table to be used instead of the
 * default table. If NULL is passed the default table is used.
 */
__attribute__((visibility("default"))) struct thread_local_data*
fbt_init(struct ia32_opcode *opcode_table);

/**
 * Shuts the BT down.
 * Must be called once for each thread.
 * @param tld pointer to thread local data
 */
__attribute__((visibility("default"))) void
fbt_exit(struct thread_local_data *tld);

/**
 * Initialize the transaction
 * @param tld pointer to thread local data
 * @param commit_function a function pointer to the function which indicates
 * end of transaction
 */
void fbt_transaction_init(struct thread_local_data *tld,
                          void (*commit_function)());

/**
 * Start a transaction.
 * The tansaction stops when a call 'commit function' is reached
 * @param tld pointer to thread local data
 * @param commit_function a function pointer to the function which indicates the
 * end of the transaction
 */
__attribute__((visibility("default"))) void
fbt_start_transaction(struct thread_local_data *tld, void (*commit_function)());

/**
 * Dummy function which can be used to pass to fbt_start_transaction
 */
__attribute__((visibility("default"))) void fbt_commit_transaction();

/**
 * Last function called in a transaction (right after finishing the
 * transaction).
 * The contents of this function are still executed in transactional mode but as
 * soon as this function returns we return to untranslated/unsandboxed code.
 */
__attribute__((visibility("default"))) void fbt_end_transaction();

#ifdef __cplusplus
}
#endif

#endif  /* LIBFASTBT_H */
