/**
 * @file sl_so_chain.h
 * Implements the global chain of loaded DSOs.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
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

#ifndef SL_SO_CHAIN_H
#define SL_SO_CHAIN_H


/**
 * Searches in global chain for an shared object with the given name.
 * @param name name of shared object
 * @return dso
 */
dso *chain_search(const char *name);

/**
 * Adds the given shared object to the chain.
 * @param so shared object to be added
 */
void chain_add(dso *so);

/**
 * Removes the given shared object from the chain.
 * @param so shared object to be removed
 */
void chain_delete(dso *so);

/**
 * Returns the DSO to which the given address belongs.
 * @param addr the address
 * @return dso on success, NULL otherwise
 */
dso *get_object_at(char *addr);

/**
 * Helper function which prints all loaded shared objects.
 */
void print_loaded_objects();
  

#endif
