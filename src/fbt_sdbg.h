/**
 * @file fbt_sdbg.h
 * Datatypes used for communication with selDebug
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2013-02-13 23:03:12 +0100 (Wed, 13 Feb 2013) $
 * $LastChangedDate: 2013-02-13 23:03:12 +0100 (Wed, 13 Feb 2013) $
 * $LastChangedBy: payerm $
 * $Revision: 1591 $
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
#ifndef FBT_SDBG_H
#define FBT_SDBG_H

/* this header file is from selDebug (see Makedefs.extensions) */
//#include "ipc.h"

#include "fbt_datatypes.h"

#define INVALID_FD (-1)

/**
 * Initialize shared data between libdetox and selDebug
 * @return pointer to shared data
 */
struct sdbg_thread_local_data *sdbg_init();

/**
 * Handle current translated instruction
 * Insert watchpoints and breakpoints if specified
 * @param tld pointer to thread local data
 */
//void sdbg_handle_instruction(struct thread_local_data *tld);
void sdbg_handle_flush_and_breaks(struct thread_local_data *tld);
void sdbg_handle_watchpoints(struct thread_local_data *tld);

#endif /* FBT_SDBG_H */
