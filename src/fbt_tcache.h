/**
 * This is the tcache module, where all translated code snipplets are put
 * including the hashtable which maps between translated and untranslated
 * code (e.g. to find the already translated block in the code cache)
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
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
#ifndef FBT_TCACHE_H
#define FBT_TCACHE_H

/** maximum size of a compiled code fragment in bytes */
#define MAX_CCF_SIZE 128

/** guarded bytes at the end of the tcache.
    because we might override the max ccf size when we
    translate an additional jump or so */
#define CCF_GUARD 512

#ifdef FBT_RET_STACK
#define FBT_RET_STACK_SIZE 2048
#endif

/**
 * translation hash table entry
 */
struct tcache_entry {
	void *src;
	void *dst;
};
typedef struct tcache_entry tcache_entry_t;


thread_local_data_t *tcache_init();

void tcache_free(thread_local_data_t *tld);

void *tcache_find(thread_local_data_t *tld, void *tu_address);

void tcache_put(thread_local_data_t *tld, void *tu_address);

void tcache_add_entry(thread_local_data_t *tld, void *tu_address, void *ccf_address);

#endif /* FBT_TCACHE_H */
