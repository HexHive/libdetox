/**
 * @file fbt_patching_socket.c
 * Patching thread implementation accepting patches
 * from a socket
 *
 * Copyright (c) 2011 ETH Zurich
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


#ifdef ONLINE_PATCHING_SOCKET

#define fbt_online_patching_thread fbt_online_patching_run_server

/** 
 * Runs a patching thread that listens on a socket
 * for incoming connections and accepts the arriving
 * data as apatch.
 *
 * Patches could be applied e.g. by using netcat
 *
 * @param shared_data Pointer to shared data
 */
int fbt_online_patching_run_server(void *shared_data);

#endif