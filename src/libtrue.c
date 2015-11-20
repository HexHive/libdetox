/**
 * @file libtrue.c
 * Entry point to the share library library.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

/* TODO: clean up definitions of include directories in Makedefs */
#include "fastbt/libfastbt.h"
#include "fastbt/fbt_code_cache.h"
#include "fastbt/fbt_llio.h"

/* Implement _init and _fini. */

static struct thread_local_data *tld;

void _init() {
#if defined(DEBUG) && !defined(SILENT_STARTUP)
  llprintf("Starting BT\n");
  llprintf("This is a debug build, so do not expect stellar performance!\n");
#endif
  
  tld = fbt_init(0);
  
  /* if thread fails to exit from the BT then we force-exit it */
  fbt_ccache_add_entry(tld, (void*)fbt_exit, (void*)fbt_exit);
    
  fbt_start_transaction(tld, fbt_commit_transaction);
}

void _fini() {
#if defined(DEBUG) && !defined(SILENT_STARTUP)
  llprintf("Stopping BT\n");
#endif /* DEBUG */
  fbt_commit_transaction();
  fbt_exit(tld);
}
