/**
 * @file sl_reloc.h
 * Implements the relocating of ELF files.
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

#ifndef SL_RELOC_H
#define SL_RELOC_H

#include "sl_datatypes.h"
#include "fbt_libc.h"

/** 
 * Relocates all loaded dynamic shared objects.
 */
void do_relocations();

/**
 * Relocates the given shared object.
 * @param so The shared object
 * @param rt_load 1 if file is loaded at runtime, 0 otherwise
 */
void relocate(dso *so, unsigned char rt_load);

/**
 * Perform all relocations associated with the plt.
 * @param so The shared object
 */
void relocate_plt(dso *so);


#endif

