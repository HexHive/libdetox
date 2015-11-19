/**
 * This is the statistics module.
 * If FBT is compiled with statistics, then they will be collected here.
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
#ifndef FBT_STATISTIC_H
#define FBT_STATISTIC_H

unsigned long long fbt_nr_tcache_fast_lookups;
unsigned long long fbt_nr_tcache_slow_lookups;
unsigned long long fbt_nr_tcache_fast_miss;

unsigned int fbt_nr_ccf;
unsigned int fbt_nr_tcache_direct;
unsigned int fbt_nr_tcache_1;
unsigned int fbt_nr_tcache_2;
unsigned int fbt_nr_tcache_4;
unsigned int fbt_nr_tcache_8;
unsigned int fbt_nr_tcache_8p;
unsigned int fbt_nr_translated_instr;
unsigned int fbt_nr_translated_jcc;
unsigned int fbt_nr_translated_jmp;
unsigned int fbt_nr_translated_jmp_ind;
unsigned int fbt_nr_translated_call;
unsigned int fbt_nr_translated_call_ind;

unsigned long long fbt_nr_ind_jump;
unsigned long long fbt_nr_ind_jump_remove;
unsigned long long fbt_nr_ind_jump_backpatch;
unsigned long long fbt_nr_ind_jump_backpatch_abs;
unsigned long long fbt_nr_ind_jump_chaining;

unsigned long long fbt_nr_call;

#if defined(FBT_RET_PREDICTION)
unsigned long long fbt_nr_ret_predict_miss;
unsigned long long fbt_nr_ret_predict_miss_remove;
#endif

#if defined(FBT_RET_STACK)
unsigned long long fbt_nr_ret_stack_synchronize;
#endif

#if defined(FBT_RET_FAST)
unsigned long long fbt_nr_ret_fast;
unsigned long long fbt_nr_ret_fast_miss;
#endif

#if defined(FBT_INLINE_CALLS)
unsigned int fbt_nr_trans_inlined_calls;
unsigned long long fbt_nr_inlined_calls;
#endif

unsigned long long fbt_nr_ind_calls;
#if defined(FBT_IND_CALL_PREDICTION)
unsigned long long fbt_nr_ind_calls_miss;
#endif

void print_statistics();

#endif
