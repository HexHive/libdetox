/**
 * @file sl_so_chain.c
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

#include "sl_libc.h"
#include "sl_so_chain.h"
#include "sl_macros.h"
#include "sl_datatypes.h"

/* The chain of loaded DSOs */
dso *so_chain;


dso *chain_search(const char *name) {
  dso *so;  

  /* Iterate over all loaded DSOs and compare name */
  for (so=so_chain; so!=0; so=so->next) {
    if (sl_strncmp(name, so->name, MAX_LIB_NAME) == 0) {
      return so;
    }
  }
  return 0;
}


void chain_add(dso *so) {
  if (so_chain == 0) {
    /* First element */
    so_chain = so;
  } else {
    /* Go to last element */
    dso *last;    
    for (last=so_chain; last->next!=0; last=last->next);

    /* Insert after last element and reprotect */
    so->prev = last;
    UNPROT(last);
    last->next = so;
    PROT(last);
  }
}


void chain_delete(dso *so) {
  if (so->prev) {
    /* Not first element */
    if (so->next) {
      /* Not last element */
      UNPROT(so->next);
      so->next->prev = so->prev;
      PROT(so->next);
    }
    UNPROT(so->prev);
    so->prev->next = so->next;
    PROT(so->prev);
  } else if (so->next) {
    /* First element */
    so_chain = so->next;
    UNPROT(so->next);
    so->next->prev = 0;
    PROT(so->next);
  }
}


void print_loaded_objects() {
  dso *so;

  /* Iterate over all loaded DSOs */
  for (so = so_chain; so != 0; so = so->next) {
    /* Print information about object and direct dependencies */
    sl_printf("%s, Base: %p End: %p Ref_count %d \n", so->name, so->base_addr,
              so->end_addr, so->ref_count);
    long i = 0;
    if (so->deps_count != 0) {
      sl_printf("Dependencies: ");
      sl_printf("[%s]", so->deps[i]->name);

      for (i = 1; i<so->deps_count; ++i){
        sl_printf(", [%s]", so->deps[i]->name);
      }
      sl_printf("\n\n");
    }
  }
}


dso *get_object_at(char *addr) {
  dso *so;
  /* Get DSO for which addr is inside memory block */
  for (so = so_chain; so != 0; so = so->next) {
    if ((char *)so->text_addr <= addr && addr <= (char *)so->end_addr) {
      return so;
    }
  }
  return 0;
}

