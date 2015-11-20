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

#ifndef FBT_LMEM_SPILLING_H_
#define FBT_LMEM_SPILLING_H_

#ifdef LMEM

#include "fbt_lmem_instruction.h"

/**
 * @param instructions an array of instructions
 * @param ninstr array length
 * @param available_regs here the result is written, each gp. register
 *                       that can be freely used between instruction
 *                       i-1 and instruction i.
 */
void lmem_compute_spilling(struct lmem_instruction* instructions,
		                       int ninstr,
		                       unsigned* available_regs);

#endif

#endif /* FBT_LMEM_SPILLING_H_ */
