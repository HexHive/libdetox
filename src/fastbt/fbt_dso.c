/**
 * @file fbt_dso.c
 * Interface to the shared library loader.
 * 
 * This is where all the CFI checks are done except for the optimized
 * checks that are inlined.
 * 
 * Note: this is the first implementation and thus some functions need
 * to be rewritten and redesigned.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * 
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

#define _GNU_SOURCE

#include "fbt_dso.h"

#include <link.h>
#include <elf.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm-generic/fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "fbt_datatypes.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "libfastbt.h"

#if defined(VERIFY_CFTX)

extern struct dso_chain *dso_chain;

#define CFI_WARN_INTRA_MOD_CALL 20
#define CFI_WARN_CALL_TO_UNIMPORTED_SYM 21
#define CFI_WARN_CALL_TO_LOCSYM 22
#define CFI_WARN_SPECIAL_CALL 23
#define CFI_WARN_SPECIAL_JMP 24
#define CFI_WARN_JMPDST_NOT_VERIFIED 25
#define CFI_WARN_DIRJMP_DIFFERENT_METHOD 26
#define CFI_WARN_INDJMP_DIFFERENT_METHOD_START 27
#define CFI_WARN_CALLBACK 28

#define CFI_ERR_NO_SRCDSO 30
#define CFI_ERR_NO_DSTDSO 31
#define CFI_ERR_NO_SYM_INTRACALLDST 32
#define CFI_ERR_NO_SYM_INTERCALLDST 33
#define CFI_ERR_CALLDST_NEXEC 34
#define CFI_ERR_NO_SYM_AT_JMP_SRCDST 35
#define CFI_ERR_JMPDST_NOT_EXEC 36
#define CFI_ERR_INDJMP_DIFFERENT_METHOD 37
#define CFI_ERR_JMPDST_DIFFERENT_DSO 38

#define CFI_ERR_CALL_NOT_VERIFIABLE 40
#define CFI_WARN_CALL_NOT_VERIFIABLE 41
#define CFI_WARN_CALLBACK_JMP 42
#define CFI_WARN_JMPDST_STRIPPED_BUT_SURROUNDED 43
#define CFI_ERR_JMPDST_NOT_VERIFIED 44

#define CFI_VIOLATION 1
#define CFI_WARNING 0

/* Do not check control flow transfers initiated by the loader */
// TODO
//#define SKIP_LOADER

static void print_cftx(int type, char *msg, long errcode, unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol) {

  if(type == CFI_VIOLATION) llprinterr("*** CFI VIOLATION ***; %s; %d; %x; ", msg, errcode, src);
  else llprinterr("CFI WARNING; %s; %d; %x; ", msg, errcode, src);
  
  if(srcdso) llprinterr("%s (%x); ", srcdso->name, ((unsigned long)src-(unsigned long)srcdso->baseaddr));
  else llprinterr("-; ");
  if(src_symbol) llprinterr("%s; ", src_symbol->name);
  else llprinterr("-; ");
  llprinterr(">>> calls/jmps >>>; %x; ", dst);
  if(dstdso) llprinterr("%s (%x); ", dstdso->name, ((unsigned long)dst-(unsigned long)dstdso->baseaddr));
  else llprinterr("-; ");
  if(dst_symbol) llprinterr("%s;\n", dst_symbol->name);
  else llprinterr("-;\n");

}

/**
 * Helper function which prints an error or fails,
 * depending on the settings
 * @param msg Message to print
 */
static void error(char *msg, long errcode, unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol) {
	
#if defined (PRINT_CFTX)
  llprintf("\n[CFI VIOLATION] %s:\n", msg);
#else

  print_cftx(CFI_VIOLATION, msg, errcode, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
  
#if defined (LOG_CFTX_ONLY)
  return;
#endif

#if defined(SECURITY_METRICS_AND_STATS)
  fbt_print_security_metrics_and_stats();
#endif

#ifdef ENABLE_UI_MESSAGES
  if((errcode >= 20 && errcode <= 23) || (errcode >= 29 && errcode <= 34)) {
	  notify_ui(UI_CALL_VIOLATION, "Invalid target for indirect call!");
  } else {
	  notify_ui(UI_JMP_VIOLATION, "Invalid target for indirect jmp!");
  }
#endif
	  
  fbt_exit_suicide(errcode);
  
#endif /* else PRINT_CFTX */

}

/**
 * Helper function which prints a warning message.
 * @param msg Message to print
 */
static void warn(char *msg, long errcode, unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol) {
	
#if defined (PRINT_CFTX)
  llprintf("\n[CFI WARNING] %s:\n", msg);
#elif defined (LOG_CFTX_ONLY)

#if defined (LOG_CFTX_WARNINGS)
  print_cftx(CFI_WARNING, msg, errcode, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
#endif

#endif

  /* DO NOTHING, it's just a warning. */
  
}

/**
 * Implementation of binary search which returns the smallest element greater
 * than the one searched for if no item is found.
 * @param array The array to search in
 * @param low Lower bound for the index
 * @param high Upper bound for the index
 * @param addr The address to search for
 * @return index in the array
 */
static long binary_search(struct sh_symbol *array, long low, long high,
                          long addr) {
  if (low >= high) {
    /* Not found, return index of smallest element > addr */
    return high;   
  } else {
    long mid = (low + high) / 2;
    if (addr == (long)array[mid].start) {
      /* Element found, return index */
      return mid;
    } else if (addr < (long)array[mid].start) {
      /* Too big, continue with smaller elements */
      return binary_search(array, low, mid-1, addr);
    } else {
      /* Too small, continue with bigger elements */
      return binary_search(array, mid+1, high, addr);
    }
  }
}
                  
/* TODO: do version sensitive symbol comparision */
int fbt_strncmp_symbols(const char *s1, const char *s2, int n) {
  /* Note: while loop might be better */
  for (; *s1 == *s2 || *s1 == 0 || *s2 == 0; ++s1, ++s2, --n)
    if (*s1 == 0 || n==0 || *s2 == 0)
      return 0;
  return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

/* TODO: this will just compare the two strings till an at is encountered,
 * we need it for one of the special cases, see fbt_special_cft_case. */
int fbt_strncmp_symbols_at(const char *s1, const char *s2, int n) {
  /* Note: while loop might be better */
  for (; *s1 == *s2 || *s1 == 0 || *s2 == 0 || *s1 == '@' || *s2 == '@'; ++s1, ++s2, --n)
    if (*s1 == 0 || n==0 || *s2 == 0 || *s1 == '@' || *s2 == '@')
      return 0;
  return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

/* Verify that the src is in the srcdso plt section
 * and that the src_symbol name matches the dst_symbol name */
int is_valid_plt_jmp(unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
	unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol) {
	if (srcdso->plt_size && PTR_IN_REGION(src, srcdso->plt, srcdso->plt_size-1)) {
		/* TODO: there is a special case in the dealII spec CPU2006 benchmark and probably
		 * also in other applications. 
		 *  - src_symbol->name = _ZNSsD1Ev@@GLIBCXX_3.4
		 *  - dst_symbol->name = _ZNSsD2Ev */
		return 1;
	}
	return 0;
}

/* libc seems to call other DSOs plt entries directly! */
int is_valid_plt_call(unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
	unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol) {
	if (dstdso->plt_size && PTR_IN_REGION(dst, dstdso->plt, dstdso->plt_size-1)) {
		return 1;
	}
	return 0;
}

#if defined(ENABLE_TRANSLATION_TIME_SYMBOL_LOOKUP)
int fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type, struct sh_symbol *src_symbol) {
#else
int fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type) {
#endif

  /* return value, 0 if transfer is not allowed, 1 if allowed */
  int return_value = 0;

#if defined(LOG_VALID_JMP_TARGETS)
 if(type == CFTX_JMP_IND) {
#if defined(LOG_JMP_INSTR)
	 if(src == LOG_JMP_INSTR) {
#endif
		fbt_log_valid_jmp_targets(src);
#if defined(LOG_JMP_INSTR)
	 }
#endif
 }
#endif /* LOG_VALID_JMP_TARGETS */

#if defined(LOG_VALID_CALL_TARGETS)
 if(type == CFTX_CALL_IND) {
#if defined(LOG_CALL_INSTR)
	 if(src == LOG_CALL_INSTR) {
#endif
		fbt_log_valid_call_targets(src);
#if defined(LOG_CALL_INSTR)
	 }
#endif
 }
#endif /* LOG_VALID_CALL_TARGETS */
			
  /* Note: CFTX_RET disabled in action_ret */
  if (type == CFTX_JMP_IND || type == CFTX_CALL_IND) {
    /* dst is only a ptr to a ptr, so we must dereference once */
    dst = (unsigned char*)(*((long*)dst));
  }

#if defined(DUMP_CFTX)
  struct control_flow_transfer_dump dump = {
    type,
    src,
    dst
  };
  long res;
  fbt_write(tld->dump_cftx_file, &dump, sizeof(dump), res, "Could not write to cftx dump.");
#endif /* DUMP_CFTX */

  struct dso_chain *cur = tld->dso_objects;
  struct dso_chain *dstdso = 0;
  struct sh_symbol *dst_symbol = 0;
  struct dso_chain *srcdso = 0;
  struct sh_symbol *src_nsymbol = 0;

#if defined(ENABLE_TRANSLATION_TIME_SYMBOL_LOOKUP)
  if(src_symbol) srcdso = src_symbol->dso;
#else
  struct sh_symbol *src_symbol = 0;
#endif

#if defined(CFTX_ENABLE_SYMBOL_CACHE)
  /* check our (small) cache if the src and dst are in the recently used objects */
  long i;
  for (i = 0; i < SYMBOL_CACHE_SIZE; ++i) {

#if defined(ENABLE_TRANSLATION_TIME_SYMBOL_LOOKUP)
    if (srcdso == 0 && tld->symb_cache[i].addr == src) {
      src_symbol = tld->symb_cache[i].symbol;
      srcdso = tld->symb_cache[i].dso;
    }
#else
    if (tld->symb_cache[i].addr == src) {
      src_symbol = tld->symb_cache[i].symbol;
      srcdso = tld->symb_cache[i].dso;
    }
#endif

    if (tld->symb_cache[i].addr == dst) {
      dst_symbol = tld->symb_cache[i].symbol;
      dstdso = tld->symb_cache[i].dso;
    }
  }

#endif /* CFTX_ENABLE_SYMBOL_CACHE */

/* Do a fastlookup. */
#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)

	/* Do a fast srcdso lookup. */
	if(!srcdso) {
	 while (cur != 0) {
		/* Check if src belongs to DSO */
		if (((ulong_t)src >= (ulong_t)cur->baseaddr) && ((ulong_t)src <= (ulong_t)cur->endaddr)) {
			srcdso = cur;
			break;
		}
        cur = cur->next;
	 }
	}

	/* Map the dst address and see if it is a valid target. */
	ulong_t offset = C_CFT_MAPPING_FUNCTION((ulong_t)dst);
	unsigned long* entry;
	unsigned long* table;
    int count = 0;

	if(type == CFTX_CALL_IND) table = (unsigned long*)srcdso->cft_valid_call_targets;
	else table = (unsigned long*)srcdso->cft_valid_jmp_targets;
	/* TODO: disable fastlookup for jmp targets? Correctness might be an issue if
	 * inlined ijmp local symbol lookup is not enabled. Inter-symbol jumps might be
	 * allowed although not compliant with the CFI policy. */
	
	entry = (unsigned long*)((unsigned long)table + offset);
	
	while (*entry != 0) {
		if ((unsigned long)dst == *entry) {
		  #if defined(PRINT_CFTX_FASTLOOKUP_STATS)
		  	long retval;
		  	FBT_UNPROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
			srcdso->nr_fastlookups++;
			#if defined(PRINT_CFTX_CHECK_TRANSFER_STATS)
			srcdso->nr_check_transfers_total++;
			#endif
			FBT_PROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
		  #endif /* defined(PRINT_CFTX_FASTLOOKUP_STATS) */
		  return 1;
		}

		offset = (offset + sizeof(unsigned long)) & (CFT_MAPPINGTABLE_SIZE-1);
		entry = (unsigned long*)((unsigned long)table + offset);
		if (count >= CFT_MAPPINGTABLE_MAXENTRIES/4) break;
		count++;
    }
    
	/* Reset cur for later. */
	cur = tld->dso_objects;

#endif /* defined(VERIFY_CFTX_ENABLE_FASTLOOKUP) */

//#ifdef SKIP_LOADER
  ///* Skip control flow transfers with src at secuLoader (always first entry) */
  //if (((ulong_t)src >= (ulong_t)cur->baseaddr)
      //&& ((ulong_t)src <= (ulong_t)cur->endaddr))
    //return;
//#endif

  /* TODO: maybe this can be optimized as it appears that the range lookup is one of the main
   * bottlenecks. Resolve src/dst symbols in parallel? */
  /* In case we still have not found srcdso/src_symbol, look it up. */
  if(!src_symbol) src_symbol = fbt_find_symbol(tld, src);
  if(src_symbol) srcdso = src_symbol->dso;
  if(!srcdso) srcdso = fbt_find_dso(tld, src);

  /* TODO: maybe looking up src in parallel with dst is nevertheless better, see revision 285. */
  /* Search all DSOs */
  while (cur != 0) {

	/* TODO: sometimes DSOs define multiple symbols for the same address (like libc malloc/__libc_malloc). Furthermore
	 * some are WEAK and some are GLOBAl. If we find the WEAK symbol with a different name as the one in the relocation
	 * entry of the importing DSO we will have a mismatch although a GLOBAL symbol in the dstdso exists. This has to be fixed! */
	/* Workaround: as this happens rarely we will just do another linear search within fbt_is_special_cft_case but clearly
	 * this has to be fixed differently. */
    /* Check if dst belongs to DSO */
    if (((ulong_t)dst >= (ulong_t)cur->baseaddr)
            && ((ulong_t)dst <= (ulong_t)cur->endaddr)) {
      dstdso = cur;

      /* Get address of page and compute offset of this page in the array */
      long page_addr = ((long)dst & PMASK);
      long poffset = (page_addr - dstdso->pages[0].addr) >> PSHIFT;

      /* If address of first symbol in page to big go to previous page */
      while (poffset >= 1 && (!(&dstdso->pages[poffset].syms[0])
                              || dst < dstdso->pages[poffset].syms[0]->start))
        poffset--;
      
      struct sh_symbol *symbols = *dstdso->pages[poffset].syms;
      if (symbols) {
        /* Perform a binary search on the symbols for symbol with start==dst */
        long nr_symbols = dstdso->pages[poffset].nr_symbols;
        long index = binary_search(symbols, 0, nr_symbols, (long)dst);
        if (dst == symbols[index].start) {
          /* Symbol found */
          dst_symbol = &symbols[index];
        } else {
		  /* TODO: why? this should only be possible for jmps, as calls should always go to symbol address */
          /* Not found, go left and search first symbol where dst is inside */
          for (;index>=0; --index) {
            struct sh_symbol *cur_symbol = &(symbols[index]);
            if (PTR_IN_REGION(dst, cur_symbol->start, cur_symbol->size)) {
              dst_symbol = cur_symbol;
              break;
            } 
          }
        }
      }
      break;
    }
    cur = cur->next;
  }  /* iterate all dsos */

#if defined(CFTX_ENABLE_SYMBOL_CACHE)
  /* update the cache, store our two current objects in the front, older
     objects get kicked out of the cache on an LRU principle. */
  long k, l;
  for (k = 0, l=2; k < SYMBOL_CACHE_SIZE-3; ++k, ++l) {
    if (tld->symb_cache[k].addr == src) {
      k++;
      continue;
    }
    if (tld->symb_cache[k].addr == dst) {
      k++;
      continue;
    }
    if (l == k) break;
    tld->symb_cache[l].addr = tld->symb_cache[k].addr;
    tld->symb_cache[l].symbol = tld->symb_cache[k].symbol;
    tld->symb_cache[l].dso = tld->symb_cache[k].dso;
  }
  tld->symb_cache[0].addr = src;
  tld->symb_cache[0].symbol = src_symbol;
  tld->symb_cache[0].dso = srcdso;
  tld->symb_cache[1].addr = dst;
  tld->symb_cache[1].symbol = dst_symbol;
  tld->symb_cache[1].dso = dstdso;
#endif /* CFTX_ENABLE_SYMBOL_CACHE */

#if defined(PRINT_CFTX_CHECK_TRANSFER_STATS)
  if(srcdso) {
	  long retval;
	  FBT_UNPROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
	  srcdso->nr_check_transfers_total++;
	  FBT_PROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
  }
#endif /* defined(PRINT_CFTX_CHECK_TRANSFER_STATS) */

#if defined(PRINT_CFTX_TYPE_STATS)
nr_total_cftx_checks++;
#endif

  if (srcdso == 0) {
    /* Src address does not belong to a loaded dso */
    error("No source dso found", CFI_ERR_NO_SRCDSO, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
    return_value = 0;
  } else if (dstdso == 0) {
    /* Dst address does not belong to a loaded dso */
    error("No destination dso found", CFI_ERR_NO_DSTDSO, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
    return_value = 0;
  } else if (type == CFTX_CALL_IND) { /* ---------------------------------------- CALL ----------------------------------------------- */
    if (srcdso == dstdso) { /* case: call where srcdso == dstdso ***********************************************************************/

      if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) { /* no symbol at call destination */
     
        if (!dstdso->stripped) { /* NOT STRIPPED */
          /* check if it is a callback */
		  if(fbt_find_callback(dstdso, dst)) {
			warn("Call is a callback", CFI_WARN_CALLBACK, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			return_value = 1;
			#if defined(PRINT_CFTX_TYPE_STATS)
			nr_icalls_callback++;
			#endif
		  } else {
			error("No symbol defined at call destination", CFI_ERR_NO_SYM_INTRACALLDST, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			return_value = 0;
		  }
        } else { /* STRIPPED */

			/* check if it is a callback */
			if(fbt_find_callback(dstdso, dst)) {
				warn("Call is a callback", CFI_WARN_CALLBACK, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_callback++;
				#endif
			} else if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
              && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
              && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
				/* call goes to non-executable section */
				/* TODO: soffice has more executable sections. See __libc_freeres_fn/___libc_thread_fre and more... 
				* To be fixed! We need to check all executable segments and not only text, init, fini! Same for ijmp! */
				error("Call destination not in executable section", CFI_ERR_CALLDST_NEXEC, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;
            } else { /* call not verifiable */
#if defined(ENABLE_STRICT_CFI_POLICY)
				error("Could not verify inner module call dest, object stripped", CFI_ERR_CALL_NOT_VERIFIABLE, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;
#else
				warn("Could not verify inner module call dest, object stripped", CFI_WARN_CALL_NOT_VERIFIABLE, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_not_verified++;
				#endif
#endif
			}
            
        } /* end STRIPPED case */
        
      } else { /* call goes to symbol */
		  return_value = 1;
	  }
      #if defined(PRINT_CFTX_TYPE_STATS)
	  nr_icalls_intra_dso++;
	  #endif

    } else { /* case: call where srcdso != dstdso  **************************************************************************************/

		if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) { /* no symbol at call destination */

			if(fbt_find_callback(dstdso, dst)) {
				warn("Call is a callback", CFI_WARN_CALLBACK, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_callback++;
				#endif
			} else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				warn("Special case", CFI_WARN_SPECIAL_CALL, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_special_case++;
				#endif
			} else {
				error("No symbol defined at call destination", CFI_ERR_NO_SYM_INTERCALLDST, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;
			}

       } else { /* call goes to symbol */
       
        /* Check if destination symbol is imported in source DSO */
        /* TODO (hartobia) we should optimize this */
		/* Use sym_lookup/elf_hash_lookup instead of linear search! */
		/* TODO: we need to make sure that we check all defined symbols in the dstdso, there might be
		 * multiple definitions for the same address. E.g. malloc/__libc_malloc. */
        int i;
        unsigned char found = 0;
        for (i=0; i<srcdso->nr_isymbols; ++i) {
          if (srcdso->isymbols[i].start != 0)
            break;
          if (!fbt_strncmp_symbols(srcdso->isymbols[i].name, dst_symbol->name, 512)) {
            found = 1;
            break;
          }
        }
        
        if (!found) { /* not imported in srcdso */
			
#if defined(ENABLE_STRICT_CFI_POLICY)

			if(fbt_find_callback(dstdso, dst)) {
				warn("Call is a callback", CFI_WARN_CALLBACK, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_callback++;
				#endif
			} else if (fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				warn("Special case", CFI_WARN_SPECIAL_CALL, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;					
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_special_case++;
				#endif				
			} else {
				error("Call to unimported symbol", CFI_WARN_CALL_TO_UNIMPORTED_SYM, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;				
			}

#else
			warn("Call to unimported symbol", CFI_WARN_CALL_TO_UNIMPORTED_SYM, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			return_value = 1;
			#if defined(PRINT_CFTX_TYPE_STATS)
			nr_icalls_unimported_symbols++;
			#endif
#endif

        } else if (dst_symbol->binding != SB_GLOBAL) {
        /* check if symbol is exported in destination DSO */
        /* TODO: should we be more strict here? Deny? */
        
        /* symbol is local symbol */
#if defined(ENABLE_STRICT_CFI_POLICY)

			if(fbt_find_callback(dstdso, dst)) {
				warn("Call is a callback", CFI_WARN_CALLBACK, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_callback++;
				#endif
			} else if (fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				warn("Special case", CFI_WARN_SPECIAL_CALL, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;					
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_icalls_special_case++;
				#endif				
			} else {
				error("Call to local symbol in different module", CFI_WARN_CALL_TO_LOCSYM, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;				
			}

#else
          warn("Call to local symbol in different module", CFI_WARN_CALL_TO_LOCSYM, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
          return_value = 1;
          #if defined(PRINT_CFTX_TYPE_STATS)
          nr_icalls_local_symbol++;
          #endif				
#endif

        } else { /* symbol imported in srcdso and exported in dstdso */
		  return_value = 1;
		}
      } /* end - call goes to symbol */

      #if defined(PRINT_CFTX_TYPE_STATS)
	  nr_icalls_inter_dso++;
	  #endif
    } /* end - srcdso != dstdso */

  } else if (type == CFTX_JMP_IND) {  /* ---------------------------------------- JMP ----------------------------------------------- */
    if (srcdso == dstdso) { /* case: call where srcdso == dstdso **********************************************************************/

      if (dst_symbol == 0 || src_symbol == 0) {

        if (!srcdso->stripped) { /* NOT STRIPPED */

            if(fbt_find_callback(dstdso, dst)) {
				warn("Jmp to callback within same module", CFI_WARN_CALLBACK_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_callback++;
				#endif
				return_value = 1;				
			} else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				warn("Special case", CFI_WARN_SPECIAL_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_special_case++;
				#endif
			} else {
				error("No symbol defined at jmp source/destination", CFI_ERR_NO_SYM_AT_JMP_SRCDST, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;
			}
			
        } else { /* STRIPPED */



/*****************************************************************************************************************************************/
		  int is_surrounded = fbt_addr_is_surrounded_by_symbols(srcdso, src, dst);
		  src_nsymbol = fbt_find_nsymbol(tld, src);

		  if(is_surrounded > 0) {
			  
			  if(is_surrounded == 2) {
			    warn("Could not verify jmp dest, object stripped but surrounded by symbols.", CFI_WARN_JMPDST_STRIPPED_BUT_SURROUNDED, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			    return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_within_stripped_symbol++;
				#endif
			  } else {
				error("Indirect jmp to different method (stripped)", CFI_ERR_INDJMP_DIFFERENT_METHOD, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;				  
			  }

		  } else if (src_symbol && src_nsymbol) {
			  
			/* check if dest is in region defined by surrounding symbols */
			if (dst < src_symbol->start || dst > src_nsymbol->start) {
				error("Indirect jmp to different method (stripped)", CFI_ERR_INDJMP_DIFFERENT_METHOD, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 0;
			} else {
			    warn("Could not verify jmp dest, object stripped but surrounded by symbols.", CFI_WARN_JMPDST_STRIPPED_BUT_SURROUNDED, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_within_stripped_symbol++;
				#endif
			}

          } else if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
                     && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
                     && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
            /* no surrounding symbols - check if at least dst is in an executable section */
            error("Jmp destination not in executable section", CFI_ERR_JMPDST_NOT_EXEC, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
            return_value = 0;

          } else { /* no surrounding symbols found */

			  if(fbt_find_callback(dstdso, dst)) {
				warn("Jmp to callback", CFI_WARN_CALLBACK_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_callback++;
				#endif		  
			  } else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
			    warn("Special case", CFI_WARN_SPECIAL_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
				return_value = 1;
				#if defined(PRINT_CFTX_TYPE_STATS)
				nr_ijmps_special_case++;
				#endif
			  } else {
				  /* Emit a warning */
				  /* Could be error for stripped libraries as they at least have some surrounding symbols catched
				   * by our check before. But in case of stripped executables (e.g. netstat) no symbol is defined!
				   * We therefore have to allow it here! */
				  if(srcdso->exec) {
					warn("Could not verify jmp dest, object stripped (src is executable)", CFI_WARN_JMPDST_NOT_VERIFIED, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
					return_value = 1;
					#if defined(PRINT_CFTX_TYPE_STATS)
					nr_ijmps_not_verified++;
					#endif
				  } else {
#if defined(ENABLE_STRICT_CFI_POLICY)
					error("Could not verify jmp dest, object stripped (src is library)", CFI_ERR_JMPDST_NOT_VERIFIED, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
					return_value = 0;
#else
					warn("Could not verify jmp dest, object stripped (src is library)", CFI_WARN_JMPDST_NOT_VERIFIED, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
					return_value = 1;
					#if defined(PRINT_CFTX_TYPE_STATS)
					nr_ijmps_not_verified++;
					#endif
#endif
				  }
			  }
			  
		  }
/*****************************************************************************************************************************************/

        } /* end - STRIPPED */
        
      /* end - src_symbol == 0 || dst_symbol == 0 */
      
      } else if (src_symbol != dst_symbol) { /* src_symbol != dst_symbol */
		  
        /* We jump to another method */
        /* Indirect jmp, fail */
        /* NOTE: terminating execution in this case seems too strict */
        /* At least one of the specCPU2006 benchmarks (povray) seems to use
         * an indirect jmp table to different functions/methods within the
         * same DSO. */
         
        /* Suggestion: just alllow if dst_symbol->start == dst (of indirect jmp) */
        /* TODO: this was changed as it failed when debug symbols were available, we need to rethink this case or add a strict exception */

		/* TODO: more spec benchmarks use jmp tables to different methods within the same DSO, in future we should find a way
		 * to identify such targets. For now we will just allow it if the jmp targets a symbol start. */

           if(dst_symbol->start == dst) {
			   warn("Indirect jmp to different method start", CFI_WARN_INDJMP_DIFFERENT_METHOD_START, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			   return_value = 1;
			   #if defined(PRINT_CFTX_TYPE_STATS)
			   nr_ijmps_inter_symbol++;
			   #endif
		   } else {
			 
			 if(fbt_find_callback(dstdso, dst)) {
			   warn("Jmp to callback within same module", CFI_WARN_CALLBACK_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			   return_value = 1;
			   #if defined(PRINT_CFTX_TYPE_STATS)
			   nr_ijmps_callback++;
			   #endif				 
			 } else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
			   warn("Special case", CFI_WARN_SPECIAL_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			   return_value = 1;
			   #if defined(PRINT_CFTX_TYPE_STATS)
			   nr_ijmps_special_case++;
			   #endif
			 } else {
			   error("Indirect jmp into different method", CFI_ERR_INDJMP_DIFFERENT_METHOD, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			   return_value = 0;
		     }
		     
		   }

      } else { /* src_symbol == dst_symbol */
		  return_value = 1;
		  #if defined(PRINT_CFTX_TYPE_STATS)
		  nr_ijmps_intra_symbol++;
	      #endif
	  }
      #if defined(PRINT_CFTX_TYPE_STATS)
	  nr_ijmps_intra_dso++;
	  #endif
	
	/* end - srcdso == dstdso*/
	
    } else { /* case: call where srcdso != dstdso  ************************************************************************************/

		/* TODO: should we also allow inter-DSO callback jumps? */
		if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
			warn("Special case", CFI_WARN_SPECIAL_JMP, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			return_value = 1;
			#if defined(PRINT_CFTX_TYPE_STATS)
			nr_ijmps_special_case++;
			#endif
		} else {
			error("Jmp destination in different dso", CFI_ERR_JMPDST_DIFFERENT_DSO, src, srcdso, src_symbol, dst, dstdso, dst_symbol);
			return_value = 0;
		}
	  #if defined(PRINT_CFTX_TYPE_STATS)
	  nr_ijmps_inter_dso++;
	  #endif
    } /* end - srcdso != dstdso */
    
  }

/* TODO: need to review this.
 * See /usr/lib/i386-linux-gnu/libstdc++.so.6. An address to a plt function is put on the stack
 * before calling this libstdc++ function. This will result in a callback to the plt of the application
 * directly.
 * Therefore our inlining of PLT is bypassed by libc this way. This will result in an usual indirect
 * jmp from the PLT to the intended function in some DSO which then will cause a CFI violation.
 * 	 
 * 00083a50 <_ZNSolsEPFRSoS_E>:
   83a50:       83 ec 1c                sub    $0x1c,%esp
   83a53:       8b 44 24 20             mov    0x20(%esp),%eax
   83a57:       89 04 24                mov    %eax,(%esp)
   83a5a:       ff 54 24 24             call   *0x24(%esp)
   83a5e:       83 c4 1c                add    $0x1c,%esp
   83a61:       c3                      ret    
   83a62:       90                      nop
 * */

#if defined(PRINT_CFTX_CHECK_TRANSFER_STATS)
  if(srcdso) {
	  long retval;
	  FBT_UNPROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
	  srcdso->nr_check_transfers_slowpath++;
	  FBT_PROT_DATA(srcdso, sizeof(struct dso_chain), retval, "fbt_check_transfer: unprotect dso failed.");
  }
#endif /* defined(PRINT_CFTX_CHECK_TRANSFER_STATS) */

/* Fill up table for fastlookup. */
#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)
  entry = (unsigned long*)((unsigned long)table + offset);
  count = 0;
  
  while (*entry != 0 && *entry != (unsigned long)dst) {
    offset = (offset + sizeof(unsigned long)) & (CFT_MAPPINGTABLE_SIZE - 1);
    entry = (unsigned long*)((unsigned long)table + offset);
    count++;
    if (count >= CFT_MAPPINGTABLE_MAXENTRIES/4) { /* was CFT_MAPPINGTABLE_MAXENTRIES/10, is 10 too much? */
	   llprintf("ERROR: CFTX fastlookup table out of space (fbt_dso.c).\n");
	   fbt_exit_suicide(41);
    }
  }

  if(*entry != (unsigned long)dst) {
	//FBT_UNPROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: unprotect dso failed.");
	*entry = (unsigned long)dst;
	//FBT_PROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: protect dso failed.");
  }
#endif /* defined(VERIFY_CFTX_ENABLE_FASTLOOKUP) */

#if defined(PRINT_CFTX)
  /* Print source of cft */
  if (srcdso) {
    llprintf("%s (%p)", srcdso->name,
             (char *)src-((srcdso->exec) ? 0 : srcdso->baseaddr));
    if (src_symbol) {
      llprintf(": <%s+0x%x>", src_symbol->name, src-src_symbol->start);
    }
  } else {
    llprintf("%p", src);
  }    

  /* Type of the cft */
  switch(type) {
  case CFTX_CALL:
    llprintf(" c-> ");
    break;
  case CFTX_CALL_IND:
    llprintf(" ci-> ");
    break;
  case CFTX_JMP:
    llprintf(" j-> ");
    break;
  case CFTX_JMP_IND:
    llprintf(" ji-> ");
    break;
  case CFTX_RET:
    llprintf(" r-> ");
    break;
  }

  /* Print destination of cft */
  if (dstdso) {
    llprintf("%s (%p)", dstdso->name,
             (char *)dst-((dstdso->exec) ? 0 : dstdso->baseaddr));
    if (dst_symbol) {
      llprintf(": <%s", dst_symbol->name);
      
      unsigned long offset = dst-dst_symbol->start;
      if (offset != 0) {
        llprintf("+0x%x>", offset);
      } else {
        llprintf(">");
      }  
    } 
  } else {
    llprintf("%p", dst);
  }
  llprintf("\n");
  
#endif /* PRINT_CFTX */

 /* Finally return value. */
 return return_value;
}

/* Important notes on special case handling:
 * 
 * This function determines if a certain CFT belongs to a limited set of special cases.
 * This is the location where the whitelist checks are done. Note: a productive implementation
 * needs to be much more careful here. As the security evaluation uses the exact available location
 * at run-time we are fine since all these special cases are considered to be legitimate transfers. 
 * Still, an improved version could further reduce attack surface.
 * 
 * Note on whitelist: the whitelist is hardcoded in this function. This is very bad and should be rewritten.
 * This list should be exported to a separate file and ideally generated automatically. Further, some very
 * specific application library cases need to be removed from the whitelist. This is feasible but requires
 * additional engineering efforts. The whitelist should only contain any run-time specific transfers that are
 * difficult to detect statically/dynamically.
 * 
 * We currently have exceptions/whitelist handling for some run-time and very limited application level libraries.
 * The application level exceptions need to be handled differently in a productive version.
 * 
 */
int fbt_is_special_cft_case(unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol, enum cftx_type type) {

	if(srcdso == 0 || dstdso == 0) return 0;

	if (type == CFTX_CALL_IND) {
		
		if(srcdso != dstdso) {
			
			if ((dst_symbol == 0 || (dst-dst_symbol->start) != 0) && src_symbol != 0) {

				if(!fbt_strncmp(srcdso->name, "secuLoader", 10)) {
					
					if(dst_symbol != 0 && !fbt_strncmp(src_symbol->name, "so_init", 7) && !fbt_strncmp(dst_symbol->name, "__libc_init_first", 17)) {
						return 1;
					}
					
					if(dstdso != 0) {
						if(dst == (unsigned char*)dstdso->init && !fbt_strncmp(src_symbol->name, "so_init", 7)) return 1;
						if(dst == (unsigned char*)dstdso->fini && !fbt_strncmp(src_symbol->name, "unload_elf", 10)) return 1;
					}	
					
				} else if(!fbt_strncmp(srcdso->name, "libgobject-2.0.so.0", 19)
					&& !fbt_strncmp(src_symbol->name, "g_type_module_use", 17)
					&& dstdso != 0 && !fbt_strncmp(dstdso->name, "libgtk-x11-2.0.so.0", 19)) {
					return 1;
				}
				
			} else if(dst_symbol != 0) {
				
				if(!fbt_strncmp(srcdso->name, "secuLoader", 10)) {
					return 1;
					/* Our loader needs to be able to call any symbol! See our exceptions testcase. */
				
				} else if(!fbt_strncmp(srcdso->name, "libc.so.6", 9)) {
					
					if(!fbt_strncmp(dstdso->name, "secuLoader", 10)) {
						
						if(!fbt_strncmp(dst_symbol->name, "lookup_symbol_x", 15) || !fbt_strncmp(dst_symbol->name, "catch_error", 11)
						|| !fbt_strncmp(dst_symbol->name, "dl_open", 7) || !fbt_strncmp(dst_symbol->name, "tls_get_addr", 12)) return 1;
					
						if(src_symbol != 0) {
							if(!fbt_strncmp(src_symbol->name, "_dl_addr", 8)) {
								if(!fbt_strncmp(dst_symbol->name, "lock_recursive", 14) || !fbt_strncmp(dst_symbol->name, "unlock_recursive", 16)) return 1;
							}
						}
					}
					
					if(is_valid_plt_call(src, srcdso, src_symbol, dst, dstdso, dst_symbol)) return 1;
					
				} else if(!fbt_strncmp(srcdso->name, "libdl.so.2", 10)) {
					
					if(!fbt_strncmp(dstdso->name, "secuLoader", 10)) {
					
						if(!fbt_strncmp(dst_symbol->name, "catch_error", 11) || !fbt_strncmp(dst_symbol->name, "dl_open", 7)
						|| !fbt_strncmp(dst_symbol->name, "dl_close", 8)) return 1;
						else if(src_symbol != 0 && !fbt_strncmp(src_symbol->name, "dlsym", 5) && !fbt_strncmp(dstdso->name, "secuLoader", 10)
						&& (!fbt_strncmp(dst_symbol->name, "lock_recursive", 14) || !fbt_strncmp(dst_symbol->name, "unlock_recursive", 16))) return 1;
						else if(src_symbol != 0 && !fbt_strncmp(src_symbol->name, "dladdr", 6) && !fbt_strncmp(dst_symbol->name, "dl_addr", 7)) return 1;
						
					} else if(!fbt_strncmp(dstdso->name, "libpthread.so.0", 15)) {
						if(!fbt_strncmp(dst_symbol->name, "__pthread_setspecific_internal", 30)) return 1;
						
					} else if(src_symbol != 0 && !fbt_strncmp(dstdso->name, "libc.so.6", 9) && !fbt_strncmp(src_symbol->name, "dlerror", 7)) return 1; 
				
				} else if(!fbt_strncmp(srcdso->name, "libpthread.so.0", 16)) {
						if(!fbt_strncmp(dstdso->name, "secuLoader", 10)) {
							if(!fbt_strncmp(src_symbol->name, "__pthread_initialize_minimal", 28)) {
								if(!fbt_strncmp(dst_symbol->name, "get_tls_static_info", 19)) return 1;
								else if(!fbt_strncmp(dst_symbol->name, "error_catch_tsd", 15)) return 1;
							} else if (!fbt_strncmp(src_symbol->name, "pthread_create@@GLIBC_2.1", 25)) {
								if(!fbt_strncmp(dst_symbol->name, "allocate_tls", 12)) return 1;
							}
						}
						
						if(!fbt_strncmp(src_symbol->name, "start_thread", 12) && dstdso->exec) return 1;
				
				} else if(!fbt_strncmp(srcdso->name, "libuno_", 6)) {
					if(src_symbol != 0 && !fbt_strncmp(dstdso->name, "libuno_", 6)) return 1;
					/* special case for libuno (soffice), allow calls to all symbols between libs. */
				} 
				
				if(!fbt_strncmp(dstdso->name, "secuLoader", 10) && !fbt_strncmp(dst_symbol->name, "tls_get_addr", 12)) {
					return 1;
					/* TODO: we allow tls_get_addr to be called from everywhere,
				     * ideally it can just be called by modules that have TLS i.e. some thread variables. */
				}

				/* TODO: this is a simple but not so strict way to allow the c++ runtime to do
				 * exception handling. This requires improvement!
				 * Note: as the security evaluation does consider these transfers as legitimate
				 * we are on the safe side here as we do consider these targets as potential gadget locations.
				 * Still, for a productive version these needs additional engineering efforts. */
				if(!fbt_strncmp(srcdso->name, "libstdc++.so.6", 14) && dst_symbol != 0) {
					if(!fbt_strncmp("_Z", dst_symbol->name, 1)) return 1;
				} else if(!fbt_strncmp(srcdso->name, "libgcc_s.so.1", 13) && dst_symbol != 0) {
					if(!fbt_strncmp( "__gxx", dst_symbol->name, 4)) return 1;
					if(!fbt_strncmp( "_Unwind", dst_symbol->name, 6)) return 1;
				} else if(!fbt_strncmp(srcdso->name, "libc.so.6", 9) && dst_symbol != 0) {
					if(!fbt_strncmp("_Z", dst_symbol->name, 1)) return 1;
				}
				/* TODO: allow only if libstdc++ is in the dependency list of srcdso. */
				if(!fbt_strncmp(dstdso->name, "libstdc++.so.6", 14) && dst_symbol != 0) {
					if(!fbt_strncmp( "_Z", dst_symbol->name, 1)) return 1;
				}
				
			} /* dst_symbol != 0 */
			
			/* Dynamically loaded DSOs can be tricky. If we load a DSO dynamically with local scope and a symbol gets looked up that actually
			 * exists but because of the local scope the symbol with the same name is found in another DSO (which of course is not imported by
			 * the callder DSO) then we need to make sure the call is allowed. The problem here is that the symbol is not imported by the
			 * dynamically loaded DSO as it has it's own symbol definition with the same name. So no imported symbol with that name is found
			 * although the symbol exists. */
			 if(srcdso != 0 && dstdso != 0 && src_symbol != 0 && dst_symbol != 0 && srcdso->dloaded == 1 && (srcdso->dload_mode & RTLD_GLOBAL) != RTLD_GLOBAL) {
						return 1;
						/* TODO: go through all symbols in srcdso and find one with the same name as the dstsymbol! */
						//if(!fbt_strncmp(dst_symbol->name, ..., MAX_SYM_NAME)) {
							//return 1;
						//}
			 }
			 
			 /* Antohter special case: if dstdso defines several symbols at the same address we need to make sure we checked against all symbols. */
			 if(dst_symbol != 0) {
			  
			  /* Get address of page and compute offset of this page in the array */
			  long page_addr = ((long)dst & PMASK);
			  long poffset = (page_addr - dstdso->pages[0].addr) >> PSHIFT;

			  /* If address of first symbol in page to big go to previous page */
			  while (poffset >= 1 && (!(&dstdso->pages[poffset].syms[0])
									  || dst < dstdso->pages[poffset].syms[0]->start))
				poffset--;
			  
			  struct sh_symbol *symbols = *dstdso->pages[poffset].syms;
			  int j, i;
			  int found = 0;
			  for(j = 0; j < dstdso->pages[poffset].nr_symbols && !found; j++) {
				  
				  if(symbols[j].start == dst) { /* symbol found */
							for (i=0; i<srcdso->nr_isymbols; ++i) {
							  if (srcdso->isymbols[i].start != 0)
								break;
							  if (!fbt_strncmp_symbols_at(srcdso->isymbols[i].name, symbols[j].name, 512)) {
								found = 1;
								break;
							  }
							}
				  }
			  }
			  if(found) return 1;
			 
			  /* TODO: this is another very special case, in case an IFUNC is resolved we will adapt the symbol entry 
			   * in the exported symbols datastructure. In case we try to match it later during a CFT check we need
			   * to make sure we just check the name without the '@', as this is not copied. We can not do it earlier
			   * as the versioning test case would fail (in case we really have symbols with the same name and different
			   * versions). It's checked here very late as this is a rare case. Still, to be improved! */
			   /* TODO: This special case will decrease the DAIR in case we have debug symbols! As we will evaluate this
			    * mor often! Which binaries require this? */
			  for (i=0; i<srcdso->nr_isymbols; ++i) {
				  if (srcdso->isymbols[i].start != 0)
					break;
				  if (!fbt_strncmp_symbols_at(srcdso->isymbols[i].name, dst_symbol->name, 512)) {
					//found = 1;
					break;
				  }
				}
			  if(found) return 1;
			  
			  
			} /*  dst_symbol != 0, multiple symbol check */
			
		} /* srcdso != dstdso */
		
	} /* type == CFTX_CALL_IND */ else if (type == CFTX_JMP_IND) {

		if(srcdso == dstdso) {
			
			/* secureLoader can jump to catch_error */
			if(!fbt_strncmp(srcdso->name, "secuLoader", 10) && !fbt_strncmp(dstdso->name, "secuLoader", 10)
				&& dst_symbol != 0 && !fbt_strncmp(dst_symbol->name, "catch_error", 11)) {
				return 1;
			}

			/* special cases for extended debug symbols */
			
			if(src_symbol && dst_symbol) {
				/* In case we have all debug symbols some libraries define .L symbols which actually
				 * have to be handled as if they were all in the same symbol */
				if(!fbt_strncmp(".L", src_symbol->name, 1) && !fbt_strncmp(".L", dst_symbol->name, 1)) return 1;
				/* For nginx 1.4.0, TODO: we need to have a closer look at these .L* symbols */
				if(!fbt_strncmp("__strftime_internal", src_symbol->name, 18) && !fbt_strncmp(".L", dst_symbol->name, 1)) return 1;
			}

			/* IFUNC special case handling*/
			
			/* In case we have an IFUNC it might be that the symbol search misses the effective symbol the
			 * src address is contained in. The symbol will actually be the symbol of the IFUNC symbol and
			 * not the one that was resolved. Any jmp within the resolved effective symbol will then be
			 * identified as an jmp into different method, see memcpy / __memcpy_ssse3 in e.g. the exception test case */
			/* TODO: this is a hack, as usual, has to be rewritten! */
			/* Get address of page and compute offset of this page in the array */
			if(srcdso != 0 && src_symbol != 0 && dst_symbol != 0) {
			  long page_addr = ((long)src & PMASK);
			  long poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;

			  /* If address of first symbol in page too big go to previous page */
			  while (poffset >= 1 && (!(&srcdso->pages[poffset].syms[0])
									  || src < srcdso->pages[poffset].syms[0]->start))
				poffset--;
			  
			  struct sh_symbol *symbols = *srcdso->pages[poffset].syms;
			  if (symbols) {
				/* Perform a binary search on the symbols for symbol with start==src */
				long nr_symbols = srcdso->pages[poffset].nr_symbols;
				int i;
				
				for(i = 0; i < nr_symbols; i++) {
					if (PTR_IN_REGION(src, symbols[i].start, symbols[i].size)) {	
						if(!fbt_strncmp(symbols[i].name, dst_symbol->name, 512)) {
							return 1;
						}
					}
				}
			  }
			} /* IFUNC special case handling */
			
			
		/* srcdso == dstdso */
		} else if(srcdso != dstdso) {

			if(!fbt_strncmp(srcdso->name, "secuLoader", 10) && src_symbol != 0) {
				if(dst_symbol != 0 && !fbt_strncmp(src_symbol->name, "transfer_control", 16) && !fbt_strncmp(dst_symbol->name, "_start", 6)) {
					return 1;
				} else if(!fbt_strncmp(src_symbol->name, "transfer_control", 16) && dstdso != 0) {
					if(dst == (unsigned char*)dstdso->text || dst == (unsigned char*)dstdso->entry) return 1;
				} else if(!fbt_strncmp(src_symbol->name, "so_fini", 7) && dstdso != 0) {
					if(dst == (unsigned char*)dstdso->fini) return 1;
				} else if(!fbt_strncmp(src_symbol->name, "resolve_symbol", 14)) {
					return 1; /* TODO: should be improved. */
				}
			}

			if(!fbt_strncmp(dstdso->name, "secuLoader", 11) && dst_symbol != 0 && !fbt_strncmp(dst_symbol->name, "runtime_trampoline", 19)) {
				return 1;
			} else if(!fbt_strncmp(srcdso->name, "libc.so.6", 9) && src_symbol != 0) {
				if(!fbt_strncmp(src_symbol->name, "__longjmp_chk", 13)) return 1;
				else if(!fbt_strncmp(src_symbol->name, "____longjmp_chk", 15)) return 1;
				/* TODO: implement own setjmp/longjmp */
            } else if(is_valid_plt_jmp(src, srcdso, src_symbol, dst, dstdso, dst_symbol)) {
				return 1; /* TODO: is valid plt jmp is still too loose. */
            }

		} /* srcdso != dstdso */
		
	} /* type == CFTX_JMP_IND */

	return 0;
}

void set_symbol_address(char *so_name, char *name, void *value, void *origvalue, struct dso_chain *dso_objects) {
	if(dso_objects != NULL) {
		llprintf("set_symbol_adress called with dso_objects: %x, %s, %s\n", dso_objects, so_name, name);
	}
}

void fbt_rescan_dsos(struct thread_local_data *tld) {
  /* This is disabled as currently CFT checks are only supported with secuLoader */
  //dl_iterate_phdr(callback, tld);
}

unsigned long fbt_find_callback(struct dso_chain *dso, void *orig_address) {

  ulong_t offset = C_CALLBACK_MAPPING_FUNCTION((ulong_t)orig_address);
  ulong_t pos = 0;
  unsigned long* entry = (unsigned long*)((unsigned long)dso->callbacks + offset);
  
  #if defined(SECURITY_METRICS_AND_STATS)
  long ret;
  unsigned long* stat_entry = (unsigned long*)((unsigned long)dso->callbacks_stats + offset);
  #endif
	  
  if(dso->callbacks == NULL) return 0;

  while (*entry != 0) {
    if ((unsigned long)orig_address == *entry) {
      
      #if defined(SECURITY_METRICS_AND_STATS)
      	//FBT_UNPROT_DATA(dso->callbacks_stats, CALLBACK_MAPPINGTABLE_SIZE, ret, "fbt_find_callback: unprotect callbacks_stats failed.");
		*stat_entry = *stat_entry + 1;
		//FBT_PROT_DATA(dso->callbacks_stats, CALLBACK_MAPPINGTABLE_SIZE, ret, "fbt_find_callback: protect callbacks_stats failed.");
	  #endif
				
      return *entry;
    }

    offset = (offset + sizeof(unsigned long)) & (CALLBACK_MAPPINGTABLE_SIZE-1);
    pos++;
    entry = (unsigned long*)((unsigned long)dso->callbacks + offset);
    #if defined(SECURITY_METRICS_AND_STATS)
    stat_entry = (unsigned long*)((unsigned long)dso->callbacks_stats + offset);
    #endif
  }

  return 0; 
}

/* TODO: protect callback hash table during execution of translated code. */
void fbt_add_callback(struct dso_chain *cur, void *orig_address) {

  ulong_t offset = C_CALLBACK_MAPPING_FUNCTION((ulong_t)orig_address);
  unsigned long *entry = (unsigned long*)((unsigned long)cur->callbacks + offset);

  int count = 0;
 
  if(cur->callbacks == NULL) return;

  while (*entry != 0 && *entry != (unsigned long)orig_address) {
    offset = (offset + sizeof(unsigned long)) & (CALLBACK_MAPPINGTABLE_SIZE - 1);
    entry = (unsigned long*)((unsigned long)cur->callbacks + offset);
    count++;
    if (count >= CALLBACK_MAPPINGTABLE_MAXENTRIES/4) { /* was CALLBACK_MAPPINGTABLE_MAXENTRIES/10, is 10 too much? */
	   llprintf("ERROR: callback mappingtable out of space (fbt_dso.c), nr of entries: %d\n", cur->nr_callbacks);
	   fbt_exit_suicide(40);
       //fbt_suicide_str("ERROR: callback mappingtable out of space (fbt_dso.c)\n");
    }
  }

  if(*entry != (unsigned long)orig_address) {
	FBT_UNPROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: unprotect dso failed.");
	*entry = (unsigned long)orig_address;
	cur->nr_callbacks++;
	/* TODO: make this thread safe! */
	//FBT_PROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: protect dso failed.");
  }
  
  return;
}

void fbt_print_callbacks(struct dso_chain *dso) {

  ulong_t offset = 0;
  unsigned long *entry = (unsigned long*)(dso->callbacks);

  if(dso->callbacks == NULL) {
	  llprintf("ERROR: callback table is NULL.\n");
	  return;
  }

  llprintf("[BEGIN] fbt_print_callbacks: total nr of potential callbacks %d for %s\n", dso->nr_callbacks, dso->name);

  int count = 0;
 
  while (count < dso->nr_callbacks) {
	  
	if(*entry) {
		llprintf("potential callback: %x\n", *entry);
		count++;
	}
	
	if(count >= dso->nr_callbacks) {
		llprintf("all callbacks found\n");
		break;
	}
	
	if((offset + sizeof(unsigned long)) > CALLBACK_MAPPINGTABLE_SIZE) {
		llprintf("end of callback table reached\n");
		break;
	}
	
    offset = (offset + sizeof(unsigned long)) & (CALLBACK_MAPPINGTABLE_SIZE - 1);
    entry = (unsigned long*)((unsigned long)dso->callbacks + offset);
    
  }

  llprintf("[END] fbt_print_callbacks: total nr of potential callbacks printed %d\n", count);
  return;
}



/*
 * FBT CHECK TRANSFER TEST
 * 
 * TODO
 * Note: this is only used for testing.
 * This is a duplicate of fbt_check_transfer and a redesign is required to
 * make them use the same code.
 * 
 * */	
int fbt_check_transfer_test(unsigned char* src, struct dso_chain* srcdso, unsigned char* dst, struct dso_chain* dstdso, enum cftx_type type) {

  /* return value, 0 if transfer is not allowed, 1 if allowed */
  int return_value = 0;

  struct sh_symbol *dst_symbol = 0;
  struct sh_symbol *src_symbol = 0;
  struct sh_symbol *src_nsymbol = 0;

/* We need to simulate the local symbol boundaries check. */
#if defined(VERIFY_CFTX_ENABLE_IJMP_LSYMOPT)
if(srcdso == dstdso) {
	
	struct sh_symbol* symbol = fbt_get_symbol_at_address(srcdso, src);

	if(symbol && symbol->start && symbol->size) {
		if(PTR_IN_REGION(dst, symbol->start, symbol->size)) return 1;
	}
	
}
#endif	

/* Do a fastlookup. */
#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)
	/* Map the dst address and see if it is a valid target. */
	ulong_t offset = C_CFT_MAPPING_FUNCTION((ulong_t)dst);
	unsigned long* entry;
	unsigned long* table;
    int count = 0;

	if(type == CFTX_CALL_IND) table = (unsigned long*)srcdso->cft_valid_call_targets;
	else table = (unsigned long*)srcdso->cft_valid_jmp_targets;
	/* TODO: disable fastlookup for jmp targets? Correctness might be an issue if
	 * inlined ijmp local symbol lookup is not enabled. Inter-symbol jumps might be
	 * allowed although not compliant with the CFI policy. */
	
	entry = (unsigned long*)((unsigned long)table + offset);
	
	while (*entry != 0) {
		if ((unsigned long)dst == *entry) {
		  return 1;
		}

		offset = (offset + sizeof(unsigned long)) & (CFT_MAPPINGTABLE_SIZE-1);
		entry = (unsigned long*)((unsigned long)table + offset);
		if (count >= CFT_MAPPINGTABLE_MAXENTRIES/4) break;
		count++;
    }
#endif /* defined(VERIFY_CFTX_ENABLE_FASTLOOKUP) */

  /* Lookup source symbol. */
		  /* Get address of page and compute offset of this page in the array */
		  long page_addr = ((long)src & PMASK);
		  long poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;

		  /* If address of first symbol in page too big go to previous page */
		  while (poffset >= 1 && (!(&srcdso->pages[poffset].syms[0])
								  || src < srcdso->pages[poffset].syms[0]->start))
			poffset--;
		  
		  struct sh_symbol *symbols = *srcdso->pages[poffset].syms;
		  if (symbols) {
			/* Perform a binary search on the symbols for symbol with start==src */
			long nr_symbols = srcdso->pages[poffset].nr_symbols;
			long index = binary_search(symbols, 0, nr_symbols, (long)src);
			
			if (src == symbols[index].start) {
			  /* Symbol found */
			  src_symbol = &symbols[index];
			  if (index+1 < nr_symbols)
				  src_nsymbol = &symbols[index+1];
			} else {
			  /* Not found, go left and search first symbol where src is inside */
			  for (;index>=0; --index) {
				struct sh_symbol *cur_symbol = &(symbols[index]);
				if (PTR_IN_REGION(src, cur_symbol->start, cur_symbol->size)) {
				  src_symbol = cur_symbol;
				  src_nsymbol = &symbols[index+1];
				  break;
				} 
			  }
			}
		  }
  /* Lookup source symbol. */
  /* Lookup dst symbol. */
      /* Get address of page and compute offset of this page in the array */
      page_addr = ((long)dst & PMASK);
      poffset = (page_addr - dstdso->pages[0].addr) >> PSHIFT;

      /* If address of first symbol in page to big go to previous page */
      while (poffset >= 1 && (!(&dstdso->pages[poffset].syms[0])
                              || dst < dstdso->pages[poffset].syms[0]->start))
        poffset--;
      
      symbols = *dstdso->pages[poffset].syms;
      if (symbols) {
        /* Perform a binary search on the symbols for symbol with start==dst */
        long nr_symbols = dstdso->pages[poffset].nr_symbols;
        long index = binary_search(symbols, 0, nr_symbols, (long)dst);
        if (dst == symbols[index].start) {
          /* Symbol found */
          dst_symbol = &symbols[index];
        } else {
		  /* TODO: why? this should only be possible for jmps, as calls should always go to symbol address */
          /* Not found, go left and search first symbol where dst is inside */
          for (;index>=0; --index) {
            struct sh_symbol *cur_symbol = &(symbols[index]);
            if (PTR_IN_REGION(dst, cur_symbol->start, cur_symbol->size)) {
              dst_symbol = cur_symbol;
              break;
            } 
          }
        }
      }
  /* Lookup dst symbol. */


  /* CFI LOGIC */
  if (srcdso == 0) {
    /* Src address does not belong to a loaded dso */
    return_value = 0;
  } else if (dstdso == 0) {
    /* Dst address does not belong to a loaded dso */
    return_value = 0;
  } else if (type == CFTX_CALL_IND) { /* ---------------------------------------- CALL ----------------------------------------------- */
    if (srcdso == dstdso) { /* case: call where srcdso == dstdso ***********************************************************************/
      if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) { /* no symbol at call destination */
        if (!dstdso->stripped) { /* NOT STRIPPED */
		  if(fbt_find_callback(dstdso, dst)) {
			return_value = 1;
		  } else {
			return_value = 0;
		  }
        } else { /* STRIPPED */
			if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;
			} else if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
              && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
              && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
				return_value = 0;
            } else { /* call not verifiable */
#if defined(ENABLE_STRICT_CFI_POLICY)
				return_value = 0;
#else
				return_value = 1;
#endif
			}
        } /* end STRIPPED case */
      } else { /* call goes to symbol */
		  return_value = 1;
	  }
    } else { /* case: call where srcdso != dstdso  **************************************************************************************/
		if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) { /* no symbol at call destination */
			if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;
			} else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				return_value = 1;
			} else {
				return_value = 0;
			}
       } else { /* call goes to symbol */
        int i;
        unsigned char found = 0;
        for (i=0; i<srcdso->nr_isymbols; ++i) {
          if (srcdso->isymbols[i].start != 0)
            break;
          if (!fbt_strncmp_symbols(srcdso->isymbols[i].name, dst_symbol->name, 512)) {
            found = 1;
            break;
          }
        }
        if (!found) { /* not imported in srcdso */
#if defined(ENABLE_STRICT_CFI_POLICY)
			if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;
			} else if (fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				return_value = 1;								
			} else {
				return_value = 0;				
			}
#else
			return_value = 1;
#endif
        } else if (dst_symbol->binding != SB_GLOBAL) {
        /* symbol is local symbol */
#if defined(ENABLE_STRICT_CFI_POLICY)
			if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;
			} else if (fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				return_value = 1;								
			} else {
				return_value = 0;				
			}
#else
          return_value = 1;
#endif
        } else { /* symbol imported in srcdso and exported in dstdso */
		  return_value = 1;
		}
      } /* end - call goes to symbol */
    } /* end - srcdso != dstdso */
  } else if (type == CFTX_JMP_IND) {  /* ---------------------------------------- JMP ----------------------------------------------- */
    if (srcdso == dstdso) { /* case: call where srcdso == dstdso **********************************************************************/
      if (dst_symbol == 0 || src_symbol == 0) {
        if (!srcdso->stripped) { /* NOT STRIPPED */
            if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;				
			} else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				return_value = 1;
			} else {
				return_value = 0;
			}
        } else { /* STRIPPED */
/*****************************************************************************************************************************************/
		  int is_surrounded = fbt_addr_is_surrounded_by_symbols(srcdso, src, dst);
		  if(is_surrounded > 0) {
			  if(is_surrounded == 2) {
			    return_value = 1;
			  } else {
				return_value = 0;				  
			  }
		  } else if (src_symbol && src_nsymbol) {
			/* check if dest is in region defined by surrounding symbols */
			if (dst < src_symbol->start || dst > src_nsymbol->start) {
				return_value = 0;
			} else {
				return_value = 1;
			}
          } else if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
                     && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
                     && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
            return_value = 0;
          } else { /* no surrounding symbols found */
			  if(fbt_find_callback(dstdso, dst)) {
				return_value = 1;				  
			  } else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
				return_value = 1;
			  } else {
				  if(srcdso->exec) {
					return_value = 1;
				  } else {
#if defined(ENABLE_STRICT_CFI_POLICY)
					return_value = 0;
#else
					return_value = 1;
#endif
				  }
			  }
		  }
/*****************************************************************************************************************************************/
        } /* end - STRIPPED */
      /* end - src_symbol == 0 || dst_symbol == 0 */
      } else if (src_symbol != dst_symbol) { /* src_symbol != dst_symbol */
           if(dst_symbol->start == dst) {
			   return_value = 1;
		   } else {
			 if(fbt_find_callback(dstdso, dst)) {
			   return_value = 1;			 
			 } else if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
			   return_value = 1;
			 } else {
			   return_value = 0;
		     }
		   }
      } else { /* src_symbol == dst_symbol */
		  return_value = 1;
	  }
	/* end - srcdso == dstdso*/
    } else { /* case: call where srcdso != dstdso  ************************************************************************************/
		/* TODO: should we also allow inter-DSO callback jumps? */
		if(fbt_is_special_cft_case(src, srcdso, src_symbol, dst, dstdso, dst_symbol, type)) {
			return_value = 1;
		} else {
			return_value = 0;
		}
    } /* end - srcdso != dstdso */
  }

/* This will fill up the fastlookup tables therefore do not put new elements into
 * fastlookup tables for check transfer tests. */
/* Fill up table for fastlookup. */
#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)
  //entry = (unsigned long*)((unsigned long)table + offset);
  //count = 0;
  ////long ret;
  
  //while (*entry != 0 && *entry != (unsigned long)dst) {
    //offset = (offset + sizeof(unsigned long)) & (CFT_MAPPINGTABLE_SIZE - 1);
    //entry = (unsigned long*)((unsigned long)table + offset);
    //count++;
    //if (count >= CFT_MAPPINGTABLE_MAXENTRIES/4) { /* was CFT_MAPPINGTABLE_MAXENTRIES/10, is 10 too much? */
	   //llprintf("ERROR: CFTX fastlookup table out of space (fbt_dso.c).\n");
	   //fbt_exit_suicide(41);
    //}
  //}

  //if(*entry != (unsigned long)dst) {
	////FBT_UNPROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: unprotect dso failed.");
	//*entry = (unsigned long)dst;
	////FBT_PROT_DATA(cur, sizeof(struct dso_chain), ret, "fbt_add_callback: protect dso failed.");
  //}
#endif /* defined(VERIFY_CFTX_ENABLE_FASTLOOKUP) */

 /* Finally return value. */
 return return_value;
}
/*
 * FBT CHECK TRANSFER TEST - END
 * */
 

#if defined(PRINT_CALLBACKS)
void fbt_print_all_callbacks() {
	
	  if(dso_chain != NULL) {
		  
		  	  struct dso_chain *iter_dso;

			  llprintf("\n* CALLBACKS\n");

		  	  for (iter_dso = dso_chain; iter_dso->next != 0; iter_dso = iter_dso->next) {
				  
				  		 llprintf(" * DSO %s, %x\n", iter_dso->name, iter_dso->baseaddr);
						 llprintf("  * callbacks %d\n", iter_dso->nr_callbacks);
						  
						  if(iter_dso->callbacks) {
							  ulong_t offset = 0;
							  unsigned long *entry = (unsigned long*)(iter_dso->callbacks);
							  unsigned long count = 0;
							  while (count < iter_dso->nr_callbacks) {
								if(*entry) {
									llprintf(" * callback %d: %x\n", count, *entry);
									count++;
								}
								if(count >= iter_dso->nr_callbacks) break;
								if((offset + sizeof(unsigned long)) > CALLBACK_MAPPINGTABLE_SIZE) break;
								
								offset = (offset + sizeof(unsigned long)) & (CALLBACK_MAPPINGTABLE_SIZE - 1);
								entry = (unsigned long*)((unsigned long)iter_dso->callbacks + offset);
							  }
 
							  llprintf("  * callbacks count %d\n", count);

						  }
 
			  }

	  }
}
#endif

/* Functions for security metrics and statistics. Only used for testing and evaluation. */
#if defined(SECURITY_METRICS_AND_STATS)
void fbt_print_security_metrics_and_stats() {

  int fd, ret;
  struct dso_chain *dso;
  double dair = 0; /* dynamic average indirect target reduction */
  double dair_without_loader = 0;
  unsigned long n = 0; /* counter for indirect target instructions */
  unsigned long n_without_loader = 0;
  unsigned long s = 0; /* total nr of valid indirect target destinations */
  unsigned long s_mapped = 0; /* total nr of valid indirect target destinations */

  double dair_icall = 0;
  double dair_ijmp = 0;
  double dair_ret = 0;
  unsigned long n_icall = 0;
  unsigned long n_ijmp = 0;
  unsigned long n_ret = 0;
   
  unsigned long nr_valid_icall_targets = 0;
  unsigned long nr_valid_ijmp_targets = 0;
  unsigned long nr_valid_ret_targets = 0;

  fbt_open(SECMETRICS_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd, "Could not open secmetrics stats file.\n");

  fllprintf(fd, "\n ********************* [SECURITY METRICS AND STATS] *********************\n\n");

  if(dso_chain != NULL) {
	   
   /* Determine total nr of executable bytes mapped into the process' memory. 
	* This corresponds to the nr of valid destinations in an unprotected process. */
	  struct dso_chain *iter_dso;
	  
	  fllprintf(fd, "\n   ********************* loaded DSOs \n");
	  /* TODO: with iter_dso->next != 0 we miss vdso! */
	  for (iter_dso = dso_chain; iter_dso->next != 0; iter_dso = iter_dso->next) {
			  s += iter_dso->text_size + iter_dso->init_size + iter_dso->fini_size;
			  //s += iter_dso->endaddr - iter_dso->baseaddr;
			  s_mapped += iter_dso->endaddr - iter_dso->baseaddr;
			  /* TODO: we need to add up all executable memory mappings, let's be conservative
			   * and just use text, init, fini sizes. */
			  fllprintf(fd, "   * DSO: %s, stripped %d\n", iter_dso->name, iter_dso->stripped);
	  }
	  fllprintf(fd, "\n   ********************* loaded DSOs - END \n\n");
	  
	  /* TODO: with dso->next != 0 we miss vdso! */
	  for (dso = dso_chain; dso->next != 0; dso = dso->next) {
		  
			unsigned long i = 0;
		    double dair_dso = 0; /* dynamic average indirect target reduction for the current dso */
			unsigned long n_dso = 0; /* counter for indirect target instructions for the current dso */

		    double dair_dso_icall = 0;
		    double dair_dso_ijmp = 0;
		    double dair_dso_ret = 0;

			unsigned long n_dso_icall = 0;
			unsigned long n_dso_ijmp = 0;
			unsigned long n_dso_ret = 0;
			
			unsigned long nr_dso_valid_icall_targets = 0;
			unsigned long nr_dso_valid_ijmp_targets = 0;
			unsigned long nr_dso_valid_ret_targets = 0;
			  
		  /* Determine nr of used callbacks. */
			  unsigned long used_callbacks = 0;
			  
			  if(dso->callbacks) {
				  ulong_t offset = 0;
				  unsigned long *entry = (unsigned long*)(dso->callbacks);
				  unsigned long *stat_entry = (unsigned long*)(dso->callbacks_stats);
				  unsigned long count = 0;
				  unsigned long used_count = 0;
				  while (count < dso->nr_callbacks) {
					if(*entry) {
						if(*stat_entry) used_count++;
						count++;
					}
					if(count >= dso->nr_callbacks) break;
					if((offset + sizeof(unsigned long)) > CALLBACK_MAPPINGTABLE_SIZE) break;
					
					offset = (offset + sizeof(unsigned long)) & (CALLBACK_MAPPINGTABLE_SIZE - 1);
					entry = (unsigned long*)((unsigned long)dso->callbacks + offset);
					stat_entry = (unsigned long*)((unsigned long)dso->callbacks_stats + offset);
				  }
				  used_callbacks = used_count;
			  }
		   /* Determine nr of used callbacks. */


			unsigned long nr_valid_targets_cached = 0;
			unsigned char *symbol_start_cached = 0;
			int cached = 0;
			int m;
			
			/* Actually, we just need to do this for one icall as the result for
			 * all icalls will likely be the same. TODO */
		   for(i = 0; i < dso->nr_icalls; i++) {
			   
			   unsigned char* icall = dso->icalls[i];
			   unsigned long nr_valid_targets;
			   
			   nr_valid_targets = fbt_get_valid_icall_targets(icall, dso);			   
			   nr_dso_valid_icall_targets += nr_valid_targets;
			   dair_dso += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso++;
			   dair_dso_icall += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso_icall++;
			   fllprintf(fd, " (Nr of valid targets for icall %p = %d)\n", icall, nr_valid_targets);
		   }
		   
		   for(i = 0; i < dso->nr_ijmps; i++) {
			   unsigned char* ijmp = dso->ijmps[i];
			   unsigned long nr_valid_targets = fbt_get_valid_ijmp_targets(ijmp, dso);
			   nr_dso_valid_ijmp_targets += nr_valid_targets;
			   dair_dso += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso++;
			   dair_dso_ijmp += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso_ijmp++;
			   fllprintf(fd, " (Nr of valid targets for ijmp %p = %d)\n", ijmp, nr_valid_targets);
		   }
		   
		   for(i = 0; i < dso->nr_retinstrs; i++) {
			   unsigned char* retinstr = dso->retinstrs[i];
			   unsigned long nr_valid_targets = fbt_get_valid_ret_targets(retinstr, dso);
			   nr_dso_valid_ret_targets += nr_valid_targets;
			   dair_dso += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso++;
			   dair_dso_ret += ((double)1 - (((double)nr_valid_targets)/(double)s));
			   n_dso_ret++;
			   fllprintf(fd, " (Nr of valid targets for ret %p = %d)\n", retinstr, nr_valid_targets);
		   }
		   
		   /* add the values for this dso to the overall value */	
		   dair += dair_dso;
		   n += n_dso;
		   
		   dair_icall += dair_dso_icall;
		   dair_ijmp += dair_dso_ijmp;
		   dair_ret += dair_dso_ret;
		   
		   n_icall += n_dso_icall;
		   n_ijmp += n_dso_ijmp;
		   n_ret += n_dso_ret;

		   nr_valid_icall_targets += nr_dso_valid_icall_targets;
		   nr_valid_ijmp_targets += nr_dso_valid_ijmp_targets;
		   nr_valid_ret_targets += nr_dso_valid_ret_targets;
			
		   
		   /* calculate dair also without counting ICFs in the loader */
		   if(fbt_strncmp(dso->name, "secuLoader", 10)) {
			   dair_without_loader += dair_dso;
		       n_without_loader += n_dso;
		   }

		   fllprintf(fd, " * DSO %s\n", dso->name);
		   fllprintf(fd, "  * icalls %d (valid targets: %d)\n", dso->nr_icalls, nr_dso_valid_icall_targets);
		   fllprintf(fd, "  * ijmps %d (valid targets: %d)\n", dso->nr_ijmps, nr_dso_valid_ijmp_targets);
		   fllprintf(fd, "  * retinstrs %d (valid targets: %d)\n", dso->nr_retinstrs, nr_dso_valid_ret_targets);
		   fllprintf(fd, "  * callbacks %d\n", dso->nr_callbacks);
		   fllprintf(fd, "   * used callbacks %d\n", used_callbacks);
		   fllprintf(fd, "  ************ DAIR ************\n");
		   fllprintf(fd, "  * DAIR for %s %d\n", dso->name, (unsigned long)((dair_dso/(double)n_dso)*(double)10000));
		   fllprintf(fd, "  ******************************\n");
		   fllprintf(fd, "  * icall DAIR for %s %d\n", dso->name, (unsigned long)((dair_dso_icall/(double)n_dso_icall)*(double)10000));
		   fllprintf(fd, "  ******************************\n");
		   fllprintf(fd, "  * ijmp DAIR for %s %d\n", dso->name, (unsigned long)((dair_dso_ijmp/(double)n_dso_ijmp)*(double)10000));
		   fllprintf(fd, "  ******************************\n");
		   fllprintf(fd, "  * ret DAIR for %s %d\n", dso->name, (unsigned long)((dair_dso_ret/(double)n_dso_ret)*(double)10000));
		   fllprintf(fd, "  ******************************\n\n");

	  }
	  
	  fllprintf(fd, " ************ OVERALL ************\n");
	  fllprintf(fd, " * Nr of total targets: %d\n", s);
	  fllprintf(fd, " * Nr of total mapped: %d\n", s_mapped);
	  fllprintf(fd, " * Nr of ICF instructions: %d\n", n);
	  fllprintf(fd, " * Nr of valid targets in average: %d \n", (unsigned long)((nr_valid_icall_targets+nr_valid_ijmp_targets+nr_valid_ret_targets)/n));

	  fllprintf(fd, " * \n");
	  fllprintf(fd, " * Total DAIR %d\n", (unsigned long)((dair/(double)n)*(double)10000));
	  fllprintf(fd, " * \n");
	  fllprintf(fd, " * Total DAIR %d (without loader ICFs)\n", (unsigned long)((dair_without_loader/(double)n_without_loader)*(double)10000));
	  fllprintf(fd, " * \n");
	  fllprintf(fd, " * Total icall DAIR %d\n", (unsigned long)((dair_icall/(double)n_icall)*(double)10000));
	  fllprintf(fd, " * \n");
	  fllprintf(fd, " * Total ijmp DAIR %d\n", (unsigned long)((dair_ijmp/(double)n_ijmp)*(double)10000));
	  fllprintf(fd, " * \n");
	  fllprintf(fd, " * Total ret DAIR %d\n", (unsigned long)((dair_ret/(double)n_ret)*(double)10000));
	  fllprintf(fd, " * \n");
	  fllprintf(fd, " *********************************\n\n");

  }
  
  fllprintf(fd, "\n ********************* [END END END END END] *********************\n\n");

  fbt_close(fd, ret, "Failed to close callback stats file.\n");
  
  return;
}

unsigned long fbt_get_valid_icall_targets(unsigned char *icall, struct dso_chain* srcdso) {
	
	struct dso_chain *iter_dso;
	unsigned long valid_targets = 0;
	unsigned char* target_ptr = 0;
	
	/* TODO: with iter_dso->next != 0 we miss vdso! */
	for (iter_dso = dso_chain; iter_dso->next != 0; iter_dso = iter_dso->next) {
			  /* We need to go through the entire mapped memory as it seems that other executable areas exist 
			   * beside text, init, fini! */
			  //for(target_ptr = (unsigned char*)iter_dso->baseaddr; (unsigned long)target_ptr < ((unsigned long)iter_dso->endaddr-3); target_ptr++) {
			  for(target_ptr = (unsigned char*)iter_dso->text; (unsigned long)target_ptr < ((unsigned long)iter_dso->text+(unsigned long)iter_dso->text_size); target_ptr++) {
				  valid_targets += fbt_check_transfer_test(icall, srcdso, target_ptr, iter_dso, CFTX_CALL_IND);
			  }
			  /* go also through init and fini sections? */
	}
	
	return valid_targets;
}

unsigned long fbt_get_valid_ijmp_targets(unsigned char *ijmp, struct dso_chain* srcdso) {
	
	struct dso_chain *iter_dso;
	unsigned long valid_targets = 0;
	unsigned char* target_ptr = 0;
	
    /* TODO: with iter_dso->next != 0 we miss vdso! */
	for (iter_dso = dso_chain; iter_dso->next != 0; iter_dso = iter_dso->next) {
		
			  /* We need to go through the entire mapped memory as it seems that other executable areas exist 
			   * beside text, init, fini! */
			  //for(target_ptr = (unsigned char*)iter_dso->baseaddr; (unsigned long)target_ptr < ((unsigned long)iter_dso->endaddr-3); target_ptr++) {
			  for(target_ptr = (unsigned char*)iter_dso->text; (unsigned long)target_ptr < ((unsigned long)iter_dso->text+(unsigned long)iter_dso->text_size); target_ptr++) {
				  valid_targets += fbt_check_transfer_test(ijmp, srcdso, target_ptr, iter_dso, CFTX_JMP_IND);
			  }  
			  /* go also through init and fini sections? */			  
	}
	
	return valid_targets;
}

unsigned long fbt_get_valid_ret_targets(unsigned char *retinstr, struct dso_chain* dso) {
	/* What about c++ exception handling? This has to be considered here! */
	/* 1? or 10? (to account for possible reauthentications?) */
	return 1;
}
#endif /* SECURITY_METRICS_AND_STATS */

struct sh_symbol* fbt_get_symbol_at_address(struct dso_chain *dso, unsigned char* address) {

      struct sh_symbol *symbol_found = 0;

	  /* Get address of page and compute offset of this page in the array */
	  long page_addr = ((long)address & PMASK);
	  long poffset = (page_addr - dso->pages[0].addr) >> PSHIFT;

	  /* If address of first symbol in page to big go to previous page */
	  while (poffset >= 1 && (!(&dso->pages[poffset].syms[0])
							  || address < dso->pages[poffset].syms[0]->start))
		poffset--;
	  
	  struct sh_symbol *symbols = *dso->pages[poffset].syms;
	  
	  if (symbols) {
		/* Perform a binary search on the symbols for symbol with start==dst */
		long nr_symbols = dso->pages[poffset].nr_symbols;
		long index = binary_search(symbols, 0, nr_symbols, (long)address);
		if (address == symbols[index].start) {
		  /* Symbol found */
		  symbol_found = &symbols[index];
		} else {
		  /* Not found, go left and search first symbol where dst is inside */
		  for (;index>=0; --index) {
			struct sh_symbol *cur_symbol = &(symbols[index]);
			if (PTR_IN_REGION(address, cur_symbol->start, cur_symbol->size)) {
			  symbol_found = cur_symbol;
			  break;
			} 
		  }
		}
	  }
    
	  return symbol_found;	
}

/* Functions for valid targets logging. */
#if defined(LOG_VALID_JMP_TARGETS)
void fbt_log_valid_jmp_targets(unsigned char *ijmp) {

  char padbuf[DUMP_SIZE_JMP_TARGETS];
  int fd, ret, fd_dump, ret_dump;
  unsigned int size;
  unsigned int padsize;
  int retval;
  struct dso_chain *dso;
  struct dso_chain *iter_dso;
  struct dso_chain *srcdso;
  int i;
  unsigned long s = 0; /* total nr of valid indirect target destinations */

#if defined(LOG_JMP_INSTR)
  if(nr_of_logged_jmps == 1) return;
#endif

  /* increase jmp count */
  nr_of_logged_jmps++;
  
  for(i = 0; i < DUMP_SIZE_JMP_TARGETS; i++) {
	  padbuf[i] = '\x90';
  }

  fbt_open(VALID_JMP_TARGETS_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd, "Could not open valid jump targets file.\n");
  fbt_open(VALID_JMP_TARGETS_DUMP_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd_dump, "Could not open valid jump targets dump file.\n");

  fllprintf(fd, "* VALID JMP TARGETS for: %p\n", ijmp);

  if(dso_chain != NULL) {
	   
	  /* determine source dso */
	  for (dso = dso_chain; dso != 0; dso = dso->next) {
		      
		      if (((ulong_t)ijmp >= (ulong_t)dso->baseaddr) && ((ulong_t)ijmp <= (ulong_t)dso->endaddr)) {
					srcdso = dso;
			   }
    
	  }
	  
	  for (iter_dso = dso_chain; iter_dso != 0; iter_dso = iter_dso->next) {
			  //s += iter_dso->text_size + iter_dso->init_size + iter_dso->fini_size;
			  s += iter_dso->endaddr - iter_dso->baseaddr;
	  }
	  
	  fllprintf(fd, "* SRCDSO %s, baseaddr = %p\n\n", srcdso->name, srcdso->baseaddr);

	  if(srcdso) {
		  
		  unsigned char* target_ptr = 0;
		  unsigned long valid_targets_total = 0;
		  struct sh_symbol* target_symbol;
		  
		  for (dso = dso_chain; dso != 0; dso = dso->next) {

				  unsigned long valid_targets = 0;
				  
				  fllprintf(fd, "*   DSTDSO %s, baseaddr = %p\n", dso->name, dso->baseaddr);

				  for(target_ptr = (unsigned char*)dso->baseaddr; (unsigned long)target_ptr < ((unsigned long)dso->endaddr-3); target_ptr++) {

					  if(fbt_check_transfer_test(ijmp, srcdso, target_ptr, dso, CFTX_JMP_IND)) {
						  
						  fllprintf(fd, "%p,", target_ptr);
						  						  
						  /* resolve address to symbol */
						  target_symbol = fbt_get_symbol_at_address(dso, target_ptr);
						  if(target_symbol) fllprintf(fd, " %s\n", target_symbol->name);
						  else fllprintf(fd, " <no_symbol>\n");
						  
						  /* are we getting near the end of the dso? */
						  size = (unsigned int)dso->endaddr-(unsigned int)target_ptr;
						  if(DUMP_SIZE_JMP_TARGETS <= size) {
							  size = DUMP_SIZE_JMP_TARGETS;
							  padsize = 0;
						  } else {
							  padsize = DUMP_SIZE_JMP_TARGETS - size;
						  }
						  
						  /* dump size bytes of instructions beginning at the address */
						  fbt_writeK(fd_dump, target_ptr, size, retval, 255);
						  /* dump nop padding */
					      if(padsize > 0) fbt_writeK(fd_dump, padbuf, padsize, retval, 255);

						  valid_targets++;
					  }

				  }
				  
				  fllprintf(fd, "*   Total valid targets for DSTDSO %s, baseaddr = %p, %d\n\n", dso->name, dso->baseaddr, valid_targets);
				  valid_targets_total += valid_targets;
		  }
		  
		  fllprintf(fd, "* Total valid targets for JMP %d\n", valid_targets_total);
		  fllprintf(fd, "* Total nr of targets/bytes %d\n", s);
		  fllprintf(fd, "* DAIR for this specific JMP %d\n", (unsigned long)(((double)1 - (((double)valid_targets_total)/(double)s))*10000));
		  fllprintf(fd, "* Logged JMPs so far %d\n\n", nr_of_logged_jmps);
	  }

  }
  
  fbt_close(fd, ret, "Failed to close valid jump targets file.\n");
  fbt_close(fd_dump, ret_dump, "Failed to close valid jump targets dump file.\n");

  return;
}
#endif /* LOG_VALID_JMP_TARGETS */

#if defined(LOG_VALID_CALL_TARGETS)
void fbt_log_valid_call_targets(unsigned char *icall) {
	
  char padbuf[DUMP_SIZE_CALL_TARGETS];
  int fd, ret, fd_dump, ret_dump;
  unsigned int size;
  unsigned int padsize;
  int retval;
  struct dso_chain *dso;
  struct dso_chain *iter_dso;
  struct dso_chain *srcdso;
  int i;
  unsigned long s = 0; /* total nr of valid indirect target destinations */

#if defined(LOG_CALL_INSTR)
  if(nr_of_logged_calls == 1) return;
#endif

  /* increase call count */
  nr_of_logged_calls++;
  
  for(i = 0; i < DUMP_SIZE_CALL_TARGETS; i++) {
	  padbuf[i] = '\x90';
  }

  fbt_open(VALID_CALL_TARGETS_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd, "Could not open valid call targets file.\n");
  fbt_open(VALID_CALL_TARGETS_DUMP_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd_dump, "Could not open valid call targets dump file.\n");

  fllprintf(fd, "* VALID CALL TARGETS for: %p\n", icall);

  if(dso_chain != NULL) {
	   
	  /* determine source dso */
	  for (dso = dso_chain; dso != 0; dso = dso->next) {
		      
		      if (((ulong_t)icall >= (ulong_t)dso->baseaddr) && ((ulong_t)icall <= (ulong_t)dso->endaddr)) {
					srcdso = dso;
			   }
    
	  }
	  
	  for (iter_dso = dso_chain; iter_dso != 0; iter_dso = iter_dso->next) {
			  //s += iter_dso->text_size + iter_dso->init_size + iter_dso->fini_size;
			  s += iter_dso->endaddr - iter_dso->baseaddr;
	  }
	  
	  fllprintf(fd, "* SRCDSO %s, baseaddr = %p\n\n", srcdso->name, srcdso->baseaddr);

	  if(srcdso) {
		  
		  unsigned char* target_ptr = 0;
		  unsigned long valid_targets_total = 0;
		  struct sh_symbol* target_symbol;
		  
		  for (dso = dso_chain; dso != 0; dso = dso->next) {

				  unsigned long valid_targets = 0;
				  
				  fllprintf(fd, "*   DSTDSO %s, baseaddr = %p\n", dso->name, dso->baseaddr);

				  for(target_ptr = (unsigned char*)dso->baseaddr; (unsigned long)target_ptr < ((unsigned long)dso->endaddr-3); target_ptr++) {

					  if(fbt_check_transfer_test(icall, srcdso, target_ptr, dso, CFTX_CALL_IND)) {
						  
						  fllprintf(fd, "%p,", target_ptr);
						  						  
						  /* resolve address to symbol */
						  target_symbol = fbt_get_symbol_at_address(dso, target_ptr);
						  if(target_symbol) fllprintf(fd, " %s\n", target_symbol->name);
						  else fllprintf(fd, " <no_symbol>\n");
						  
						  /* are we getting near the end of the dso? */
						  size = (unsigned int)dso->endaddr-(unsigned int)target_ptr;
						  if(DUMP_SIZE_CALL_TARGETS <= size) {
							  size = DUMP_SIZE_CALL_TARGETS;
							  padsize = 0;
						  } else {
							  padsize = DUMP_SIZE_CALL_TARGETS - size;
						  }
						  
						  /* dump size bytes of instructions beginning at the address */
						  fbt_writeK(fd_dump, target_ptr, size, retval, 255);
						  /* dump nop padding */
					      if(padsize > 0) fbt_writeK(fd_dump, padbuf, padsize, retval, 255);

						  valid_targets++;
					  }

				  }
				  
				  fllprintf(fd, "*   Total valid targets for DSTDSO %s, baseaddr = %p, %d\n\n", dso->name, dso->baseaddr, valid_targets);
				  valid_targets_total += valid_targets;
		  }
		  
		  fllprintf(fd, "* Total valid targets for CALL %d\n", valid_targets_total);
		  fllprintf(fd, "* Total nr of targets/bytes %d\n", s);
		  fllprintf(fd, "* DAIR for this specific CALL %d\n", (unsigned long)(((double)1 - (((double)valid_targets_total)/(double)s))*10000));
		  fllprintf(fd, "* Logged CALLs so far %d\n\n", nr_of_logged_calls);
	  }

  }
  
  fbt_close(fd, ret, "Failed to close valid call targets file.\n");
  fbt_close(fd_dump, ret_dump, "Failed to close valid call targets dump file.\n");

  return;
}
#endif /* LOG_VALID_CALL_TARGETS */

#if defined(LOG_VALID_RET_TARGETS)
void fbt_log_valid_ret_targets(struct thread_local_data *tld, unsigned char *src, unsigned char *dst, enum cftx_type type) {

  char padbuf[DUMP_SIZE_RET_TARGETS];
  int fd, ret, fd_dump, ret_dump;
  unsigned int size;
  unsigned int padsize;
  int retval;
  struct dso_chain *dso;
  struct dso_chain *iter_dso;
  struct dso_chain *srcdso;
  int i;
  unsigned long s = 0; /* total nr of valid indirect target destinations */

#if defined(LOG_RET_INSTR)
  if(nr_of_logged_rets == 1) return;
#endif

  /* increase ret count */
  nr_of_logged_rets++;
  
  for(i = 0; i < DUMP_SIZE_RET_TARGETS; i++) {
	  padbuf[i] = '\x90';
  }

  fbt_open(VALID_RET_TARGETS_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd, "Could not open valid ret targets file.\n");
  fbt_open(VALID_RET_TARGETS_DUMP_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, fd_dump, "Could not open valid ret targets dump file.\n");

  fllprintf(fd, "* VALID RET TARGETS for: %p\n", src);

  if(dso_chain != NULL) {
	   
	  /* determine source dso */
	  for (dso = dso_chain; dso != 0; dso = dso->next) {
		      
		      if (((ulong_t)src >= (ulong_t)dso->baseaddr) && ((ulong_t)src <= (ulong_t)dso->endaddr)) {
					srcdso = dso;
			   }
    
	  }
	  
	  for (iter_dso = dso_chain; iter_dso != 0; iter_dso = iter_dso->next) {
			  //s += iter_dso->text_size + iter_dso->init_size + iter_dso->fini_size;
			  s += iter_dso->endaddr - iter_dso->baseaddr;
	  }
	  
	  fllprintf(fd, "* SRCDSO %s, baseaddr = %p\n\n", srcdso->name, srcdso->baseaddr);

	  if(srcdso) {

		  unsigned char* target_ptr = 0;
		  unsigned long valid_targets_total = 0;
		  struct sh_symbol* target_symbol;
		  
		  struct shadowstack_entry *current = tld->top_of_shadowstack - 1;

		  while (current >= tld->shadowstack) {
			  
			  fllprintf(fd, "%d: %p, %p (transl), %p (esp), %p (ebp), ", (tld->top_of_shadowstack - current), current->return_address, current->translated_return_address, current->stack_pointer, current->call_target);
			  unsigned char* target_ptr = current->return_address;
			  
			  for (dso = dso_chain; dso != 0; dso = dso->next) {
				  if((unsigned long)target_ptr >= (unsigned long)dso->baseaddr && (unsigned long)target_ptr < ((unsigned long)dso->endaddr-3)) break;
			  }
			  
			  target_symbol = 0;
			  
			  /* resolve address to symbol */
			  if(dso) target_symbol = fbt_get_symbol_at_address(dso, target_ptr);
			  if(target_symbol) fllprintf(fd, " %s (in %s)\n", target_symbol->name, dso->name);
			  else fllprintf(fd, " <no_symbol>\n");
			  
			  /* are we getting near the end of the dso? */
			  if(dso) size = (unsigned int)dso->endaddr-(unsigned int)target_ptr;
			  else size = 0;
			  if(DUMP_SIZE_RET_TARGETS <= size) {
				  size = DUMP_SIZE_RET_TARGETS;
				  padsize = 0;
			  } else {
				  padsize = DUMP_SIZE_RET_TARGETS - size;
			  }
			  
			  /* dump size bytes of instructions beginning at the address */
			  fbt_writeK(fd_dump, target_ptr, size, retval, 255);
			  /* dump nop padding */
			  if(padsize > 0) fbt_writeK(fd_dump, padbuf, padsize, retval, 255);

			  current--;
			  valid_targets_total++;
		  }
		  
		  fllprintf(fd, "\n* Total valid targets for RET %d\n", valid_targets_total);
		  fllprintf(fd, "* Total nr of targets/bytes %d\n", s);
		  fllprintf(fd, "* DAIR for this specific RET %d\n", (unsigned long)(((double)1 - (((double)valid_targets_total)/(double)s))*10000));
		  fllprintf(fd, "* Logged RETs so far %d\n\n", nr_of_logged_rets);
	  }

  }
  
  fbt_close(fd, ret, "Failed to close valid ret targets file.\n");
  fbt_close(fd_dump, ret_dump, "Failed to close valid ret targets dump file.\n");

  return;
}
#endif /* LOG_VALID_RET_TARGETS */

#if defined(PRINT_CFTX_TYPE_STATS)
void fbt_print_cftx_type_stats() {
	llprintf("\n***** CFTX *****\n");
	llprintf("\n* total nr of cft checks: %d\n", nr_total_cftx_checks);
	llprintf("\n*** indirect calls\n");
	llprintf("* total intra dso: %d\n", nr_icalls_intra_dso);
	llprintf("* total inter dso: %d\n", nr_icalls_inter_dso);
	llprintf("-------------------\n");
	llprintf("* total to unimported symbols (only if not strict): %d\n", nr_icalls_unimported_symbols);
	llprintf("* total to local symbols (only if not strict): %d\n", nr_icalls_local_symbol);
	llprintf("* total not verified (only if not strict): %d\n", nr_icalls_not_verified);
	llprintf("* total callback: %d\n", nr_icalls_callback);
	llprintf("* total special case: %d\n", nr_icalls_special_case);
	llprintf("\n*** indirect jumps\n");
	llprintf("* total intra dso: %d\n", nr_ijmps_intra_dso);
	llprintf("* total inter dso: %d\n", nr_ijmps_inter_dso);
	llprintf("-------------------\n");
	llprintf("* total intra symbol: %d\n", nr_ijmps_intra_symbol);
	llprintf("* total inter symbol (to symbol start): %d\n", nr_ijmps_inter_symbol);
	llprintf("* total surrounded by symbols (because stripped): %d\n", nr_ijmps_within_stripped_symbol);
	llprintf("* total not verified (because stripped): %d\n", nr_ijmps_not_verified);
	llprintf("-------------------\n");
	llprintf("* total callback: %d\n", nr_ijmps_callback);
	llprintf("* total special case: %d\n", nr_ijmps_special_case);
	llprintf("\n***** CFTX *****\n\n");
}
#endif

struct sh_symbol* fbt_find_symbol(struct thread_local_data *tld, unsigned char *src) {

  struct dso_chain *cur = tld->dso_objects;
  struct dso_chain *srcdso = 0;
  struct sh_symbol *src_symbol = 0;
  
  while (cur != 0) {
		/* Check if src belongs to DSO */
		if (((ulong_t)src >= (ulong_t)cur->baseaddr)
			&& ((ulong_t)src <= (ulong_t)cur->endaddr)) {
		  srcdso = cur;

		  /* Get address of page and compute offset of this page in the array */
		  long page_addr = ((long)src & PMASK);
		  long poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;

		  /* If address of first symbol in page too big go to previous page */
		  while (poffset >= 1 && (!(&srcdso->pages[poffset].syms[0])
								  || src < srcdso->pages[poffset].syms[0]->start))
			poffset--;
		  
		  struct sh_symbol *symbols = *srcdso->pages[poffset].syms;
		  if (symbols) {
			/* Perform a binary search on the symbols for symbol with start==src */
			long nr_symbols = srcdso->pages[poffset].nr_symbols;
			long index = binary_search(symbols, 0, nr_symbols, (long)src);
			
			if (src == symbols[index].start) {
			  /* Symbol found */
			  src_symbol = &symbols[index];
			} else {
			  /* Not found, go left and search first symbol where src is inside */
			  for (;index>=0; --index) {
				struct sh_symbol *cur_symbol = &(symbols[index]);
				if (PTR_IN_REGION(src, cur_symbol->start, cur_symbol->size)) {
				  src_symbol = cur_symbol;
				  break;
				} 
			  }
			}
		  }
		  break;
		}
	 cur = cur->next;
	}

	return src_symbol;
}

struct dso_chain* fbt_find_dso(struct thread_local_data *tld, unsigned char *src) {

  struct dso_chain *cur = tld->dso_objects;
  struct dso_chain *srcdso = 0;

  while (cur != 0) {
		/* Check if src belongs to DSO */
		if (((ulong_t)src >= (ulong_t)cur->baseaddr)
			&& ((ulong_t)src <= (ulong_t)cur->endaddr)) {
		  srcdso = cur;
		  break;
		}
	 cur = cur->next;
	}

	return srcdso;
}

struct sh_symbol* fbt_find_nsymbol(struct thread_local_data *tld, unsigned char *src) {

  struct dso_chain *cur = tld->dso_objects;
  struct dso_chain *srcdso = 0;
  //struct sh_symbol *src_symbol = 0;
  struct sh_symbol *src_nsymbol = 0;
  
  while (cur != 0) {
		/* Check if src belongs to DSO */
		if (((ulong_t)src >= (ulong_t)cur->baseaddr)
			&& ((ulong_t)src <= (ulong_t)cur->endaddr)) {
		  srcdso = cur;

		  /* Get address of page and compute offset of this page in the array */
		  long page_addr = ((long)src & PMASK);
		  long poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;

		  /* If address of first symbol in page too big go to previous page */
		  while (poffset >= 1 && (!(&srcdso->pages[poffset].syms[0])
								  || src < srcdso->pages[poffset].syms[0]->start))
			poffset--;
		  
		  struct sh_symbol *symbols = *srcdso->pages[poffset].syms;
		  if (symbols) {
			/* Perform a binary search on the symbols for symbol with start==src */
			long nr_symbols = srcdso->pages[poffset].nr_symbols;
			long index = binary_search(symbols, 0, nr_symbols, (long)src);
			
			if (src == symbols[index].start) {
			  /* Symbol found */
			  //src_symbol = &symbols[index];
			  if (index+1 < nr_symbols)
				  src_nsymbol = &symbols[index+1];
			} else {
			  /* Not found, go left and search first symbol where src is inside */
			  for (;index>=0; --index) {
				struct sh_symbol *cur_symbol = &(symbols[index]);
				if (PTR_IN_REGION(src, cur_symbol->start, cur_symbol->size)) {
				  //src_symbol = cur_symbol;
				  src_nsymbol = &symbols[index+1];
				  break;
				} 
			  }
			}
		  }
		  break;
		}
	 cur = cur->next;
	}

	return src_nsymbol;
}

int fbt_addr_is_surrounded_by_symbols(struct dso_chain *srcdso, unsigned char *src, unsigned char *dst) {

  int i;
  int found;
  long nr_symbols;
  long index;
  unsigned char *upper_bound;
  unsigned char *lower_bound;
  
  /* Get address of page and compute offset of this page in the array */
  long page_addr = ((long)src & PMASK);
  long poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;

  /* If address of first symbol in page too big go to previous page */
  while (poffset >= 1 && (!(&srcdso->pages[poffset].syms[0]) || src < srcdso->pages[poffset].syms[0]->start)) poffset--;

  struct sh_symbol *symbols = *srcdso->pages[poffset].syms;

  if (symbols) {

     /* Perform a binary search on the symbols for symbol with start==src */
	 nr_symbols = srcdso->pages[poffset].nr_symbols;
	 index = binary_search(symbols, 0, nr_symbols, (long)src);
	 
	 if((index-1) >=0 ) {

		if (src >= symbols[index-1].start && src < symbols[index].start) {	/* src boundaries found */
			if(dst >= symbols[index-1].start && dst < symbols[index].start) return 2;
			else return 1;	
		} else {	
			for(i = index; i < nr_symbols; i++) {
				if(src >= symbols[i-1].start && src < symbols[i].start) { /* src boundaries found */
					if(dst >= symbols[i-1].start && dst < symbols[i].start) return 2;
					else return 1;
				}		
			}
		}

		/* upper bound symbol is in next page */
		if(((poffset+1) < srcdso->nr_pages) && (srcdso->pages[poffset+1].nr_symbols > 0)) {
			struct sh_symbol *nsymbols = *srcdso->pages[poffset+1].syms;
			if(src >= symbols[nr_symbols-1].start && src < nsymbols[0].start) { /* src boundaries found */
				if(dst >= symbols[nr_symbols-1].start && dst < nsymbols[0].start) return 2;
				else return 1;
			}
		}

	 }

	} /* end if(symbols) */

	
	/* let's catch all special cases (slow, to be improved) */
	/* - index might be 0 */
	/* - symbols might be 0 */
	/* the current page has probably no symbol definitions, let's find the cross page boundaries */
	
	upper_bound = NULL;
	lower_bound = NULL;
	
	page_addr = ((long)src & PMASK);
    poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;
	found = 0;
	for(;poffset < srcdso->nr_pages && !found; poffset++) {
		if(srcdso->pages[poffset].nr_symbols > 0) {
			struct sh_symbol *nsymbols = *srcdso->pages[poffset].syms;
			for(i=0; i<srcdso->pages[poffset].nr_symbols; i++) {
				if(src < nsymbols[i].start) {
					upper_bound = nsymbols[i].start;
					found = 1;
					break;
				}
			}
		}
	}
	
	page_addr = ((long)src & PMASK);
    poffset = (page_addr - srcdso->pages[0].addr) >> PSHIFT;
	found = 0;
	for(;poffset >= 0 && !found; poffset--) {
		if(srcdso->pages[poffset].nr_symbols > 0) {
			struct sh_symbol *nsymbols = *srcdso->pages[poffset].syms;
			for(i=(srcdso->pages[poffset].nr_symbols-1); i>=0; i--) {
				if(src > nsymbols[i].start) {
					lower_bound = nsymbols[i].start;
					found = 1;
					break;
				}
			}
		}
	}

	if(lower_bound || upper_bound) {
		if(dst >= lower_bound && ((upper_bound == NULL) || (dst < upper_bound))) return 2;
		else return 1;
	}

  return 0;
}

#endif  /* VERIFY_CFTX */
