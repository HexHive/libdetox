/**
 * This handles statistics
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
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
    fprintf(statfile, "Fast tcache lookups:   %10llu\n", fbt_nr_tcache_fast_lookups);
#endif
    fprintf(statfile, "Slow tcache lookups:   %10llu\n", fbt_nr_tcache_slow_lookups);
#if defined(FBT_FIND_FAST)
    fprintf(statfile, "Fast tcache misses:    %10llu (%f%%)\n\n", fbt_nr_tcache_fast_miss, (double)fbt_nr_tcache_fast_miss/fbt_nr_tcache_fast_lookups*100);
#endif
    
    fprintf(statfile, "nr of ccf:             %10d (basic blocks)\n", fbt_nr_ccf);
    fprintf(statfile, "tcache hashmap:        %10d (direct) %d (1) %d (2) %d (-4) %d (-8) %d (8-)\n", fbt_nr_tcache_direct, fbt_nr_tcache_1, fbt_nr_tcache_2, fbt_nr_tcache_4, fbt_nr_tcache_8, fbt_nr_tcache_8p);
    fprintf(statfile, "translated instrs:     %10d\n", fbt_nr_translated_instr);
    fprintf(statfile, "translated jccs:       %10d (conditional jumps)\n", fbt_nr_translated_jcc);
    fprintf(statfile, "translated jmps:       %10d\n", fbt_nr_translated_jmp);
    fprintf(statfile, "translated ind jmps:   %10d\n", fbt_nr_translated_jmp_ind);
    fprintf(statfile, "translated calls:      %10d\n", fbt_nr_translated_call);
    fprintf(statfile, "translated ind calls:  %10d (indirect - no static prediction)\n", fbt_nr_translated_call_ind);
#if defined(FBT_INLINE_CALLS)
    fprintf(statfile, "inlined calls:         %10d (no call/ret overhead for these)\n", fbt_nr_trans_inlined_calls);
#endif
    
    fprintf(statfile, "\nIndirect jumps\n");
    fprintf(statfile, "==============\n");
    fprintf(statfile, "nr ind jump:           %10llu\n", fbt_nr_ind_jump);
    fprintf(statfile, "nr ind jump remove:    %10llu (called by ret that removes bytes)\n", fbt_nr_ind_jump_remove);
    fprintf(statfile, "nr ind jump backpatch: %10llu (trampoline removed)\n", fbt_nr_ind_jump_backpatch);
    fprintf(statfile, "nr ind jump chaining:  %10llu (two blocks get connected) \n", fbt_nr_ind_jump_chaining);
    
#if defined(FBT_RET_PREDICTION)
    fprintf(statfile, "nr ret predict miss:   %10llu\n", fbt_nr_ret_predict_miss);
    fprintf(statfile, "nr ret pred miss rem:  %10llu\n", fbt_nr_ret_predict_miss_remove);
#endif
	
#if defined(FBT_RET_STACK)
    fprintf(statfile, "nr ret stack sync:     %10llu\n", fbt_nr_ret_stack_synchronize);
    fprintf(statfile, "nr ind jmp bp abs:     %10llu\n", fbt_nr_ind_jump_backpatch_abs);
#endif

#if defined(FBT_RET_FAST)
    fprintf(statfile, "nr ret fast ret:       %10llu\n", fbt_nr_ret_fast);
    fprintf(statfile, "nr ret fast ret miss:  %10llu (%f%%)\n", fbt_nr_ret_fast_miss, (double)fbt_nr_ret_fast_miss/fbt_nr_ret_fast*100);
#endif

    fprintf(statfile, "nr calls:              %10llu\n", fbt_nr_call);

#if defined(FBT_INLINE_CALLS)
    fprintf(statfile, "nr inlined calls exec: %10llu (exec per inlined call: %llu %f%% of calls)\n", fbt_nr_inlined_calls, fbt_nr_inlined_calls/fbt_nr_trans_inlined_calls, (double)fbt_nr_inlined_calls/fbt_nr_call*100);
#endif

    fprintf(statfile, "nr ind calls exec:     %10llu\n", fbt_nr_ind_calls);
#if defined(FBT_IND_CALL_PREDICTION)
    fprintf(statfile, "nr ind calls exe miss: %10llu (%f%%)\n", fbt_nr_ind_calls_miss, (double)fbt_nr_ind_calls_miss/fbt_nr_ind_calls*100);
#endif

}
