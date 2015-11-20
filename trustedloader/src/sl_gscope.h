/**
 * @file sl_gscope.h
 * Implements the global search scope. All shared objects in this scope are
 * searched when resolving symbols.
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


#ifndef SL_GSCOPE_H
#define SL_GSCOPE_H

#include "sl_datatypes.h"

/**
 * Adds the given shared object and all its dependencies recursively to the
 * global search scope. This must be done in a breadth-first order.
 * @param so The shared object to add
 */
void gscope_add_recursive(dso *so);

/**
 * Removes the given shared object from the global search scope.
 * @param so The shared object to remove
 */
void gscope_remove(dso *so);

#endif
