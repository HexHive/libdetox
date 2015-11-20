/**
 * @file sl_map.h
 * Implements the loading of elf segments into memory.
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

#ifndef SL_MAP_H
#define SL_MAP_H

#include "sl_datatypes.h"

/**
 * Maps the two LOAD segments into memory with the necessary alignment.
 * @param fd File descriptor of file to load
 * @param segs Program headers of the two segments
 * @param type Type of object (executable or shared object)
 * @param so The DSO the segmen
 */
void map_segments (long fd, Elf32_Phdr *segs[2], Elf32_Half type, dso *so);

/**
 * Unmaps all segments of the given DSO.
 * @param so The DSO to unmap
 */
void unmap_segments(dso *so);

#endif
