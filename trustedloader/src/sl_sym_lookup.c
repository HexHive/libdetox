/**
 * @file sl_sym_lookup.c
 * Implements the symbol lookup functionality.
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

#include "sl_libc.h"
#include "sl_tls.h"
#include "sl_sym_lookup.h"
#include "sl_macros.h"
#include "sl_datatypes.h"
#include "sl_so_chain.h"
#include "sl_gscope.h"
#include "sl_libdetox.h"

/* Global chain of loaded objects */
extern dso *so_chain;

/* Global search scope */
extern dso *gscope;

extern void REG_ARGS(3) signal_error(long errcode, const char *objname,
                                     const char *occasion,
                                     const char *errstring);

/* Not yet supported methods */
void debug_state (void);
long __attribute__((regparm (1))) make_stack_executable (void **stack_endp);
void mcount (Elf32_Addr frompc, Elf32_Addr selfpc);
void rtld_di_serinfo ();

/* Global loader variables */
extern char **dl_argv;
extern long libc_enable_secure;
extern long libc_stack_end;
extern struct rtld_global *rtld_glob;
extern struct rtld_global_ro *rtld_glob_ro;


/**
 * Runtime trampoline for lazy symbol resolving.
 * This is called by the plt whenever a function
 * call to a shared object must be resolved.
 */
asm (".text\n"
     ".globl runtime_trampoline\n"
     ".type runtime_trampoline, @function\n"
     ".align 16\n"
     "runtime_trampoline:\n"
     "pushl %eax\n"
     "pushl %ecx\n"
     "pushl %edx\n"
     "call resolve\n"
     "popl %edx\n"
     "popl %ecx\n"
     "xchgl %eax, (%esp)\n"
     "ret $8\n");

/* Variables exported by secuLoader */
struct variable loader_variables[] = {
  {"_rtld_global", (void *)&rtld_glob, 1},
  {"_rtld_global_ro", (void *)&rtld_glob_ro, 1},
  {"__libc_stack_end", (void *)&libc_stack_end, 0},
  {"__libc_enable_secure", (void *)&libc_enable_secure, 0},
  {"_dl_argv", (void *)&dl_argv, 0},
  {0,0}};

/* Functions exported by secuLoader */
struct function loader_functions[] = {
  /* There are two versions of tls_get_addr with different calling conventions.
     The names only differ in the number of underscores. This one uses the
     normal calling convention, whereas the other one gets the parameter in a
     register. */
  {"___tls_get_addr", &tls_get_addr},
  {"__tls_get_addr", &tls_get_addr_2},
  {"_dl_allocate_tls", &allocate_tls},
  {"_dl_allocate_tls_init", &allocate_tls_init},
  {"_dl_deallocate_tls", &deallocate_tls},
  {"_dl_get_tls_static_info", &get_tls_static_info},
  {"_dl_tls_setup", &tls_setup},
  {"_dl_debug_state", &debug_state},
  {"_dl_make_stack_executable", &make_stack_executable},
  {"_dl_mcount", &mcount},
  {"_dl_rtld_di_serinfo", &rtld_di_serinfo},
  {"_dl_addr", &dl_addr},
  {"dl_iterate_phdr", &dl_iterate_phdr},

#if defined(ATTACH_LIBDETOX)
  /* This is for libdetox */
  {"sl_resolve_plt_call", &sl_resolve_plt_call},
#endif
  
  {0,0}};


/**
 * Returns the symbol defined at the given address in the DSO.
 * @param dso The dynamic shared object 
 * @param addr Address of the symbol
 * @return Elf32_Sym on success, NULL otherwise
 */
static Elf32_Sym * get_symbol_at(dso *so, const void *addr) {
  Elf32_Sym *sym_found = 0;

  /* Right now we only support the gnu_hash_table for get_symbol_at */
  if (!so->gnu_hash_table) {
    sl_printf("Error get_symbol_at: no gnu hash table\n");
    sl_exit(1);
  }

  /* Get hash table data (see gnu_hash_lookup) */
  Elf32_Word nbuckets = so->gnu_hash_table[0];
  Elf32_Word bias = so->gnu_hash_table[1];
  Elf32_Word bloom_nwords = so->gnu_hash_table[2];
  Elf32_Word *bucket = &so->gnu_hash_table[4 + bloom_nwords];
  Elf32_Word *chain = &bucket[nbuckets] - bias;

  /* Get all symbols defined in this DSO by iterating over bucket entries 
     and following the chain for each entry (see gnu_hash_lookup) */
  long i = 0; 
  for (i = 0; i < nbuckets; ++i) {
    Elf32_Word bucket_entry = bucket[i];

    if (bucket_entry != 0) {
      /* Get symbol */
      Elf32_Word *hasharr = &chain[bucket_entry];
      Elf32_Sym *sym = &so->symbol_table[bucket_entry];
      
      do {
        /* Get nearest lower symbol */
        if (sym != 0 && (sym->st_shndx != SHN_UNDEF || sym->st_value != 0)
            && ELF32_ST_TYPE(sym->st_info)!= STT_TLS) {

          /* Check if sym_addr <= addr < sym_end */
          if (addr >= so->base_addr + sym->st_value
              && (((sym->st_shndx == SHN_UNDEF || sym->st_size == 0)
                   && addr == so->base_addr + sym->st_value)
                  || (char *)addr < ((char *)so->base_addr + sym->st_value +
                                     sym->st_size))
              && (sym_found==0 || sym_found->st_value < sym->st_value)) {
            
            sym_found = sym;
          }
        }
        
        /* Try next symbol */
        sym++;
        
        /* Until chain exhausted */
      } while ((*hasharr++ & 1) == 0);
    }
  }

  return sym_found;
}


/**
 * ELF Hashing function. Returns hash table entry for a symbol with the given
 * name. For details see ELF format documentation.
 * @param name Name of the symbol
 * @return hash table entry
 */
static unsigned long elf_hash(const char *name) {
  unsigned long h = 0, g;

  while (*name) {
    h = (h << 4) + *name++;
    if ((g = h & 0xf0000000))
      h ^= g >> 24;
    
    h &= ~g;
  }
  
  return h;
}


/**
 * GNU Hashing function. Returns hash table entry for a symbol with the
 * given name.
 * @param name Name of the symbol
 * @return hash table entry
 */
static Elf32_Word gnu_hash (const char *name) {
  Elf32_Word h = 5381;
  unsigned char c;
  
  for (c = *name; c != '\0'; c = *++name)
    h = h * 33 + c;

  return h & 0xffffffff;
}


/**
 * Checks if found symbol in the given shared object has the right version.
 * @param so The shared object the symbol was found in
 * @param symtab_index The symbol table index of the symbol
 * @param version The version the symbol should have
 * @return 1 if right version, 0 otherwise
 */
static long check_version(const dso *so, long symtab_index,
                          const struct version_info *version) {
  if (version != 0) {
    /* Versioned lookup */
    if (so->versym == 0) {
      /* No version information found -> fail */
      return 0;
    } else {
      /* Compare versions*/
      Elf32_Half index = so->versym[symtab_index] & BITMASK_LOW15;
      
      /* Check hash and name */
      if (so->versions[index].hash == version->hash
          && !sl_strncmp(so->versions[index].name, version->name,
                         MAX_VERS_LEN)) {
        return 1;   
      } 
    }
  } else {
    /* Non-versioned lookup */
    
    if (so->versym == 0) {
      /* No version information in shared object,
         just use the symbol we found */
      return 1; 
    } else {
      /* Use default version */

      /* Skip hidden symbols */
      if ((so->versym[symtab_index] & BITMASK_16) == 0) {
        return 1;
      }
    }    
  }
  return 0;
}


/**
 * Does a symbol lookup in the elf hash table of the given shared object.
 * Returns pointer to the symbol if found, otherwise NULL.
 * @param so Shared object to search in
 * @param name Name of symbol to search
 * @return pointer to symbol on success, NULL otherwise
 */
static Elf32_Sym *elf_hash_lookup(const dso *so,
                                  const char *name, long name_len,
                                  const struct version_info *version) {
  /*
    Hash table structure:
    
    nbucket
    nchain
    bucket[0]
    ...
    bucket[nbucket - 1]
    chain[0]
    ...
    chain[nchain - 1]
  */
  
  Elf32_Word nbucket = so->hash_table[0];
  Elf32_Word *bucket = &so->hash_table[2];
  Elf32_Word *chain = bucket + nbucket;
    
  /* Get bucket entry */
  Elf32_Word hash = elf_hash(name);
  unsigned long bucket_entry = bucket[hash % nbucket];
  
  /* Check first result */
  Elf32_Sym *sym = so->symbol_table + bucket_entry;
  if (sym->st_name) {
    /* Check name */
    if (!sl_strncmp(so->string_table + sym->st_name, name, name_len)) {
      /* Check version */
      if (check_version(so, bucket_entry, version))
        return sym;
    }
  }

  /* Not found. Chain[bucket_entry] gives next result */
  while (chain[bucket_entry] != STN_UNDEF) {
    long symtab_index = chain[bucket_entry];
    sym = so->symbol_table + symtab_index;

    /* Check name */
    if (sym->st_name && !sl_strncmp(so->string_table + sym->st_name,
                                    name, name_len)) {
      /* Check version */
      if (check_version(so, symtab_index, version))
        return sym;
    }
    bucket_entry = chain[bucket_entry];
  }

  return 0;
}


/**
 * Does a symbol lookup in the gnu hash table for the given shared object.
 * Returns a pointer to the symbol if found, otherwise NULL.
 * @param so Shared object to search in
 * @param name Name of symbol to search
 * @return pointer to symbol on success, NULL otherwise
 */
static Elf32_Sym *gnu_hash_lookup(const dso *so,
                                  const char *name, long name_len,
                                  const struct version_info *version) {
  /*
    Read the gnu hash table setup:
    nbuckets        The number of hash buckets
    bias            The index of the first symbol in symbol table
    bloom_nwords    Number of bloom filter words - 1
    shift           Bloom filter hash shift
    bloom           Bloom filter words
    ...
    bucket          Hash buckets
    ...
    chain           Hash chain
    ...
  */
  Elf32_Word nbuckets = so->gnu_hash_table[0];
  Elf32_Word bias = so->gnu_hash_table[1];
  Elf32_Word bloom_nwords = so->gnu_hash_table[2];
  Elf32_Word shift = so->gnu_hash_table[3];
  Elf32_Addr *bloom = (Elf32_Addr *) &so->gnu_hash_table[4];
  Elf32_Word *bucket = &so->gnu_hash_table[4 + bloom_nwords];
  Elf32_Word *chain = &bucket[nbuckets] - bias;
 
  /* Get hash */
  Elf32_Word hash = gnu_hash(name);

  /* Check with bloom filter */
  Elf32_Addr bloom_word = bloom[(hash / 32) & (bloom_nwords - 1)];
  Elf32_Word h1 = hash & (32 - 1);
  Elf32_Word h2 = (hash >> shift) & (32 - 1);
  if (!((bloom_word >> h1) & (bloom_word >> h2) & 1))
    return 0;
  
  /* Get bucket entry */
  Elf32_Word bucket_entry = bucket[hash % nbuckets];
  if (bucket_entry == 0)
    return 0;

  /* and find match in chain */
  Elf32_Word *hasharr = &chain[bucket_entry];
  Elf32_Sym *sym = &so->symbol_table[hasharr - chain];

  do {  
    if (((*hasharr ^ hash) >> 1) == 0) {
      /* Hash matches, check name */  
      if (sym->st_name && !sl_strncmp(&so->string_table[sym->st_name],
                                      name, name_len)) {
        /* Check version */
        long symtab_index = hasharr-chain;
        if (check_version (so, symtab_index, version))
          return sym;        
      }
    }

    /* Try next symbol */
    sym++;

    /* Until chain exhausted */
  } while ((*hasharr++ & 1) == 0);

  return 0;
}


/**
 * Does a symbol lookup in the DSO for a symbol with the given name and
 * version.
 * @param so The dynamic shared_object to search in
 * @param name Name of the symbol
 * @param name_len Length of the name
 * @param version Version of the symbol
 * @return Elf32_Sym on success, NULL otherwise
 */
static Elf32_Sym *sym_lookup(const dso *so, const char *name, long name_len,
                             const struct version_info *version) {

  if (so->gnu_hash_table != 0) {
    /* Use fast GNU hash table lookup */
    return gnu_hash_lookup(so, name, name_len, version);
  } else if (so->hash_table != 0) {
    /* Use slower "old" hash table lookup */
    return elf_hash_lookup(so, name, name_len, version);
  } else {
    /* No hash table found */
    sl_printf("Error sym_lookup: no hash_table found.\n");
    sl_exit(1);
  }
  return 0;
}


unsigned long resolve_plt(dso *so, unsigned long offset) {
  /* Get relocation entry and fixup location in the Global Offset Table */
  Elf32_Rel *relocation_entry = (Elf32_Rel *) BYTE_STEP(so->pltrel, offset); 
  unsigned long *got_fixup =
    (unsigned long *) (relocation_entry->r_offset + so->base_addr); 

  /* Get symbol table entry */
  long symtab_index = ELF32_R_SYM(relocation_entry->r_info);

#ifdef SL_STATISTIC
  num_relocs++;
#endif
  
  /* Symbol versioning */
  struct version_info *version = 0;
  if (so->versym != 0) {
    /* Get version index of needed symbol */
    Elf32_Half index = so->versym[symtab_index] & BITMASK_LOW15;
    /* Get version_info entry */
    version = &so->versions[index];
    if (version->hash == 0)
      version = 0;
  }
  
#ifdef D_SL
  sl_printf("\nResolving symbol \"%s\" [%s] in %s.\n",
            so->string_table + so->symbol_table[symtab_index].st_name,
            version ? version->name : "", so->name);
#endif
  
  dso *so_found;
  Elf32_Sym *sym_found;
 
#ifdef MEMORY_PROTECTION
  /* Check for GNU_RELRO */
  if (so->relro) {
    /* Global Offset Table is readonly, make it writable */
    UNPROT_DATA(TRUNC_PAGE(got_fixup), ROUND_PAGE(sizeof(*got_fixup)));
  }
#endif

  /* Resolve address of symbol and set GOT entry */
  *got_fixup =
    (unsigned long) resolve_symbol(so, so->string_table
                                   + so->symbol_table[symtab_index].st_name,
                                   version, 1, 0, &sym_found, &so_found);

#ifdef MEMORY_PROTECTION
  /* Check for GNU_RELRO */
  if (so->relro) {
    /* Reprotect GOT */
    char *rstart = (char *)TRUNC_PAGE(so->relro);
    char *rend = (char *)TRUNC_PAGE(so->relro + so->relro_size);
    PROT_DATA(rstart, rend-rstart);
  }  
#endif

  /* Check result */
  if(*got_fixup == 0) {
    sl_printf("Error resolving symbol %s in %s\n",
              so->string_table + so->symbol_table[symtab_index].st_name,
              so->path);
    sl_exit(1);
  }


#ifdef D_SL
  sl_printf("Fixed value at %p with %p.\n", got_fixup,
            (void *)*got_fixup);
#endif
  /* Return symbol address */
  return *got_fixup;
}


void *resolve_symbol(dso *so_start, const char *name,
                     const struct version_info *version,
                     unsigned char global_scope, unsigned char skip_main,
                     Elf32_Sym **sym_found, dso **so_found) {
  Elf32_Sym *sym = 0;
  dso *so = 0;
  long name_len = sl_strnlen(name, MAX_SYM_NAME);

  /*
   * We have 3 different search scopes:
   * - secuLoader itself: all exported functions/variables of the loader
   * - global scope: all DSOs loaded at startup + those loaded at runtime
   *   with flag RTLD_GLOBAL
   * - local scope: all DSOs the initiating DSO depends on
   */
  
  /* We always have to search first in the exported variables/functions of the
     loader itself to prevent other DSOs from overriding these symbols. */
  
  /* Search in exported functions of the loader */
  struct function *func = loader_functions;
  for (func = loader_functions; func->addr != 0; ++func) {
    if (!sl_strncmp(name, func->name, name_len))
      return func->addr;
  }

  /* Search in exported variables of the loader */
  struct variable *var = loader_variables;
  for (var = loader_variables; var->addr != 0; ++var) {
    if (!sl_strncmp(name, var->name, name_len)) {
      /* Do we have to dereference once? */
      if(var->deref)
        return *var->addr;
      else
        return var->addr;
    }
  }

  if (global_scope) {
    /* Search objects in global scope */
    for (so = gscope; so != 0; so = so->gs_next) {
      /* Skip main DSO? */
      if (skip_main && so->type == ET_EXEC)
        continue;
#ifdef D_SL      
      sl_printf("Looking in %s\n", so->name);
#endif
      /* Lookup symbol in this DSO */
      sym = sym_lookup(so, name, name_len, version);
      
      /* Check result */
      if (sym != 0 && sym->st_shndx != SHN_UNDEF) {
#ifdef D_SL
        sl_printf("Found symbol %s in %s at %p.\n", name, so->path,
                  (char*) sym->st_value);
#endif
        *so_found = so;
        *sym_found = sym;
        
        /* Get address by adding base address to symbol value */
        void *value = so->base_addr + sym->st_value;
        
        if (ELF32_ST_TYPE(sym->st_info) == STT_GNU_IFUNC) {
          /* GNU IFUNC extension: the function at the symbol address
             returns the real address */
          value = ((void *(*) (void))(value))();
        }
        return value;
      }
    }
  }
  
  /* If we have a global handle or the initiating DSO is in the global scope,
     we do not have to search the local scope */
  if (so_start == 0
      || ((so_start->gs_next || so_start->gs_prev) && global_scope))
    return 0;
  
  /* Search DSOs in local scope */
  so = so_start;
  long i = 0;
  for (i = 0; i<so->lscope_num; ++i) {
    if (!so->lscope[i])
      break;

#ifdef D_SL
    sl_printf("Looking in %s\n", so->lscope[i]->name);
#endif
    /* Lookup symbol in this DSO */
    sym = sym_lookup(so->lscope[i], name, name_len, version);
    
    /* Check result */
    if (sym != 0 && sym->st_shndx != SHN_UNDEF) {
#ifdef D_SL
      sl_printf("Found symbol %s in %s at %p.\n", name,
                so->lscope[i]->path, (char*) sym->st_value);
#endif
      *so_found = so->lscope[i];
      *sym_found = sym;
      
      /* Get address by adding base address to symbol value */
      void *value = so->lscope[i]->base_addr + sym->st_value;
      
      if (ELF32_ST_TYPE(sym->st_info) == STT_GNU_IFUNC) {
        /* GNU IFUNC extension: the function at the symbol address
           returns the real address */
        value = ((void *(*) (void))(value))();
      }
      return value;
    }
  }
  
  return 0;
}


/**
 * This is called by the runtime_trampoline and resolves
 * the symbol specified by the arguments on the stack.
 * Returns the resolved address.
 */
unsigned long resolve() {
  dso *so;
  unsigned long offset;

  /* 
    Stack Layout:
    24:  offset
    20:  so
    16:  return address
    12:  %eax
     8:  %ecx
     4:  %edx
  */

  /* Get parameters (shared object and offset) from stack */ 
  asm("movl 20(%%ebp), %0\n"
      "movl 24(%%ebp), %1"
      :"=r"(so), "=r"(offset)
      );

#ifdef SL_STATISTIC
  num_relocs++;
#endif
  
  return resolve_plt(so, offset);
}



/*
  -----------------------------------------------------------------------------
  Note: The following functions are exported and used e.g. by the libc.
  DO NOT CHANGE THEIR BEHAVIOUR!
  -----------------------------------------------------------------------------
*/


/* EXPORTED FUNCTIONS */
struct link_map * REG_ARGS(3) lookup_symbol_x
  (const char *undef_name, struct link_map *undef_map, const Elf32_Sym **ref,
   struct r_scope_elem *symbol_scope[], const struct version_info *version,
   long type_class, long flags, struct link_map *skip_map) {

#ifdef D_RSL
  sl_printf("\nLookup symbol: \"%s\" in %s (type_class %d, flags %x, skip %p)\n", undef_name, undef_map->l_name, type_class, flags, skip_map);
#endif

  /* Not supported flags */
  if (flags & DL_LOOKUP_ADD_DEPENDENCY) {
    /* If necessary add dependency between user and provider object. */
    //    sl_printf("Error lookup_symbol: DL_LOOKUP_ADD_DEPENDENCY\n");
    //    sl_exit(1);
  } else if (flags & DL_LOOKUP_RETURN_NEWEST) {
    /* Return most recent version instead of default version for
    unversioned lookup.  */
    //    sl_printf("Error lookup_symbol: DL_LOOKUP_RETURN_NEWEST\n");
    //    sl_exit(1);
  } else if (flags & DL_LOOKUP_GSCOPE_LOCK) {
    /* Set if dl_lookup called with GSCOPE lock held.  */
    //    sl_printf("Error lookup_symbol: DL_LOOKUP_GSCOPE_LOCK\n");
    //    sl_exit(1);
  } else {
    sl_printf("Error lookup_symbol: unknown flag 0x%x.\n", flags);
    sl_exit(1);
  }

  if (skip_map != 0) {
    /* Not yet supported */
    sl_printf("Error lookup_symbol: skip_map not suppported.\n");
    sl_exit(1);
  }
    
  dso *so_found = 0;
  Elf32_Sym *sym_found = 0;
  void *ret = 0;

  if (undef_map == (struct link_map *) so_chain) {
    /* Handle to global scope */

    /* Search symbol in global scope */
    ret = resolve_symbol(0, undef_name, version, 1, 0, &sym_found, &so_found);

    /* Check result */
    if ((long) ret == 0) {
      /* Symbol not found */
#ifdef D_RSL
      sl_printf("Symbol \"%s\" not found, throwing error\n", undef_name);
#endif
      /* Throw error */
      signal_error(1, "dl_lookup_symbol_x", 0, "undefined symbol");
    }
  } else {
    /* Only search in undef_map */
    ret = resolve_symbol((dso *) undef_map, undef_name, version, 0,
                          0, &sym_found, &so_found);

    /* Check result */
    if ((long) ret == 0) {
      /* Symbol not found */
#ifdef D_RSL
      sl_printf("Symbol \"%s\" not found, throwing error\n", undef_name);
#endif
      /* Throw error */
      signal_error(1, "dl_lookup_symbol_x", 0, "undefined symbol");
    }
  }

#ifdef D_RSL
  sl_printf("Symbol found at %p in %s\n", sym_found->st_value, so_found->name);
#endif
  
  /* Return Elf32_Sym and address */
  *ref = sym_found;
  return (struct link_map *)so_found;
}


long REG_ARGS(3) dl_addr(const void *address, Dl_info *info,
                         struct link_map **mapp, const Elf32_Sym **symbolp) {
#ifdef D_RSL
  sl_printf("dl_addr: address %p, info %p\n", address, info);
#endif

  /* Try get DSO at address */
  dso *so = get_object_at((char *) address);
  if (so == 0) {
#ifdef D_RSL
    sl_printf("Error dl_addr: DSO not found\n");
#endif
    return 0;
  }

  /* Set result */
  if (mapp != 0) {
    *mapp = (struct link_map *)so;
  }
   
  /* Try to resolve a symbol at address */ 
  Elf32_Sym *sym = get_symbol_at(so, (char *) address);
  if (symbolp != 0) {
    *symbolp = sym;
  }
  
  /* Path and start address of DSO */
  info->dli_fname = so->path;
  info->dli_fbase = so->text_addr;

  if (sym != 0) {
    /* Symbol name and address */
    info->dli_sname = so->string_table + sym->st_name;
    info->dli_saddr = so->base_addr + sym->st_value;
  } else {
    info->dli_sname = 0;
    info->dli_saddr = 0;
  }  
  
  return 1;
}


long dl_iterate_phdr (long (*callback) (struct dl_phdr_info *info,
                                        unsigned long size, void *data),
                      void *data) {
#ifdef D_RSL
  sl_printf("dl_iterate_phdr called\n");
#endif
  struct dl_phdr_info info;
  long ret = 0;
  dso *so;

  /* Iterate over loaded shared objects and gather information */  
  for (so = so_chain->next; so != 0; so = so->next) {
    /* We only support the first 4 members of the dl_phdr_info
       struct (First 16 bytes). */
    info.dlpi_addr = (Elf32_Addr)so->base_addr;
    info.dlpi_name = so->path;
    info.dlpi_phdr = so->program_header;
    info.dlpi_phnum = so->program_header_num;

    /* Callback */
    ret = callback (&info, 16, data);
    if (ret)
      return ret;
  }
  
  return 0;
}


/* This function exists only for the debugger to set a breakpoint */
void debug_state (void) {

}

/*
 ----------------------------------------------------
 Note: The following functions are not yet supported.
 ----------------------------------------------------
*/

long __attribute__((regparm (1))) make_stack_executable (void **stack_endp) {
  sl_printf("Error: not implemented make_stack_executable\n");
  sl_exit(1);
  return 0;
}


void mcount (Elf32_Addr frompc, Elf32_Addr selfpc) {
  sl_printf("Error: not implemented mcount\n");
  sl_exit(1);
}


void rtld_di_serinfo () {/*__attribute__((regparm (3))) (struct link_map *loader, Dl_serinfo *si, _Bool counting) {*/
  sl_printf("Error: not implemented rtld_di_serinfo\n");
  sl_exit(1);
}
