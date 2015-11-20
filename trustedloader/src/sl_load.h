/**
 * @file sl_load.h
 * Implements the loading of elf files.
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

#ifndef SL_LOAD_H
#define SL_LOAD_H

#include "sl_datatypes.h"

/**
 * Reads all necessary information from an elf file, loads it into
 * memory, does all the relocations and resolves all the dependencies to other
 * shared objects.
 * @param loader The loader of this file
 * @param name Name of the elf file
 * @param path Path to the elf file
 * @param fd File descriptor of file
 * @param rt_load 1 if file is loaded at runtime, 0 otherwise
 * @return struct shared_object
 */
dso *load_elf(dso *loader, const char *name, const char *path, long fd,
              unsigned char rt_load);

/**
 * Calls the fini functions of the given shared object, removes it from all
 * search scopes, unmaps the text and data segments and frees all memory.
 * @param so Shared object
 */
void unload_elf(dso *so);

/**
 * Calls init functions of the shared object with the given parameters.
 * @param so The shared object
 * @param argc Number of arguments
 * @param argv Pointer to arguments
 * @param envp Pointer to environment variables
 */
void so_init(dso *so, long argc, char **argv, char **envp);

/**
 * Calls fini functions of the given shared object.
 * @param so The shared object
 */
void so_fini(dso *so);

#endif
