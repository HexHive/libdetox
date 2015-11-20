/**
 * @file sl_map.h
 * Implements the loading of elf segments into memory.
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

#ifndef SL_MAP_H
#define SL_MAP_H

#include "sl_datatypes.h"

void set_page_protection (dso *so);

/**
 * Maps the two LOAD segments into memory with the necessary alignment.
 * @param fd File descriptor of file to load
 * @param segs Program headers of the two segments
 * @param type Type of object (executable or shared object)
 * @param so The DSO the segment
 */
void map_segments (long fd, Elf32_Phdr *segs[2], Elf32_Half type, dso *so);

/**
 * Check if the two LOAD segments were already mapped by the kernel. Do not map again.
 * @param fd File descriptor of file to load
 * @param segs Program headers of the two segments
 * @param type Type of object (executable or shared object)
 * @param so The DSO the segment
 * @param segs_auxv The program headers provided by the kernel
 */
void map_segments_RTLD (long fd, Elf32_Phdr *segs[2], Elf32_Half type, dso *so, Elf32_Phdr *segs_auxv[2]);

/**
 * Unmaps all segments of the given DSO.
 * @param so The DSO to unmap
 */
void unmap_segments(dso *so);

#endif
