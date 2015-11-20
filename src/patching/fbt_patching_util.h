/**
 * @file fbt_patching_util.h
 * Header file for utility functions related to patching
 *
 * Copyright (c) 2011 ETH Zurich
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


#define BUFFER_SIZE 1024

/**
 * Gets the path to the executable that is currently executing.
 * @param target Target buffer for the string
 * @param len    Maximum number of bytes to write into target
 * @return Number of bytes written to target
 */
int fbt_get_executable_path(char *target, long len);

/** 
 * Gets path where a patch for the current exceutable would be located if it 
 * exists
 *  
 * @param target Target buffer
 * @param len    Maximum number of bytes to write into target
 * @return Number of bytes written to target
 */
int fbt_get_patch_path(char *target, long len);

/** 
 * Uses multiple read system calls from file descriptor <code>fd</code> 
 * to read <code>size</code> bytes and stores them continuously at 
 * <code>target</code>.
 * 
 * @param fd File descriptor to read from
 * @param target Memory location to write to
 * @param size Number of bytes to read
 */
int read_struct(long fd, void *target, int size);