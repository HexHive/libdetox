/**
 * @file sl_libsearch.h
 * Implements functions to search for needed shared libraries.
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


#ifndef SL_LIBSEARCH_H
#define SL_LIBSEARCH_H

#include "sl_datatypes.h"

/**
 * Decomposes a path string of the form "/path1/:/path2/" and stores the 
 * result in a r_search_path_elem struct.
 * @param path String containing the paths
 * @param where Object which contains the string
 * @param what Type of the search paths
 * @return struct r_search_path_elem
 */
struct r_search_path_elem *decompose_path(const char *path, const char *where,
                                          const char *what);

/**
 * Searches for the library called name.
 * Returns a file descriptor and sets path if found.
 * @param loader Shared object which needs the library
 * @param name The name of the library
 * @param path Is set to the path where the library is found
 * @return file descriptor
 */
long search_lib(dso *loader, const char *name, char **path);

#endif
