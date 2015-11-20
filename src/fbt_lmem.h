/**
 * @file fbt_lmem.h
 * Datatypes used for communication with selDebug
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Enrico Kravina <enrico.kravina@gmail.com>
 *
 * $Date: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedDate: 2012-01-18 12:17:00 +0100 (mer, 18 gen 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1184 $
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

#ifndef FBT_LMEM_H
#define FBT_LMEM_H

#include "fbt_datatypes.h"

/*
 * Basically enables/disable the whole memory tracing
 * functionality.
 */
#define ENABLE_MEMCHECKS

/*
 * In practice the 'flags' register is never used
 * throughout function boundaries. That is, no one
 * uses it as some sort of implicit parameter or
 * some sort of implicit return value. This switch
 * says that lmem can make this assumption to
 * get better performance. This makes sense since
 * lmem is meant mainly to debug and not for security.
 * A malicious program may detect the sandbox by
 * making a function that changes flags in a specific
 * way and let the caller check if this change has
 * been preserved.
 */
#define ENABLE_UNSOUND_ASSUMPTIONS

#define LMEM_SPECIAL_SYSCALL 500

enum syscall_auth_response lmem_syscall(struct thread_local_data *tld,
                                         ulong_t syscall_nr,
                                         ulong_t arg1,
                                         ulong_t arg2 __attribute__((unused)),
                                         ulong_t arg3 __attribute__((unused)),
                                         ulong_t arg4 __attribute__((unused)),
                                         ulong_t arg5 __attribute__((unused)),
                                         ulong_t *arg6 __attribute__((unused)),
                                         ulong_t is_sysenter __attribute__((unused)),
                                         ulong_t *retval __attribute__((unused)));

void lmem_init();

#endif
