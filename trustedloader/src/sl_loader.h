/**
 * @file sl_loader.h
 * Implements the main functionality of the secuLoader. 
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

#ifndef SL_LOADER_H
#define SL_LOADER_H

#include "sl_datatypes.h"
#include "sl_macros.h"


/**
 * This libc malloc implementation which is needed in dl_signal_error.
 */
void *(*libc_malloc)(long size);

/**
 * Returns some static memory for the catch structure needed to safe state     
 * in catch_error. This function is later replaced by the thread library.
 */
void **error_catch_tsd (void) __attribute__ ((const));

/**
 * Calls the function "operate" and catches errors thrown by signal_error.
 * @param objname Name of the object which throwed the error
 * @param errstring Error message
 * @param malloced Are strings malloced by libc malloc?
 * @param operate The function which may throw an error
 * @param args Arguments for operate
 * @return error code
 */
long REG_ARGS(3) catch_error(const char **objname, const char **errstring,
                             long *mallocedp, void (*operate) (void *),
                             void *args);

/**
 * Throws an error (longjump) which is then catched by catch_error.
 * @param errcode Error code
 * @param objname Name of the object which throws the error
 * @param occasion Reason of the error
 * @param errstring Error message
 */
void REG_ARGS(3) signal_error(long errcode, const char *objname,
                              const char *occasion, const char *errstring);

/**
 * Loads the dynamic shared object specified by "file" and returns a 
 * handle for it. If file is NULL the handle is for the main program.
 * @param file Path or filename of DSO
 * @param mode Different modes
 * @param caller_dlopen The caller function
 * @param nsid Namespace id
 * @param argc Number of command line arguments
 * @param argv Pointer to command line arguments
 * @param env Pointer to environment variables
 * @return struct shared_object
 */
void *dl_open(const char *file, long mode, const void *caller_dlopen,
               long nsid, long argc, char *argv[], char *env[]);

/**
 * Unloads the dynamic shared object specified by the given handle if 
 * the internal reference counter is zero.
 * @param map Handle to the object obtained by dl_open
 */
void dl_close(void *map);

/**
 * Default lock function.
 * Increments counter of the given lock.
 * @param lock The mutex
 */
void lock_recursive(void *lock);

/**
 * Default unlock function.
 * Decrements counter of the given lock.
 * @param lock The mutex
 */ 
void unlock_recursive(void *lock);


/*                                                                              
 ----------------------------------------------------                           
 Note: The following functions are not yet supported.                           
 ----------------------------------------------------                           
*/

void debug_printf(const char *fmt, ...);
void mcount_internal(Elf32_Addr frompc, Elf32_Addr selfpc);
long check_caller(const void *caller, enum allowmask mask);
void *tls_get_addr_soft(struct link_map *l);
long discover_osversion(void);
long make_stack_executable_hook(void **stack_endp) REG_ARGS(1);
void init_static_tls (struct link_map *map);
void wait_lookup_done (void);

#endif
