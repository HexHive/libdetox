/**
 * This handles statistics
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Antonio Barresi <abarresi@student.ethz.ch>
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

#ifdef FBT_STATISTIC

#include <stdio.h>
#include <dlfcn.h>

#include "libfastbt.h"
#include "fbt_debug.h"
#include "fbt_private_datatypes.h"
#include "fbt_statistic.h"

void print_statistics()
{
    FILE *statfile = fopen("fbt_stats.log", "a+");
    if (statfile==NULL) {
	perror("Could not open statistic file - bailing out!\n");
	return;
    }
    fprintf(statfile, "\ntcache statistics\n");
    fprintf(statfile, "=================\n");
#if defined(FBT_FIND_FAST)
    fprintf(statfile, "Fast tcache lookups:   %12llu\n", fbt_nr_tcache_fast_lookups);
#endif
    fprintf(statfile, "Slow tcache lookups:   %12llu\n", fbt_nr_tcache_slow_lookups);
#if defined(FBT_FIND_FAST)
    if(fbt_nr_tcache_fast_lookups != 0) fprintf(statfile, "Fast tcache misses:    %12llu (%f%%)\n\n", fbt_nr_tcache_fast_miss, (double)fbt_nr_tcache_fast_miss/fbt_nr_tcache_fast_lookups*100);
#endif
    
    fprintf(statfile, "nr of ccf:             %12d (basic blocks)\n", fbt_nr_ccf);
    fprintf(statfile, "tcache hashmap:        %12d (direct) %d (1) %d (2) %d (-4) %d (-8) %d (8-) (density: %f%%, non-optimal for: %f%%)\n", fbt_nr_tcache_direct, fbt_nr_tcache_1, fbt_nr_tcache_2, fbt_nr_tcache_4, fbt_nr_tcache_8, fbt_nr_tcache_8p, (double)fbt_nr_ccf/(HASHTABLE_SIZE>>3)*100, (double)(fbt_nr_tcache_1+fbt_nr_tcache_2+fbt_nr_tcache_4+fbt_nr_tcache_8+fbt_nr_tcache_8p)/fbt_nr_ccf*100);
    fprintf(statfile, "translated instrs:     %12d\n", fbt_nr_translated_instr);
    fprintf(statfile, "translated jccs:       %12d (conditional jumps)\n", fbt_nr_translated_jcc);
    fprintf(statfile, "translated jmps:       %12d\n", fbt_nr_translated_jmp);
    fprintf(statfile, "translated ind jmps:   %12d\n", fbt_nr_translated_jmp_ind);
#if defined(FBT_IND_PREDICTION)
    fprintf(statfile, "trans ind jump pred:   %12d (indirect - dynamic prediction)\n", fbt_nr_translated_jump_ind_pred);
    fprintf(statfile, "trans ij pred removed: %12d (indirect - dynamic prediction)\n", fbt_nr_translated_jump_ind_pred_removed);
#endif
    fprintf(statfile, "translated rets:       %12d\n", fbt_nr_translated_ret);
    fprintf(statfile, "translated calls:      %12d\n", fbt_nr_translated_call);
    fprintf(statfile, "translated ind calls:  %12d (indirect - no static prediction)\n", fbt_nr_translated_call_ind);
#if defined(FBT_IND_CALL_PREDICTION)
    fprintf(statfile, "translated ic pred:    %12d (indirect - dynamic prediction)\n", fbt_nr_translated_call_ind_pred);
    fprintf(statfile, "trans ic pred removed: %12d (indirect - dynamic prediction)\n", fbt_nr_translated_call_ind_pred_removed);
#endif
#if defined(FBT_INLINE_CALLS)
    fprintf(statfile, "inlined calls:         %12d (no call/ret overhead for these)\n", fbt_nr_trans_inlined_calls);
#endif
    
    fprintf(statfile, "\nIndirect jumps\n");
    fprintf(statfile, "==============\n");
    fprintf(statfile, "nr ind jump:           %12llu\n", fbt_nr_ind_jump);
    fprintf(statfile, "nr ind jump remove:    %12llu (called by ret that removes bytes)\n", fbt_nr_ind_jump_remove);
    fprintf(statfile, "nr ind jump backpatch: %12llu (trampoline removed)\n", fbt_nr_ind_jump_backpatch);
    fprintf(statfile, "nr ind jump chaining:  %12llu (two blocks get connected) \n", fbt_nr_ind_jump_chaining);
    
#if defined(FBT_RET_PREDICTION)
    fprintf(statfile, "nr ret predict miss:   %12llu\n", fbt_nr_ret_predict_miss);
    fprintf(statfile, "nr ret pred miss rem:  %12llu\n", fbt_nr_ret_predict_miss_remove);
#endif
	
#if defined(FBT_RET_STACK)
    fprintf(statfile, "nr ret stack sync:     %12llu\n", fbt_nr_ret_stack_synchronize);
    fprintf(statfile, "nr ind jmp bp abs:     %12llu\n", fbt_nr_ind_jump_backpatch_abs);
#endif

#if defined(FBT_RET_FAST)
    fprintf(statfile, "nr ret fast ret:       %12llu\n", fbt_nr_ret_fast);
    fprintf(statfile, "nr ret fast ret miss:  %12llu (%f%%)\n", fbt_nr_ret_fast_miss, (double)fbt_nr_ret_fast_miss/fbt_nr_ret_fast*100);
#endif

#if defined(FBT_RET_CACHE)
    fprintf(statfile, "nr ret cache ret:      %12llu\n", fbt_nr_ret_cache);
    fprintf(statfile, "nr ret cache ret miss: %12llu (%f%%)\n", fbt_nr_ret_cache_miss, (double)fbt_nr_ret_cache_miss/fbt_nr_ret_cache*100);
    fprintf(statfile, "nr ret cache miss def: %12llu\n", fbt_nr_ret_cache_miss_default);
#endif

    fprintf(statfile, "nr calls:              %12llu\n", fbt_nr_call);

#if defined(FBT_INLINE_CALLS)
    fprintf(statfile, "nr inlined calls exec: %12llu (exec per inlined call: %llu %f%% of calls)\n", fbt_nr_inlined_calls, fbt_nr_inlined_calls/fbt_nr_trans_inlined_calls, (double)fbt_nr_inlined_calls/fbt_nr_call*100);
#endif

#if defined(FBT_IND_JUMP_FAST)
    fprintf(statfile, "nr ind jump fast:      %12llu (through local trampoline)\n", fbt_nr_ind_jump_fast);
    fprintf(statfile, "nr ind jump fast miss: %12llu (%f%%)\n", fbt_nr_ind_jump_miss, (double)fbt_nr_ind_jump_miss/fbt_nr_ind_jump_fast*100);
#endif

#if defined(FBT_IND_JUMP_MULTIPLE)
    //fprintf(statfile, "nr ind jumps predicted:                      %10llu\n", fbt_nr_ind_jump_predicted);
    //if(fbt_nr_ind_jump_predicted != 0) fprintf(statfile, "nr ind jumps misspredicted:                  %10llu (%f%%)\n", fbt_nr_ind_jump_misspredicted, (double)fbt_nr_ind_jump_misspredicted/fbt_nr_ind_jump_predicted*100);
    //fprintf(statfile, "nr ind jump fast:                            %10llu\n", fbt_nr_ind_jump_fast);
    //    fprintf(statfile, "nr ind jump fast miss:                       %10llu (%f%%)\n", fbt_nr_ind_jump_fast_miss, (double)fbt_nr_ind_jump_fast_miss/fbt_nr_ind_jump_fast*100);
    fprintf(statfile, "nr ind jump jumptable:                       %10llu\n", fbt_nr_ind_jump_jumptable);
    fprintf(statfile, "nr ind jump jumptable used:                  %10llu\n", fbt_nr_ind_jump_jumptable_used);
    fprintf(statfile, "nr ind jump jumptables created:              %10llu\n", fbt_nr_ind_jump_jumptables_created);
    fprintf(statfile, "nr ind jump jumptable fixup code executed:   %10llu\n", fbt_nr_ind_jump_jumptable_fixup);
    if(fbt_nr_ind_jump_jumptable_used != 0) fprintf(statfile, "nr ind jump jumptable miss:                  %10llu (%f%%)\n", fbt_nr_ind_jump_jumptable_miss, (double)fbt_nr_ind_jump_jumptable_miss/fbt_nr_ind_jump_jumptable_used*100);
#endif

#if defined(FBT_IND_PREDICTION)
    fprintf(statfile, "nr ind jump exe fast:  %12llu\n", fbt_nr_ind_jump_fast); 
    fprintf(statfile, "nr ind jump fast miss: %12llu (%f%%)\n", fbt_nr_ind_jump_fast_miss, (double)fbt_nr_ind_jump_fast_miss/fbt_nr_ind_jump_fast*100); 
    fprintf(statfile, "nr ind jumps predicted:%12llu\n", fbt_nr_ind_jump_pred);
    if(fbt_nr_ind_jump_pred != 0) fprintf(statfile, "nr ind jumps misspred: %12llu (%f%%)\n", fbt_nr_ind_jump_pred_miss, (double)fbt_nr_ind_jump_pred_miss/fbt_nr_ind_jump_pred*100);
#endif

    fprintf(statfile, "nr ind calls exec:     %12llu\n", fbt_nr_ind_calls);
#if defined(FBT_IND_CALL_PREDICTION)
#if defined(FBT_RET_CACHE)
    fprintf(statfile, "nr ind calls exe fast: %12llu %12llu\n", fbt_nr_ind_calls_fast-fbt_nr_ret_cache_miss-fbt_nr_ret_cache_miss_default, fbt_nr_ind_calls_fast);
    fprintf(statfile, "nr ic exe fast miss:   %12llu (%f%%)\n", fbt_nr_ind_calls_fast_miss, (double)fbt_nr_ind_calls_fast_miss/fbt_nr_ind_calls_fast*100);
#else
    fprintf(statfile, "nr ind calls exe fast: %12llu\n", fbt_nr_ind_calls_fast);
    fprintf(statfile, "nr ic exe fast miss:   %12llu (%f%%)\n", fbt_nr_ind_calls_fast_miss, (double)fbt_nr_ind_calls_fast_miss/fbt_nr_ind_calls_fast*100);
#endif
    fprintf(statfile, "nr ind calls exe pred: %12llu\n", fbt_nr_ind_calls_pred); 
    fprintf(statfile, "nr ind calls exe miss: %12llu (%f%%)\n", fbt_nr_ind_calls_miss, (double)fbt_nr_ind_calls_miss/fbt_nr_ind_calls_pred*100);
#endif

}

#endif /* FBT_STATISTIC */
