/**
 * @file fbt_restart_transaction.h
 * 
 * Code dealing with restarting a transaction, which resets the 
 * thread state such that translation starts anew.
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


#ifndef FBT_RESTART_TRANSACTION_H
#define FBT_RESTART_TRANSACTION_H

#include "fbt_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TRACK_CFTX)
/** Restarts the transaction of the target tld. This ensures that the thread is halted
    so the code cache can be flushed */
int fbt_restart_transaction(struct thread_local_data *tld, struct thread_local_data *target);

/**
 * Stores a control flow transfers using a relative jump (e.g. JMP, JE, ...) for
 * later reference.
 *
 * @param tld Thread local data
 * @param cft The control flow transfer to store. 
 *
 * @see control_flow_transfer
 */
void fbt_store_cftx(struct thread_local_data *tld, struct control_flow_transfer *cft);


/**
 * Interrupts all future control flow transfers by transfering to the trampoline 
 * located at <code>interruption_target</code> instead.
 *
 * Control flow transfers captured include everything registered using
 * <code>fbt_store_cftx</code> and all jumps out of trampolines. The trampoline
 * will be able to access <code>tld->interrupted_cft</code> to retrieve the address
 * of the interrupted control flow transfer instruction. If the variale is set to
 * NULL, the interrupted instruction was a jump out of a trampoline and
 * <code>tld->ind_target</code> should be used instead.
 * 
 * Note that this function will destroy the code cache as well as the trampolines
 * (reversible using <code>fbt_fix_trampolines_after_interrupt_cftx</code>).
 * 
 * @param tld Thread local data
 * @param interruption_target Trampoline to get control after interrupting 
 *                            a control flow transfer
 * @see fbt_store_cftx
 */
void fbt_interrupt_cftx(struct thread_local_data *tld, void *interruption_target);

/**
 * Fixes destroyed trampolines after using fbt_interrupt_cftx
 * 
 * @param tld Thread local data 
 */
void fbt_fix_trampolines_after_interrupt_cftx(struct thread_local_data *tld);

#endif /* TRACK_CFTX */

#ifdef __cplusplus
}
#endif


#endif