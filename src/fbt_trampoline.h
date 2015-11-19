/**
 * The trampoline module. Trampolines are used for conditional jumps
 * Trampolines are kept in a linked list and can be recycled after usage.
 *
 * A trampoline is used if a code region is not alraedy translated and the
 * trampoline will then handle the translation of this region and backpatch
 * the old code with the newly generated target
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
#ifndef FBT_TRAMPOLINE_H
#define FBT_TRAMPOLINE_H

#define NUM_TRAMPOLINES 256

void *trampoline_put(thread_local_data_t *tld, void* call_target, void* origin);

#ifdef FBT_RET_STACK
void *trampoline_put_abs(thread_local_data_t *tld, void *call_target, void *origin);
#endif

void *trampoline_put_chaining(thread_local_data_t *tld, void* call_target, void* origin);

void *trampoline_put_signal(void *call_target);

void *trampoline_put_return_to_translated(thread_local_data_t *tld, void *call_target);

void trampoline_free(thread_local_data_t *tld, trampoline_entry_t *slot);

#endif /* FBT_TRAMPOLINE_H */
