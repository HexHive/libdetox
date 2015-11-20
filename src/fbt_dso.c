/**
 * @file fbt_dso.c
 * Interface to the shared library loader
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
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

#include <assert.h>
#include <link.h>
#include <elf.h>
#include <sys/types.h>
#include <asm/stat.h>

#include "fbt_datatypes.h"
#include "fbt_libc.h"
#include "fbt_llio.h"
#include "fbt_mem_mgmt.h"
#include "libfastbt.h"

#if defined(VERIFY_CFTX)

/* Do not check control flow transfers initiated by the loader */
// TODO
#define SKIP_LOADER

/**
 * Helper function which prints an error or fails,
 * depending on the settings
 * @param msg Message to print
 */
static void error(char *msg) {
#if defined (PRINT_CFTX)
  llprintf("\n[ERROR] %s:\n", msg);
#else
  //  fbt_suicide(1);
#endif
}


/**
 * Helper function which prints a warning message.
 * @param msg Message to print
 */
static void warn(char *msg) {
#if defined (PRINT_CFTX)
  llprintf("\n[WARNING] %s:\n", msg);
#endif
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


void fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type) {
  /* Note: CFTX_RET disabled in action_ret */

  if (type == CFTX_JMP_IND || type == CFTX_CALL_IND || type == CFTX_RET) {
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
  struct dso_chain *srcdso = 0, *dstdso = 0;
  struct sh_symbol *src_symbol = 0, *dst_symbol = 0;
  struct sh_symbol *src_nsymbol = 0;

  /* check our (small) cache if the src and dst are in the recently used objects */
  long i;
  for (i = 0; i < SYMBOL_CACHE_SIZE; ++i) {
    if (tld->symb_cache[i].addr == src) {
      src_symbol = tld->symb_cache[i].symbol;
      srcdso = tld->symb_cache[i].dso;
    }
    if (tld->symb_cache[i].addr == dst) {
      dst_symbol = tld->symb_cache[i].symbol;
      dstdso = tld->symb_cache[i].dso;
    }
  }

#ifdef SKIP_LOADER
  /* Skip control flow transfers with src at secuLoader (always first entry) */
  if (((ulong_t)src >= (ulong_t)cur->baseaddr)
      && ((ulong_t)src <= (ulong_t)cur->endaddr))
    return;
#endif

  /* Search all DSOs */
  while (cur != 0 && !(srcdso && dstdso)) {
    /* Check if src belongs to DSO */
    if (!srcdso && ((ulong_t)src >= (ulong_t)cur->baseaddr)
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
    }    

    /* Check if dst belongs to DSO */
    if (!dstdso && ((ulong_t)dst >= (ulong_t)cur->baseaddr)
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
    }
    cur = cur->next;
  }  /* iterate all dsos */

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

  if (srcdso == 0) {
    /* Src address does not belong to a loaded dso */
    error("No source dso found");
  } else if (dstdso == 0) {
    /* Dst address does not belong to a loaded dso */
    error("No destination dso found");
  } else if (type == CFTX_CALL || type == CFTX_CALL_IND) {
    /* -- CALL -- */
    if (srcdso == dstdso) {
      /* We stay in the same module */
      if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) {
        /* No symbol at call destination*/
        if (!dstdso->stripped) {
          /* Not stripped - fail */
          error("No symbol defined at call destination");
        } else {
          /* Stripped - check if at least dst is in an executable section */
          if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
              && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
              && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
            error("Call destination not in executable section");
          } else {
            /* Emit a warning */
            //warn("Could not verify inner module call dest, object stripped");
          }
        }        
      }
    } else {
      /* We call to another module */
      /* Symbol has to be imported in srcdso and global in dstdso */
      if (dst_symbol == 0 || (dst-dst_symbol->start) != 0) {
        error("No symbol defined at call destination");
      } else {
        /* Check if destination symbol is imported in source DSO */
        /* TODO (hartobia) we should optimize this */
        /*
        int i;
        unsigned char found = 0;
        for (i=0; i<srcdso->nr_symbols; ++i) {
          if (srcdso->symbols[i].start != 0)
            break;
          if (!fbt_strncmp(srcdso->symbols[i].name, dst_symbol->name, 512)) {
            found = 1;
            break;
          }
        }
        if (!found) {
          warn("Call to unimported symbol");
          }*/
        
        /* Check if symbol is exported in destination DSO */
        if (dst_symbol->binding != SB_GLOBAL) {
          /* Symbol local */
          warn("Call to local symbol in different module");
        }
      }
    }
  } else if (type == CFTX_JMP || type == CFTX_JMP_IND) {
    /* -- JMP -- */
    if (srcdso == dstdso) {
      /* We stay in the same module */
      if (dst_symbol == 0 || src_symbol == 0) {
        if (!srcdso->stripped) {
          /* Not stripped - fail */
          error("No symbol defined at jmp source/destination");
        } else {
          /* Stripped - check if dst is surrounded by exported symbols */
          if (src_symbol && src_nsymbol) {
            /* Check if dest is in region defined by surrounding symbols */
            if (dst < src_symbol->start || dst > src_nsymbol->start) {
              if (type == CFTX_JMP) {
                /* Direct jmp, just warn */
                warn("Jmp to different method (stripped)");
              } else {
                /* Indirect jmp, fail */
                error("Indirect jmp to different method (stripped)");
              }
            }
          } else if (!PTR_IN_REGION(dst, dstdso->text, dstdso->text_size)
                     && !PTR_IN_REGION(dst, dstdso->init, dstdso->init_size)
                     && !PTR_IN_REGION(dst, dstdso->fini, dstdso->fini_size)) {
            /* No surrounding symbols - check if at least dst is in an
               executable section */
            error("Jmp destination not in executable section");
          }
          /* Emit a warning */
          // warn("Could not verify jmp dest, object stripped");
        }
      } else if (src_symbol != dst_symbol) {
        /* We jump to another method */
        if (type == CFTX_JMP) {
          /* Direct jmp, just warn */
          warn("Jmp to different method");
        } else {
          /* Indirect jmp, fail */
          error("Indirect jmp to different method");
        }
      }
    } else {
      /* We jump in another module */
      error("Jmp destination in different dso");
    }
  }


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
}


void fbt_rescan_dsos(struct thread_local_data *tld) {
  /* This is disabled as currently CFT checks are only supported
     with secuLoader */
  //  dl_iterate_phdr(callback, tld);
}

#endif  /* VERIFY_CFTX */
