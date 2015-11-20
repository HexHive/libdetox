/**
 * @file sl_libdetox.h
 * Communication with the libdetox framework.
 * Implements functions to load the libdetox library and supplies all 
 * needed information for the control transfer checks.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
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


#ifndef SL_LIBDETOX_H
#define SL_LIBDETOX_H

#ifdef ATTACH_LIBDETOX

#include "sl_datatypes.h"
#include "fbt_datatypes.h"
#include "fbt_dso.h"


/**
 * Loads libdetox. The libdetox DSO is treated specially by the secuLoader.
 * @return struct shared_object of libdetox library
 */
dso *load_libdetox();

/**
 * Initializes libdetox.
 * @param opcode_table user defined opcode table to be used instead of the
 * default table. If NULL is passed the default table is used.
 */
struct thread_local_data *(*libdetox_init)(void *opcode_table);

/**
 * Start a libdetox transaction.
 * The tansaction stops when a call 'commit function' is reached.
 * @param tld pointer to thread local data
 * @param commit_function a function pointer to the function which indicates the
 * end of the transaction
 */
void (*libdetox_start_transaction)(struct thread_local_data *tld,
                                     void (*commit_function)());

/**
 * Dummy function which can be used to pass to fbt_start_transaction.
 */
void (*libdetox_commit_transaction)();

/**
 * Reads all necessary information and adds the given dynamic shared object to the
 * libdetox internal list of loaded DSOs. This is needed for the control flow 
 * transfer checks.
 * If so = NULL, filebase points to the secuLoader itself.
 * @param so the dynamic shared object
 * @param filebase start address of file in memory
 */
void add_dso (dso *so, char *filebase);

/**
 * Removes the given DSO from the libdetox internal list of loaded DSOs.
 * @param so the DSO to remove
 */
void remove_dso (dso *so);

/**
 * Checks if a call to the given addr is a plt call and if so resolves the address
 * in the destination DSO and returns it. If not the call addr is returned.
 * @param addr call destination
 * @return resolved addr if plt call, call addr otherwise
 */
unsigned long sl_resolve_plt_call(unsigned long addr);


#endif /* ATTACH_LIBDETOX */

#endif /* SL_LIBDETOX_H */
