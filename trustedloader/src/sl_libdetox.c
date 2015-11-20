/**
 * @file sl_libdetox.h
 * Communication with the libdetox framework.
 * Implements functions to load the libdetox library and supplies all 
 * needed information for the control transfer checks.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
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


#ifdef ATTACH_LIBDETOX

#include "sl_libdetox.h"
#include "sl_datatypes.h"
#include "sl_macros.h"
#include "sl_libc.h"
#include "sl_load.h"
#include "sl_sym_lookup.h"
#include "sl_so_chain.h"
#include "fbt_libc.h"

extern dso *so_chain;

#ifdef VERIFY_CFTX

/* Chain of loaded objects for control flow transfer checks in libdetox */
struct dso_chain *dso_chain = 0;


/**
 * Initializes the DSO of the secuLoader itself and adds it to the 
 * libdetox dso_chain for control flow transfer checks.
 */
static void init_loader_dso() {
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

  /* Unmap file */
  sl_munmap(file_map, file_info.st_size);
}

#endif /* VERIFY_CFTX */


dso *load_libdetox() {
#ifdef D_LIBDETOX
  sl_printf("Loading libdetox.\n");
#endif
  
  dso *so_found = 0;
  Elf32_Sym *sym_found = 0;

  /* Open libdetox */
  long fd = sl_open(LIBDETOX_LIB, 0);
  if(fd == -1) {
    sl_printf("Error: could not open libdetox, check the path settings.\n");
    sl_exit(1);
  }

  /* Load libdetox */
  dso *so = load_elf(0, "libdetox", LIBDETOX_LIB, fd, 0);

  /* Resolve needed functions */
  libdetox_init = resolve_symbol(so, "fbt_init", 0, 1, 1, &sym_found,
                                 &so_found);
  libdetox_start_transaction = resolve_symbol(so, "fbt_start_transaction", 0,
                                              1, 1, &sym_found, &so_found);

  libdetox_commit_transaction = resolve_symbol(so, "fbt_commit_transaction", 0,
                                               1, 1, &sym_found, &so_found);

#ifdef VERIFY_CFTX
  /* Initialize the loader dso if libdetox checks the control flow transfers */
  init_loader_dso();
#endif
  
  return so;
}


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

  x->start = y->start;
  x->size = y->size;
  x->name = y->name;
  x->binding = y->binding;

  y->start = temp.start;
  y->size = temp.size;
  y->name = temp.name;
  y->binding = temp.binding;
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

  /* Resolve symbolic link of library to get full name */
  char *name = sl_malloc(MAX_LIB_NAME);
  long len = sl_readlink(path, name, MAX_LIB_NAME);
  if (len == -1) {
    sl_free(name, MAX_LIB_NAME);
    return -1;
  }

  /* Assemble name of debug library */
  char *dpath = sl_malloc(MAX_PATH_LEN);
  sl_strncpy(dpath, LIB_DBG_PATHS, sl_strnlen(LIB_DBG_PATHS, MAX_PATH_LEN));
  sl_strncat(dpath, name, len);

  /* Try to open */
  long fd = sl_open(dpath, O_RDONLY);

  sl_free(name, MAX_LIB_NAME);
  sl_free(dpath, MAX_PATH_LEN);

  return fd;
}


void add_dso(dso *so, char *filebase) {
  long fd = -1;
  struct kernel_stat dbg_file_info;

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
  } else {
    /* SecuLoader */
    dso->name = "secuLoader";
    dso->exec = 1;
  }

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

      /* Get base address */
      if (so != 0 && sym->st_value) {
        syms[i].start = (unsigned char *)(so->base_addr + sym->st_value);
      } else {
        syms[i].start = (unsigned char *)sym->st_value;
      }

      syms[i].name = (char *)strtab + sym->st_name;
      syms[i].size = sym->st_size;

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
unsigned long sl_resolve_plt_call(unsigned long addr) {
  dso *so = 0;
  
  for (so = so_chain; so != 0; so = so->next) {
    /* Check if this is a plt call */
    if (so->plt_size && PTR_IN_REGION(addr, so->plt, so->plt_size-1)) {
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
        return resolve_plt(so, rel_offset);
      } else {
        return got_entry;
      }
    }
  }

  /* Not a plt call, return original addr */
  return addr;
}

#endif /* ATTACH_LIBDETOX */
