/**
 * @file sl_sym_lookup.h
 * Implements the symbol lookup functionality.
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

#ifndef SL_SYM_LOOKUP_H
#define SL_SYM_LOOKUP_H

#include <elf.h>
#include "sl_datatypes.h"


/**
 * Tries to resolve the symbol with the given name by searching the symbol
 * tables of all DSOs in the different scopes of the initiating DSO.
 * Returns the address of the symbol if found, otherwise NULL;
 * @param so_start Shared object which needs the symbol
 * @param name Name of the symbol to search
 * @param version Version of the symbol
 * @param global_scope Search in global scope? 
 * @param skip_main Skip the main DSO?
 * @param sym_found Elf32_Sym of the found symbol, NULL if not found
 * @param so_found struct shared_object of the DSO the symbol is found in
 * @return address of symbol on success, NULL otherwise
 */
void *resolve_symbol(dso *so_start, const char *name,
                     const struct version_info *version,
                     unsigned char global_scope, unsigned char skip_main,
                     Elf32_Sym **sym_found, dso **so_found);

/**
 * Resolves the plt relocation at "offset" of the given shared_object and
 * returns the resolved address.
 * @param so Initiating shared object
 * @param offset Offset in the relocation table
 * @return address of the resolved symbol
 */
unsigned long resolve_plt(dso *so, unsigned long offset);

/**
 * This is called by the runtime_trampoline and resolves
 * the symbol specified by the arguments on the stack.
 * Returns the resolved address.
 * @return resolved address
 */
unsigned long resolve();
  
/**
 * This is a wrapper function for resolve_symbol which supports some more
 * functionality and is used by the glibc.

 * Tries to resolve the symbol with the given name by searching the symbol
 * tables of all DSOs in the different scopes of the initiating DSO.
 * Returns the link_map of the DSO the symbol was found in on success or
 * throws an error otherwise.
 * @param undef_name Name of the symbol
 * @param undef_map Initiating DSO
 * @param ref Pointer to the symbol if found
 * @param symbol_scope Scope of the symbol
 * @param version Version of the symbol
 * @param type_class (not supported)
 * @param flags Different lookup modes
 * @param skip_map DSO to skip while searching
 * @return struct link_map of the DSO the symbol was found in on success
 */
struct link_map * lookup_symbol_x
  (const char *undef_name, struct link_map *undef_map, const Elf32_Sym **ref,
   struct r_scope_elem *symbol_scope[], const struct version_info *version,
   long type_class, long flags, struct link_map *skip_map) REG_ARGS(3);


/**
 * Tries to resolve name and file where the function at "address" is located.
 * The information is stored in the Dl_info structure.
 * @param address Function pointer
 * @param info Dl_info structure the information is stored in.
 * @param mapp Pointer to link_map of the object
 * @param symbolp Pointer to symbol
 * @return 1 on success, 0 otherwise
 */
long REG_ARGS(3) dl_addr(const void *address, Dl_info *info,
                         struct link_map **mapp, const Elf32_Sym **symbolp);

/**
 * Walks through the list of loaded dso's and calls the function "callback" for
 * each object, until all objects have been processed or callback returns true.
 * @param callback The callback function
 * @param data Data passed to the callback function
 * @param 1 on success, 0 otherwise
 */
long dl_iterate_phdr (long (*callback) (struct dl_phdr_info *info,
                                        unsigned long size, void *data),
                      void *data);

#endif

