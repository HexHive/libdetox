/**
 * @file sl_gscope.c
 * Implements the global search scope. All shared objects in this scope are 
 * searched when resolving symbols.
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


#include "sl_gscope.h"
#include "sl_datatypes.h"
#include "sl_macros.h"
#include "sl_libc.h"

struct shared_object *gscope = 0;


/**
 * Adds the given shared object to the global search scope.
 * @param so The shared object to add       
 */
static void gscope_addl(struct shared_object *so) {
  if (gscope == 0) {
    /* First element in scope */
    gscope = so;
  } else {
    /* Add to end of scope */
    struct shared_object *last_so;    
    for (last_so = gscope; last_so->gs_next != 0; last_so = last_so->gs_next);

    /* If memory protection is enabled, first unprotect last and new object and
       then add new object */
    UNPROT(so);
    so->gs_prev = last_so;
    PROT(so);
    
    UNPROT(last_so);
    last_so->gs_next = so;
    PROT(last_so);
  }
}


void gscope_add_recursive(struct shared_object *so) {
  unsigned long i = 0;
  /* We have to add the objects in a breadth-first order! */
  
  if (so) {
    /* If not already in scope, add object itself */
    if (!so->gs_next && !so->gs_prev) {
      gscope_addl(so);
    }

    /* First add direct dependencies, if not already in scope */
    for (i = 0; i < so->deps_count; ++i) {
      struct shared_object *dep = so->deps[i];
      if (!dep->gs_next && !dep->gs_prev) {
        gscope_addl(dep);
      }
    }

    /* Now add dependencies resursively */
    for (i = 0; i < so->deps_count; ++i) {
      gscope_add_recursive(so->deps[i]);
    }
  }
}


void gscope_remove(struct shared_object *so) {
  /* Unlink the object from the linked list. We have to be careful with memory
     protection.*/
  if (so->gs_prev) {
    if (so->gs_next) {
      UNPROT(so->gs_next);
      so->gs_next->gs_prev = so->gs_prev;
      PROT(so->gs_next);
    }
    UNPROT(so->gs_prev);
    so->gs_prev->gs_next = so->gs_next;
    PROT(so->gs_prev);
  } else if (so->gs_next) {
    gscope = so->gs_next;
    UNPROT(so->gs_prev);
    so->gs_next->gs_prev = 0;
    PROT(so->gs_prev);
  }
}

