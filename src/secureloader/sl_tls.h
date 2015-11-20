/**
 * @file sl_tls.h
 * Implementation of methods needed for static and dynamic Thread Local Storage
 * (TLS). Static tls is composed of all TLS blocks that are available at
 * process startup. For DSOs loaded after process startup, only dynamic tls
 * is allowed.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

#ifndef SL_TLS_H
#define SL_TLS_H

#include "sl_macros.h"
#include "sl_datatypes.h"

/**
 * Syscall trampoline to int80.
 */
void sysinfo_int80 ();

/* Global loader-specific data (read/write) */
extern struct rtld_global *rtld_glob;

/* Global loader-specific data (readonly) */
extern struct rtld_global_ro *rtld_glob_ro;

/**
 * Sets up the tls data structures for all objects loaded at startup.
 * @return pointer to the thread control block (tcb)
 */
thread_control_block *setup_tls();

/**
 * Adds the given shared object containing tls to the slotinfo list. May
 * increase its size.
 * @param so Shared object to be added
 */
void add_to_slotinfo (dso *so);


/* --- Exported functions --- */

/**
 * Allocates memory for the dtv, maps tls init image for all loaded objects
 * and sets up dtv.
 * @param tcb Thread Control Block
 * @return pointer to the tcb
 */
void *allocate_tls(void *tcb) REG_ARGS(1);

/**
 * Maps the tls init image for all loaded objects and sets up the dynamic
 * thread vector (dtv).
 * @param tcb Thread Control Block
 * @return pointer to the tcb
 */
void *allocate_tls_init(void *tcb) REG_ARGS(1);

/**
 * Get size and alignment of static tls block.
 * @param sizep Size of the block
 * @param alignp Alignment of the block
 */
void get_tls_static_info(unsigned long *sizep, unsigned long *alignp)
  REG_ARGS(2);

/**
 * Determines the thread-specific address of a tls variable.
 * @param ti Struct containing module and offset of variable
 * @return address of tls variable
 */
void *tls_get_addr(struct tls_index *ti) REG_ARGS(1);

/**
 * Free all tls for the given thread.
 * @param tcb Thread Control Block
 * @param dealloc_tcb 1 if tcb should be freed, 0 otherwise
 */
void deallocate_tls(void *tcb, unsigned long dealloc_tcb) REG_ARGS(2);

/**
 * This function is not implemented.
 */
void *tls_get_addr_2(struct tls_index *ti);

/**
 * This function is not implemented.
 */
long tls_setup(void);

#endif
