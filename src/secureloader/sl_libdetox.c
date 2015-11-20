/**
 * @file sl_libdetox.h
 * Communication with the libdetox framework.
 * Implements functions to load the libdetox library and supplies all 
 * needed information for the control transfer checks.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
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

#include "sl_libdetox.h"
#include "sl_datatypes.h"
#include "sl_macros.h"
#include "sl_libc.h"
#include "sl_load.h"
#include "sl_sym_lookup.h"
#include "sl_so_chain.h"
#include "fbt_libc.h"
#include "fbt_dso.h"
#include "sl_libsearch.h"

extern dso *so_chain;

#ifdef VERIFY_CFTX

/* Chain of loaded objects for control flow transfer checks in libdetox */
struct dso_chain *dso_chain = 0;

/* The vdso dso_chain struct */
struct dso_chain *vdso = 0;

/**
 * Initializes the DSO of the secuLoader itself and adds it to the 
 * libdetox dso_chain for control flow transfer checks.
 */
void init_loader_dso(Elf32_Ehdr * sysinfo_ehdr) {
  /* Open loader file */
  long fd = sl_open("/proc/self/exe", O_RDONLY);
  if (fd == -1) {
    sl_printf("Error init_loader_dso: failed to open /proc/self/exe\n");
    sl_exit(1);
  }

  /* Get file size */
  struct kernel_stat file_info;
  if(sl_fstat(fd, &file_info) == -1) {
    sl_printf("Error init_loader_dso: fstat failed\n");
    sl_exit(1);
  }

  /* Map file in memory */
  char *file_map = (char *)sl_mmap(0, file_info.st_size, PROT_READ,
                                   MAP_PRIVATE, fd, 0);
  /* Add to DSO chain */
  add_dso(0, file_map);
  
  /* Add vDSO to DSO chain */
  /* TODO: create a dso struct to pass it to add_dso */
  add_vdso(sysinfo_ehdr);
  //add_vdso(0, sysinfo_ehdr);
  //sl_printf("init_loader_dso %x.\n", sysinfo_ehdr);

  /* Unmap file */
  sl_munmap(file_map, file_info.st_size);
}

#endif /* VERIFY_CFTX */

#ifdef VERIFY_CFTX

/**
 * Helper function that swaps two symbols.
 * @param x first symbol
 * @param y second symbol
 */
static void swap(struct sh_symbol *x, struct sh_symbol *y) {
  struct sh_symbol temp;
  temp.start = x->start;
  temp.size = x->size;
  temp.name = x->name;
  temp.binding = x->binding;
  temp.dso = x->dso;
  
  x->start = y->start;
  x->size = y->size;
  x->name = y->name;
  x->binding = y->binding;
  x->dso = y->dso;
  
  y->start = temp.start;
  y->size = temp.size;
  y->name = temp.name;
  y->binding = temp.binding;
  y->dso = temp.dso;
}


/**
 * Sorts all elements of the given sh_symbol array. 
 * This is a modified quicksort implementation with a second sorting criteria.
 * LOCAL symbols should always precede GLOBAL symbols.
 * @param list pointer to the symbol array
 * @param m left element
 * @param n right element
 */
static void quicksort(struct sh_symbol *list, long m, long n) {
  long key,i,j,k;
  if(m < n) {
    k = (m + n) / 2;
    swap(&list[m], &list[k]);
    key = (long)(list[m].start);
    i = m+1;
    j = n;
    while(i <= j) {
      /* Skip elements on the left while < key */
      while(i<=n && (((long)(list[i].start) < key)
                     || ((long)(list[i].start) == key
                         && !(list[i].binding==SB_GLOBAL
                              && list[m].binding==SB_LOCAL)))) i++;

      /* Skip elements on the right while > key */
      while(j>=m && (((long)(list[j].start) > key)
                     || ((long)list[j].start == key
                         && list[j].binding==SB_GLOBAL
                         && list[m].binding==SB_LOCAL))) j--;
      
      /* Swap two elements from left and right */
      if(i < j) swap(&list[i], &list[j]);
    }
    swap(&list[m], &list[j]);

    /* Recursively sort left and right part */
    quicksort(list, m, j-1);
    quicksort(list, j+1 ,n);
  }
}


/**
 * Tries to open the debug version of the library at the given path.
 * @param path Path to the library
 * @return file descriptor on success, -1 otherwise
 */
static long get_debug_version(const char *path) {

  struct r_search_path_elem *search_paths;
  char *found_path;
  
  search_paths = decompose_path(LIB_DBG_PATHS, 0, "debug paths");

  if(search_paths == 0) return -1;

  /* Resolve symbolic link of library to get full name */
  char *name = sl_malloc(MAX_LIB_NAME);
  long len = sl_readlink(path, name, MAX_LIB_NAME);
  if (len == -1) {
    sl_free(name, MAX_LIB_NAME);
    return -1;
  }

  return try_open_path(name, len, search_paths, &found_path);
}

void add_vdso(Elf32_Ehdr *sysinfo_ehdr) {

  /* Get elf headers */
  Elf32_Ehdr *ehdr = sysinfo_ehdr;
  Elf32_Shdr *shdr = (Elf32_Shdr *)((uint32_t)sysinfo_ehdr + ehdr->e_shoff);
  long shdr_size = ehdr->e_shnum;

  /* Allocate memory for dso struct */
  struct dso_chain *dso = sl_calloc(sizeof(struct dso_chain),1);
  if ((long) dso == -1) {
    sl_printf("Error add_vdso: sl_calloc failed.\n");
    sl_exit(1);
  }
  
  /* Symbol- and string table */
  Elf32_Shdr *symtab = 0;
  char *strtab = 0;

  dso->stripped = 1;
  
  /* Iterate over section headers */
  long i;
  for(i = 0; i < shdr_size; ++i) {
	
    /* Normal symbol table */
    if (shdr[i].sh_type == SHT_SYMTAB) {
      /* Get symbol table and associated string table */
      symtab = &shdr[i];
      unsigned long size = shdr[symtab->sh_link].sh_size;
      
      /* Because we only load the dynamic string table,
         we need to copy the big one */
      strtab = sl_malloc(size);
      if ((long) strtab == -1) {
        sl_printf("Error add_dso: malloc failed.\n");
        sl_exit(1);
      }
      
      sl_memcpy(strtab, (const void *)(((uint32_t)sysinfo_ehdr) + shdr[symtab->sh_link].sh_offset), size);

      /* Shared object is not stripped */
      dso->stripped = 0;
      
      continue;
    }

    /* Dynamic symbol table */
    if (shdr[i].sh_type == SHT_DYNSYM) {
      /* We prefer the static one */
      if (symtab == 0)
        symtab = &shdr[i];
    }

    if (shdr[i].sh_type == SHT_STRTAB) {
      if (strtab == 0)
        strtab = (char*)((uint32_t)sysinfo_ehdr + shdr[i].sh_offset);
    }
    
    /* Get addresses of important sections  */
    char *sh_strtab = (char *)sysinfo_ehdr + shdr[ehdr->e_shstrndx].sh_offset;
    char *name = sh_strtab + shdr[i].sh_name;
        
    if (sl_strncmp(name, ".text", 6) == 0) {
      dso->text = (char *)shdr[i].sh_addr;
      dso->text_size = shdr[i].sh_size;
      dso->text += (long)sysinfo_ehdr;
    }
    if (sl_strncmp(name, ".init", 6) == 0) {
      dso->init = (char *)shdr[i].sh_addr;
      dso->init_size = shdr[i].sh_size;
      dso->init += (long)sysinfo_ehdr;
    }
    if (sl_strncmp(name, ".fini", 6) == 0) {
      dso->fini = (char *)shdr[i].sh_addr;
      dso->fini_size = shdr[i].sh_size;
      dso->fini += (long)sysinfo_ehdr;
    }
    
  }
  
  /* TODO: use macros. */
  /* Initialize DSO */
  dso->name = "vdso";
  dso->baseaddr = (char*)sysinfo_ehdr;
  dso->endaddr = (char*)((uint32_t)sysinfo_ehdr + PSIZE-1);
  dso->exec = 0;

  /* Now we fill in all symbols defined in the shared object */
  struct sh_symbol *syms = 0;
  unsigned long sym_size = sizeof(Elf32_Sym);
  unsigned long size = symtab->sh_size;
  Elf32_Sym *sym = (Elf32_Sym *)((uint32_t)sysinfo_ehdr + symtab->sh_offset);
  
  /* Count the symbols */
  long n = 0;
  long num_isyms = 0;
  long nsyms = size/sym_size;
  for (i=0; i<nsyms; ++i) {
    /* We are only interested in function symbols */
    if ((ELF32_ST_TYPE(sym[i].st_info) == STT_FUNC
         || ELF32_ST_TYPE(sym[i].st_info) == STT_GNU_IFUNC)) {
      ++n;

      if (sym->st_value == 0) {
        /* Imported symbol */
        ++num_isyms;
      }
    }
  }

  sym++;
  size -=sym_size;

  if (num_isyms) {
    /* Allocate memory for imported symbols */
    dso->isymbols = sl_calloc(num_isyms*sizeof(struct sh_symbol), 1);
  }

  if (n) {
    /* Allocate memory for symbols */
    syms = sl_calloc((n+5)*sizeof(struct sh_symbol), 1);
    if ((long)syms == -1) {
      sl_printf("Error add_dso: malloc failed\n");
      sl_exit(1);
    }
    
    /* Iterate over symbols */
    long isymnum = 0;
    for (i=0; size > 0; sym++, size-=sym_size) {
      if (((ELF32_ST_TYPE(sym->st_info) != STT_FUNC)
           && (ELF32_ST_TYPE(sym->st_info) != STT_GNU_IFUNC))) {
        continue;
      }

      if (sym->st_value == 0) {
        /* Imported symbol, add to list */
        dso->isymbols[isymnum++].name = (char *)strtab + sym->st_name;
        continue;
      }

      /* Get binding, we only differentiate between local and global */
      if ((ELF32_ST_BIND(sym->st_info) == STB_GLOBAL
           || ELF32_ST_BIND(sym->st_info) == STB_WEAK)
          && (ELF32_ST_VISIBILITY(sym->st_other) != STV_HIDDEN))
        syms[i].binding = SB_GLOBAL;
      else
        syms[i].binding = SB_LOCAL;

	  /* Note: sym->st_value is according to the assumed base load address 0xffffe000 i.e. the vdso's LOAD segment VirtAddr */
      syms[i].start = (unsigned char *)((uint32_t)sysinfo_ehdr + (sym->st_value & (~PMASK)));
      syms[i].name = (char *)strtab + sym->st_name;
      syms[i].size = sym->st_size;
      
	  syms[i].dso = dso;

      ++i;
    }
    /* Sort symbols */
    /* There may be multiple symbols with same address but different binding,
       so we have to prefer SB_LOCAL over SB_GLOBAL in quicksort */
    quicksort(syms, 0, i-1);

    /*
     * The symbols are organized in pages. Each page has a start address and
     * size and contains all symbols defined in the corresponding memory page
     * of the DSO. The page belonging to a symbol is obtained by setting the
     * last PSIZE bits of the symbol address to zero (using the PMASK macro).
     */
    
    /* Get lowest possible page addr */
    long addr_low = (long)dso->baseaddr & PMASK;

    /* Allocate space for pages array */    
    long nr_pages = ((dso->endaddr-dso->baseaddr)/PSIZE)+1;
    struct code_page *pages =
      sl_calloc((nr_pages)*sizeof(struct code_page), 1);

    long nr_symbols = 0, index = 0;
    long last_addr = addr_low, curr_addr = last_addr;
    
    long j = 0;
    
    /* TODO: check loop condition and what i holds in the end, why is nr of symbols = nr_symbols + 1? */
    
    /* Iterate over symbols and allocate space for symbol arrays in pages */
    for (j = 0; j < i-1; ++j) {

      /* If the size is zero we calculate it as the length to the next
         symbol because the library may be stripped. */
      if (syms[j].size == 0 && syms[j].start != 0) {
        syms[j].size = syms[j+1].start - syms[j].start - 1;
      }

      /* Get address of page */
      curr_addr = (long)syms[j].start & PMASK;

      if (curr_addr != last_addr) {
        /* Page changed, so allocate a new one */
        pages[index].addr = last_addr;
        pages[index].syms =
        sl_calloc((nr_symbols+1)*sizeof(struct sh_symbol *), 1);
        pages[index].nr_symbols = nr_symbols;

        /* Get page number */
        index = (curr_addr - addr_low) >> PSHIFT;
        
        last_addr = curr_addr;
        nr_symbols = 0;
      }
      
      nr_symbols++;
    }

    /* Last page */
    pages[index].addr = curr_addr;
    pages[index].syms = sl_calloc((nr_symbols+1)*sizeof(struct sh_symbol *),1);
    pages[index].nr_symbols = nr_symbols;
    dso->nr_pages = nr_pages;
          
    /* Now add symbols to pages */
    last_addr = addr_low;
    index = 0;
    long sym_index = 0;
    /* Note: it has to be a <= as we also need the bounds, i = nr of symbols so i-1 is the index to the last one */
    for (j=0; j<=i-1; ++j) {
      /* Get address of page */
      long curr_addr = (long)syms[j].start & PMASK;
      
      if (curr_addr != last_addr) {
        /* Page changed, so get index of next one */
        index = (curr_addr - addr_low) >> PSHIFT;
        sym_index = 0;
        last_addr = curr_addr;
      }

      /* Add symbol to page */
      pages[index].syms[sym_index] = &syms[j];
      sym_index++;
    }
    
    /* Add pages to DSO */
    dso->pages = pages;

    /* Set pages and syms to readonly */
    PROT_DATA(dso->pages, dso->nr_pages*sizeof(struct code_page));
    PROT_DATA(syms, n*sizeof(struct sh_symbol));
  }

  /* Add DSO to end of chain (first element is reserved for secuLoader) */
  if (!dso_chain) {
    /* secuLoader */
    dso_chain = dso;
  } else {
    /* Insert after first element and protect data afterwards */
    dso->next = dso_chain->next;
    dso->prev = dso_chain;

    if(dso_chain->next) {
      UNPROT_DATA(dso_chain->next, sizeof(struct dso_chain));
      dso_chain->next->prev = dso;
      PROT_DATA(dso_chain->next, sizeof(struct dso_chain));
    }

    UNPROT_DATA(dso_chain, sizeof(struct dso_chain));
    dso_chain->next = dso;
    PROT_DATA(dso_chain, sizeof(struct dso_chain));
  }
    
  /* Set DSO to readonly */
  PROT_DATA(dso, sizeof(struct dso_chain));

  /* Define vdso so we can reference this struct directly. */
  vdso = dso;
  
}

void add_dso(dso *so, char *filebase) {
  long fd = -1;
  struct kernel_stat dbg_file_info;
  int is_stripped = 1;
  
	//if(so) sl_printf("add_dso load dso: %s\n", so->name);
	//else sl_printf("add_dso load dso, filebase: %s\n", filebase);

  /* If so = 0, filebase points to the secuLoader itself */
  if (so) {
    /* We first try to get a non-stripped version of the object */
    long fd = get_debug_version(so->path);

    if (fd != -1) {
      /* Get file size */
      if(sl_fstat(fd, &dbg_file_info) == -1) {
        sl_printf("Error add_dso: fstat failed\n");
        sl_exit(1);
      }
      
      /* Map file in memory */
      filebase = (char *)sl_mmap(0, dbg_file_info.st_size, PROT_READ,
                                 MAP_PRIVATE, fd, 0);
      is_stripped = 0;
      //sl_printf("add_dso: got DEBUG symbols for %s.\n", so->path);
    }
  }

  /* Get elf headers */
  Elf32_Ehdr *ehdr = (Elf32_Ehdr *)filebase;
  Elf32_Shdr *shdr = (Elf32_Shdr *)(filebase + ehdr->e_shoff);
  long shdr_size = ehdr->e_shnum;

  /* Allocate memory for dso struct */
  struct dso_chain *dso = sl_calloc(sizeof(struct dso_chain),1);
  if ((long) dso == -1) {
    sl_printf("Error add_dso: sl_calloc failed.\n");
    sl_exit(1);
  }
  
  /* Symbol- and string table */
  Elf32_Shdr *symtab = 0;
  char *strtab = 0;

  if (so) {
    strtab = (char *)so->string_table;
    so->dso = dso;
  }

  dso->stripped = is_stripped;
  
  /* Iterate over section headers */
  long i;
  for(i = 0; i < shdr_size; ++i) {
    /* Normal symbol table */
    if (shdr[i].sh_type == SHT_SYMTAB) {
      /* Get symbol table and associated string table */
      symtab = &shdr[i];
      unsigned long size = shdr[symtab->sh_link].sh_size;
        
      /* Because we only load the dynamic string table,
         we need to copy the big one */
      strtab = sl_malloc(size);
      if ((long) strtab == -1) {
        sl_printf("Error add_dso: malloc failed.\n");
        sl_exit(1);
      }
      sl_memcpy(strtab, filebase+shdr[symtab->sh_link].sh_offset, size);

      /* Save to unmap later */
      if (so) {
        so->st_strtab = strtab;
        so->st_strtab_size = size;
      }

      /* Shared object is not stripped */
      dso->stripped = 0;
      
      continue;
    }

    /* Dynamic symbol table */
    if (shdr[i].sh_type == SHT_DYNSYM) {
      /* We prefer the static one */
      if (symtab == 0)
        symtab = &shdr[i];
    }

    /* Get addresses of important sections  */
    char *sh_strtab = (char *)filebase + shdr[ehdr->e_shstrndx].sh_offset;
    char *name = sh_strtab + shdr[i].sh_name;
    
    if (sl_strncmp(name, ".text", 6) == 0) {
      dso->text = (char *)shdr[i].sh_addr;
      dso->text_size = shdr[i].sh_size;
      if (so)
        dso->text += (long)so->base_addr;
    }
    if (sl_strncmp(name, ".init", 6) == 0) {
      dso->init = (char *)shdr[i].sh_addr;
      dso->init_size = shdr[i].sh_size;
      if (so)
        dso->init += (long)so->base_addr;
    }
    if (sl_strncmp(name, ".fini", 6) == 0) {
      dso->fini = (char *)shdr[i].sh_addr;
      dso->fini_size = shdr[i].sh_size;
      if (so)
        dso->fini += (long)so->base_addr;
    }

    if (sl_strncmp(name, ".rodata", 8) == 0) {
      dso->rodata = (char *)shdr[i].sh_addr;
      dso->rodata_size = shdr[i].sh_size;
      if (so)
        dso->rodata += (long)so->base_addr;
    }
    if (sl_strncmp(name, ".rel.dyn", 9) == 0) {
      dso->reldyn = (char *)shdr[i].sh_addr;
      dso->reldyn_size = shdr[i].sh_size;
      if (so)
        dso->reldyn += (long)so->base_addr;
    }
    if (sl_strncmp(name, ".rel.plt", 9) == 0) {
      dso->relplt = (char *)shdr[i].sh_addr;
      dso->relplt_size = shdr[i].sh_size;
      if (so)
        dso->relplt += (long)so->base_addr;
    }
                
    /* Get base and end address of secuLoader */
    if (!so && sl_strncmp(name, ".text", 6) == 0)
      dso->baseaddr = (char *)shdr[i].sh_addr;
    if (!so && sl_strncmp(name, ".data", 6) == 0)
      dso->endaddr = (char *)shdr[i].sh_addr + shdr[i].sh_size;  
  }
  
  if (so != 0) {
    /* Initialize DSO */
    dso->name = so->name;
    dso->baseaddr = so->text_addr;
    dso->endaddr = so->end_addr;
    dso->exec = (so->type==ET_EXEC);
    dso->got = so->got;
    dso->got_size = so->got_size;
    dso->gotplt = so->gotplt;
    dso->gotplt_size = so->gotplt_size;
    dso->plt = so->plt;
    dso->plt_size = so->plt_size;
    dso->entry = so->entry;
  } else {
    /* SecuLoader */
    dso->name = "secuLoader";
    dso->exec = 1;
  }
  
  //sl_printf("baseaddr %p, endaddr %p\n", dso->baseaddr, dso->endaddr);
  
  /* TODO: protect hashtable before entering code cache */
  dso->callbacks = sl_calloc(CALLBACK_MAPPINGTABLE_SIZE, 1);
  dso->nr_callbacks = 0;
    
#if defined(SECURITY_METRICS_AND_STATS)
  dso->icalls = sl_calloc(ICF_TABLES_MAX_ENTRIES, sizeof(char*));
  dso->ijmps = sl_calloc(ICF_TABLES_MAX_ENTRIES, sizeof(char*));
  dso->retinstrs = sl_calloc(ICF_TABLES_MAX_ENTRIES, sizeof(char*));
  dso->nr_icalls = 0;
  dso->nr_ijmps = 0;
  dso->nr_retinstrs = 0;
  dso->callbacks_stats = sl_calloc(CALLBACK_MAPPINGTABLE_SIZE, 1);
#endif

#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)
  dso->cft_valid_call_targets = sl_calloc(CFT_MAPPINGTABLE_SIZE, 1);
  dso->cft_valid_jmp_targets = sl_calloc(CFT_MAPPINGTABLE_SIZE, 1);
#endif

/* Allocate memory for CFT check optimazations. */
#if defined(VERIFY_CFTX_ENABLE_CACHE)
  dso->cft_cache_pool = sl_calloc(CFT_CACHE_SIZE, 1);
  dso->cft_cache_pool_next = dso->cft_cache_pool;
#endif

#if defined(PRINT_CFTX_CHECK_TRANSFER_STATS)
  dso->nr_check_transfers_total = 0;
  dso->nr_check_transfers_slowpath = 0;
#endif

#if defined(PRINT_CFTX_FASTLOOKUP_STATS)
  dso->nr_fastlookups = 0;
#endif

#if defined(PRINT_CFTX_CACHE_STATS)
  dso->nr_cache_hits = 0;
#endif

  /* Now we fill in all symbols defined in the shared object */
  struct sh_symbol *syms = 0;
  unsigned long sym_size = sizeof(Elf32_Sym);
  unsigned long size = symtab->sh_size;
  Elf32_Sym *sym = (Elf32_Sym *)(filebase + symtab->sh_offset);
  
  /* Count the symbols */
  long n = 0;
  long num_isyms = 0;
  long nsyms = size/sym_size;
  for (i=0; i<nsyms; ++i) {
    /* We are only interested in function symbols */
    if ((ELF32_ST_TYPE(sym[i].st_info) == STT_FUNC
         || ELF32_ST_TYPE(sym[i].st_info) == STT_GNU_IFUNC
         || ELF32_ST_TYPE(sym[i].st_info) == STT_NOTYPE)) { /* NOTYPE added as this might also be a function. */
      ++n;

      /* 32bit ls seems to have imported free and malloc symbols defined 
       * but with value != 0. The value actually points to the .plt entry of malloc/free. */
      /* readelf -s /home/antonio/binaries/ls | grep malloc
			113: 08049820     0 FUNC    GLOBAL DEFAULT  UND malloc@GLIBC_2.0 (3)*/
	  /* was sym->st_value == 0, added || sym->st_shndx == 0 for special cases like ls */
	  /* Note: adding || sym->st_shndx == 0 will break the exception test case and in general c++ exception handling. */
      if (sym->st_value == 0 || ((sym->st_value != 0) && (sym->st_shndx == 0))) {
        /* Imported symbol */
        ++num_isyms;
      }
    }
  }

  /* TODO: why? */
  sym++;
  size -=sym_size;
  //sl_printf("add_dso nr sym: %d, n: %d, size: %x, sym_size: %x\n", sym, n, size, sym_size);

  /* Add the vdso symbols to all dsos loaded */
  if(vdso && (vdso->nr_pages > 0)) {
	  num_isyms += vdso->pages[0].nr_symbols+1;
  }

  /* TODO: To be improved! */
  /* Import some additional symbols depending on dso name */
  if(!fbt_strncmp(dso->name, "libc.so.6", 9)) {
	  num_isyms += 3;
  }
  
  /* TODO: for the main executable/DSO we need to import _IO_printf, see below */
  //if(!fbt_strncmp(dso->name, "main", 5)) {
	  num_isyms += 1;
	  num_isyms += 1;
	  num_isyms += 1;
	  num_isyms += 1;
  //}
  
  if (num_isyms) {
    /* Allocate memory for imported symbols */
    dso->isymbols = sl_calloc(num_isyms*sizeof(struct sh_symbol), 1);
  }

  long isymnum = 0;
  
  /* Add vdso symbols as imported symbols */
  if(vdso && (vdso->nr_pages > 0)) {
	for (i=0; i<(vdso->pages[0].nr_symbols+1); i++) {
     /* Imported symbol, add to list */
     dso->isymbols[isymnum++].name = vdso->pages[0].syms[i]->name;
	}
  }
  
  if(!fbt_strncmp(dso->name, "libc.so.6", 9)) {
     dso->isymbols[isymnum++].name = "main";
     dso->isymbols[isymnum++].name = "__libc_csu_init";
     dso->isymbols[isymnum++].name = "cleanup";
  }
  
  /* TODO: for the main executable/DSO we need to import _IO_printf, to be improved */
  //if(!fbt_strncmp(dso->name, "main", 5)) {
     dso->isymbols[isymnum++].name = "_IO_printf";
     dso->isymbols[isymnum++].name = "_IO_puts";
     dso->isymbols[isymnum++].name = "__libc_malloc";
     dso->isymbols[isymnum++].name = "malloc";
     /* "malloc" is imported instead of "__libc_malloc", libc has multiple exported symbols for the same address
      * which will cause a miscompare of the symbol name during cft check, TODO: fix! */
	 
  //}
  
  if (n) {
	/* TODO: why n+5? */
    /* Allocate memory for symbols */
    syms = sl_calloc((n+5)*sizeof(struct sh_symbol), 1);
    if ((long)syms == -1) {
      sl_printf("Error add_dso: malloc failed\n");
      sl_exit(1);
    }
    
    /* TODO: fix memory corruption, sl_calloc is deduced from sh_symbol size but
     * the iteration below -> i depends on sym_size which is deduced from sizeof(Elf32_Sym). */
    /* Iterate over symbols */
    for (i=0; size > 0; sym++, size-=sym_size) {
      if (((ELF32_ST_TYPE(sym->st_info) != STT_FUNC)
           && (ELF32_ST_TYPE(sym->st_info) != STT_GNU_IFUNC)
           && (ELF32_ST_TYPE(sym->st_info) != STT_NOTYPE))) {
        continue;
      }

      /* 32bit ls seems to have imported free and malloc symbols defined 
       * but with value != 0. The value actually points to the .plt entry of malloc/free. */
      /* readelf -s /home/antonio/binaries/ls | grep malloc
			113: 08049820     0 FUNC    GLOBAL DEFAULT  UND malloc@GLIBC_2.0 (3)*/
	  /* was sym->st_value == 0, added || sym->st_shndx == 0 for special cases like ls */
	  /* Note: adding || sym->st_shndx == 0 will break the exception test case and in general c++ exception handling. */
      if (sym->st_value == 0) {
        /* Imported symbol, add to list */
        dso->isymbols[isymnum++].name = (char *)strtab + sym->st_name;
        continue;
      } else if ((sym->st_value != 0) && (sym->st_shndx == 0)) { /* This is an imported and exported symbol! */
		dso->isymbols[isymnum++].name = (char *)strtab + sym->st_name;
	  }

      /* Get binding, we only differentiate between local and global */
      if ((ELF32_ST_BIND(sym->st_info) == STB_GLOBAL
           || ELF32_ST_BIND(sym->st_info) == STB_WEAK)
          && (ELF32_ST_VISIBILITY(sym->st_other) != STV_HIDDEN))
        syms[i].binding = SB_GLOBAL;
      else
        syms[i].binding = SB_LOCAL;

      /* Get base address */
      if (so != 0 && sym->st_value) {
        syms[i].start = (unsigned char *)(so->base_addr + sym->st_value);
      } else {
        syms[i].start = (unsigned char *)sym->st_value;
      }

	  /* TODO: we are calling library code directly here: needs to run in TRuE! */
	  /* TODO2: the got is not yet initialized -> init_got, i.e. calls over the plt will land into unmapped memory -> segfaults */
	  if (ELF32_ST_TYPE(sym->st_info) == STT_GNU_IFUNC && sym->st_value) {
		 //sl_printf("add_dso: a STT_GNU_IFUNC is resolved: %x, %s\n", syms[i].start, (char*)((char *)strtab + sym->st_name));
         syms[i].start = (void*)((char *(*) (void))(syms[i].start))();
         //sl_printf("add_dso: a STT_GNU_IFUNC is resolved: %x, %s\n", syms[i].start, (char*)((char *)strtab + sym->st_name));
      }

      syms[i].name = (char *)strtab + sym->st_name;
      syms[i].size = sym->st_size;
      
	  if(!fbt_strncmp(dso->name, "libc.so.6", 9) && !fbt_strncmp(syms[i].name, "__libc_init_first", 17)) {
		 syms[i].size += 128; /* TODO: this is a hack, symbol size is wrong for __libc_init_first in libc. */
	  }
      
      if(!fbt_strncmp(dso->name, "libc.so.6", 9) && !fbt_strncmp(syms[i].name, "__longjmp_chk", 13)) {
		 syms[i].size += 134; /* TODO: this is a hack, symbol size is wrong for __longjmp_chk in libc. */
	  }
	  
      //sl_printf("add_dso: dso %s, symbol, %s, sym address: %x, i = %d, index = %d\n", dso->name, syms[i].name, syms[i].start, i, sym->st_shndx);
	  
	  syms[i].dso = dso;
	  
      ++i;
    }
    //sl_printf("add_dso: iterated over symbols, %d\n",i);
    dso->nr_isymbols = isymnum;
    
    /* Sort symbols */
    /* There may be multiple symbols with same address but different binding,
       so we have to prefer SB_LOCAL over SB_GLOBAL in quicksort */
    quicksort(syms, 0, i-1);

    /*
     * The symbols are organized in pages. Each page has a start address and
     * size and contains all symbols defined in the corresponding memory page
     * of the DSO. The page belonging to a symbol is obtained by setting the
     * last PSIZE bits of the symbol address to zero (using the PMASK macro).
     */
    
    /* Get lowest possible page addr */
    long addr_low = (long)dso->baseaddr & PMASK;

    /* Allocate space for pages array */    
    long nr_pages = ((dso->endaddr-dso->baseaddr)/PSIZE)+1;
    struct code_page *pages =
      sl_calloc((nr_pages)*sizeof(struct code_page), 1);

    long nr_symbols = 0, index = 0;
    long last_addr = addr_low, curr_addr = last_addr;
    
    //sl_printf("add_dso: iterate over symbols, addr_low %x, nr_pages %d\n", addr_low, nr_pages);

    long j = 0;
    /* Iterate over symbols and allocate space for symbol arrays in pages */
    for (j = 0; j < i-1; ++j) {

      /* If the size is zero we calculate it as the length to the next
         symbol because the library may be stripped. */
      if (syms[j].size == 0 && syms[j].start != 0) {
        syms[j].size = syms[j+1].start - syms[j].start - 1;
      }

      /* Get address of page */
      curr_addr = (long)syms[j].start & PMASK;

      if (curr_addr != last_addr) {
        /* Page changed, so allocate a new one */
        pages[index].addr = last_addr;
        pages[index].syms =
          sl_calloc((nr_symbols+1)*sizeof(struct sh_symbol *), 1);
        pages[index].nr_symbols = nr_symbols;

        /* Get page number */
        index = (curr_addr - addr_low) >> PSHIFT;
        
        last_addr = curr_addr;
        nr_symbols = 0;
      }
      
      nr_symbols++;
    }
    //sl_printf("add_dso: nr_symbols, %d\n",nr_symbols);

    /* Last page */
    pages[index].addr = curr_addr;
    pages[index].syms = sl_calloc((nr_symbols+1)*sizeof(struct sh_symbol *),1);
    
    /* special case for DSO with just one symbol 
     * TODO: the entire add_dso needs to be reviewed and reimplemented 
     * What if DSO has no symbols? Do we really get all exported symbols? */
    if(i == 1) {
		pages[index].nr_symbols = i;
	} else {
		pages[index].nr_symbols = nr_symbols;
	}
    dso->nr_pages = nr_pages;

    /* Now add symbols to pages */
    last_addr = addr_low;
    index = 0;
    long sym_index = 0;
    /* TODO: for special case i == 1 this will not work */
    for (j=0; j<i-1; ++j) {
      /* Get address of page */
      long curr_addr = (long)syms[j].start & PMASK;
      
      if (curr_addr != last_addr) {
        /* Page changed, so get index of next one */
        index = (curr_addr - addr_low) >> PSHIFT;
        sym_index = 0;
        last_addr = curr_addr;
      } 

      /* Add symbol to page */
      pages[index].syms[sym_index] = &syms[j];
      sym_index++;
    }
    if(i == 1) {
		pages[index].syms[i-1] = &syms[i-1];
	}
    
#ifdef D_LIBDETOX
    /* Debug ouput */
    sl_printf("\nCreating pages for: %s\n", dso->name);
    for (j=0; j<dso->nr_pages; ++j) {
      if (!pages[j].addr)
        continue;
      sl_printf("Page %p contains:\n", pages[j].addr, pages[j].nr_symbols);
      int k = 0;
      for (k=0; k<pages[j].nr_symbols; ++k) {
        sl_printf("%p %s\n", pages[j].syms[k]->start, pages[j].syms[k]->name);
      }
      sl_printf("\n");
    }
#endif
    
    /* Add pages to DSO */
    dso->pages = pages;

    /* Set pages and syms to readonly */
    PROT_DATA(dso->pages, dso->nr_pages*sizeof(struct code_page));
    PROT_DATA(syms, n*sizeof(struct sh_symbol));
  }

  /* Add DSO to end of chain (first element is reserved for secuLoader) */
  if (!dso_chain) {
    /* secuLoader */
    dso_chain = dso;
  } else {
    /* Insert after first element and protect data afterwards */
    dso->next = dso_chain->next;
    dso->prev = dso_chain;

    if(dso_chain->next) {
      UNPROT_DATA(dso_chain->next, sizeof(struct dso_chain));
      dso_chain->next->prev = dso;
      PROT_DATA(dso_chain->next, sizeof(struct dso_chain));
    }

    UNPROT_DATA(dso_chain, sizeof(struct dso_chain));
    dso_chain->next = dso;
    PROT_DATA(dso_chain, sizeof(struct dso_chain));
  }
    
  /* Set DSO to readonly */
  PROT_DATA(dso, sizeof(struct dso_chain));
  
  /* Unmap non-stripped version of file */
  if (fd != -1)
    sl_munmap(filebase, dbg_file_info.st_size);
}


void remove_dso(dso *so) {
  /* Free stringtable */
  if (so->st_strtab_size)
    sl_free(so->st_strtab, so->st_strtab_size);

  /* Remove dso from chain */
  struct dso_chain *dso = so->dso;
  /* We never remove the first object in the chain, so we dont have to check
     for prev != 0 */
  UNPROT_DATA(dso->prev, sizeof(struct dso_chain));
  dso->prev->next = dso->next;
  PROT_DATA(dso->prev, sizeof(struct dso_chain));
  if (dso->next) {
    UNPROT_DATA(dso->next, sizeof(struct dso_chain));
    dso->next->prev = dso->prev;
    PROT_DATA(dso->next, sizeof(struct dso_chain));
  }

  /* Free all pages and symbols */
  if (dso->pages) {
    long i;
    for (i = 0; i < dso->nr_pages; ++i) {
      sl_free(dso->pages[i].syms,
              dso->pages[i].nr_symbols * sizeof(struct sh_symbol*));
    }
    sl_free(&dso->pages, dso->nr_pages*sizeof(struct code_page));
  }

  /* Free imported symbols */
  if (dso->isymbols) {
    sl_free(dso->isymbols, dso->nr_isymbols*sizeof(struct sh_symbol));
  }

  /* Free struct */
  sl_free(dso, sizeof(struct dso_chain));
}

#endif /* VERIFY_CFTX */

/*
  Example plt entries:

  Executable:
  08048364  <strlen@plt>:
   8048364  :ff 25 08 a0 04 08    jmp    *0x804a008
   804836a  :68 10 00 00 00       push   $0x10
   804836f  :e9 c0 ff ff ff       jmp    8048334 <_init+0x30>

  Shared object:
  00093f50 <test@plt>:
     93f50  :ff a3 10 00 00 00    jmp    *0x10(%ebx)
     93f56  :68 08 00 00 00       push   $0x8
     93f5b  :e9 d0 ff ff ff       jmp    93f30 <_init+0x18>
*/
/* Note: this resolve function works only for plt */
unsigned long sl_resolve_plt_call(unsigned long addr, struct dso_chain *dso_objects) {
  dso *so = 0;
  
  for (so = so_chain; so != 0; so = so->next) {
    /* Check if this is a plt call */
    if (so->plt_size && PTR_IN_REGION(addr, so->plt, so->plt_size-1)) {
		
	  /* This is a special case, if we try to resolve the first plt entry to the runtime linker
	   * lazy resolution function we should not inline it. This might happen in certain special
	   * cases. E.g. in g++ exception handling */
	  if (addr == (unsigned long)so->plt) return addr;
	  
      /* Address to got entry (jmp instruction is 2 bytes long) */
      unsigned long got_addr = *((unsigned long *)(addr + 0x2));

      /* In a shared object we get only an offset, so add this to got addr */
      if (so->type != ET_EXEC)
        got_addr = (unsigned long)so->gotplt + got_addr;

      /* Get got entry and address of instruction after jump (+ 6 bytes) */
      unsigned long got_entry =  *((unsigned long *)(got_addr));
      unsigned long next_addr = addr+0x6;

      /* Check if we already resolved this plt entry (this is the case if the
         got_entry does not point to the next instruction) */
      if (got_entry == next_addr) {
        /* We need to resolve this entry, so get the offset in the relocation
           table (argument of push instruction) and resolve it */
        unsigned long rel_offset = *((unsigned long *)(addr + 0x7));
        return resolve_plt(so, rel_offset, dso_objects);
      } else {
        return got_entry;
      }
    }
  }

  /* Not a plt call, return original addr */
  return addr;
}
