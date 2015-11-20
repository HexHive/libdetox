/**
 * @file fbt_patching_socket.c
 * Patching thread implementation accepting patches
 * from a socket
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

#include "fbt_patching_socket.h"
#include "fbt_patching.h"

#include "../fbt_datatypes.h"
#include "../fbt_debug.h"
#include "../fbt_libc.h"
#include "../fbt_llio.h"
#include "../fbt_mem_mgmt.h"
#include "../fbt_syscalls_impl.h"

/* -----------------------------------------------------------------------------
   Data structures needed for socket calls 
   ---------------------------------------------------------------------------*/
#ifdef ONLINE_PATCHING_SOCKET

extern uint32_t htonl (uint32_t __hostlong)
     __THROW __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
     __THROW __attribute__ ((__const__));

#define PF_INET   2
#define AF_INET   PF_INET
#define SOCK_STREAM 1

typedef int sa_family_t;
typedef int in_port_t;

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr_in {
  short             sin_family;   /* AF_INET */
  unsigned short    sin_port;     /* 16-bit TCP or UDP port number */
                                  /* network byte ordered */
  struct in_addr    sin_addr;     /* 32-bit IPv4 address */
                                  /* network byte ordered */
  char              sin_zero[8];
};

#define INADDR_ANY ((unsigned long int) 0x00000000)

#define fbt_online_patching_thread fbt_online_patching_run_server
int fbt_online_patching_run_server(void *shared_data) {
  struct thread_local_data *tld = ((struct shared_data *)shared_data)->patching_tld;

  PRINT_DEBUG("fbt_online_patching_run_server\n");
  
  int listen_socket;
  fbt_socket(AF_INET, SOCK_STREAM, 0, listen_socket);
  
  PRINT_DEBUG("Socket result was: %d\n", listen_socket);    
  if (listen_socket < 0) {
      PRINT_DEBUG("Opening stream socket failed.");
      return 1;
  }    
  
  // Bind
  struct sockaddr_in fbt_addr;
  fbt_addr.sin_family = AF_INET;
  fbt_addr.sin_port = htons(ONLINE_PATCHING_PORT);
  fbt_addr.sin_addr.s_addr = htonl(INADDR_ANY);    
  
  int status;
  fbt_bind(listen_socket, (struct sockaddr *)&fbt_addr, sizeof(struct sockaddr_in), status);

  if (status == -1) {
      PRINT_DEBUG("bind() failed\n");
      return 1;
  }    
  
  PRINT_DEBUG("bind() successful on port %d.\n", ONLINE_PATCHING_PORT);
  
  fbt_listen(listen_socket, 5, status);
  
  if (status == -1) {
      PRINT_DEBUG("listen() failed\n");
      return 1;
  }
  
  PRINT_DEBUG("listen() successful.\n");
  
  struct sockaddr_in client_address;
  int client_length = sizeof(client_address);
  
  while (1) {
      int client_socket;
      fbt_accept(listen_socket, (struct sockaddr *)&client_address, &client_length, client_socket);    

      if (client_socket < 0) {
          PRINT_DEBUG("accept() failed.\n");
          return 1;
      }
      
      PRINT_DEBUG("Accepted a connection.\n");    
          
      fbt_apply_patch_from_file(tld, client_socket, OP_ONLINE);
      
      int res;
      fbt_closeE(client_socket, res);
  }    
}
#endif