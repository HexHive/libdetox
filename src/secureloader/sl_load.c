/**
 * @file sl_load.c
 * Implements the loading of elf files.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
 *
 * $Date: 2012-04-13 13:31:40 +0200 (Fri, 13 Apr 2012) $
 * $LastChangedDate: 2012-04-13 13:31:40 +0200 (Fri, 13 Apr 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1286 $
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


#include "sl_load.h"
#include "sl_loader.h"
#include "sl_libc.h"
#include "sl_macros.h"
#include "sl_datatypes.h"
#include "sl_so_chain.h"
#include "sl_map.h"
#include "sl_libsearch.h"
#include "sl_gscope.h"
#include "sl_sym_lookup.h"
#include "sl_libdetox.h"

#ifdef SL_STATISTIC
/* The number of loaded DSOs during this run */
long loaded_dsos = 0;
/* The number of currently loaded DSOs */
long curr_loaded_dsos = 0;
/* The maximum number of loaded DSOs */
long max_loaded_dsos = 0;
#endif

extern struct rtld_global *rtld_glob;
extern dso *so_chain;
extern void runtime_trampoline();


/**
 * Checks if the given elf file is valid.
 * @param elf_hdr Header of the elf file
 * @return 0 if valid, error number otherwise
 */
static long check_elf(Elf32_Ehdr *elf_hdr) {

  /* Check magic number */
  if( (elf_hdr->e_ident[EI_MAG0] != 0x7F) ||
      (elf_hdr->e_ident[EI_MAG1] != 'E') ||
      (elf_hdr->e_ident[EI_MAG2] != 'L') ||
      (elf_hdr->e_ident[EI_MAG3] != 'F') ) {
    return 1;
  }

  /* Check if elf class is 32bit */
  if(elf_hdr->e_ident[EI_CLASS] != ELFCLASS32)
    return 2;

  /* Check data encoding */
  if(elf_hdr->e_ident[EI_DATA] != ELFDATA2LSB)
    return 3;

  return 0;
}


/**
 * Initializes the version array of the given shared object.
 * Each needed or defined version has a unique version index used in the
 * version symbol table (so->versym) to reference it.
 * So we have to create an array of all defined/needed versions, where we can
 * later lookup the version of a symbol using this version index.
 * (See http://www.akkadia.org/drepper/symbol-versioning)
 * @param so The shared object 
 */
static void init_versions(dso *so) {
  Elf32_Verneed *need = so->verneed;
  Elf32_Verdef *def = so->verdef;
  unsigned long i = 0, j = 0, num = 0;

  /* We first need to determine the highest version index which equals the
     number of entries we have to allocate in our versions array */
  
  if (need != 0) {
    /* This object requires versioned symbols */
    /* Iterate over dependencies */
    for (i = 0; i < so->verneed_num; ++i) {
      Elf32_Vernaux *aux = (Elf32_Vernaux *)((char *)need + need->vn_aux);

      /* Iterate over versions needed in this dep.*/
      for (j = 0; j < need->vn_cnt; ++j) {
        /* This is a unique index used in the version table */
        Elf32_Half index = aux->vna_other & BITMASK_LOW15;
        if (index > num)
          num = index;
                
        aux = (Elf32_Vernaux *) ((char *)aux + aux->vna_next);
      }
      need = (Elf32_Verneed *) ((char *)need + need->vn_next);
    }
  }

  if (def != 0) {
    /* This object defines versioned symbols */
    /* Iterate over definitions */
    for (i=0; i<so->verdef_num; ++i) {
      /* This is a unique index used in the version table */
      Elf32_Half index = def->vd_ndx & BITMASK_LOW15;
      if (index > num)
        num = index;
           
      def = (Elf32_Verdef *) ((char *)def + def->vd_next);
    }
  }

  /* No versioning */
  if (num == 0)
    return;
  
  /* We have the number, now allocate memory */
  so->versions =
    (struct version_info *) sl_calloc(num + 1, sizeof(struct version_info *));

  need = so->verneed;
  def = so->verdef;
  
  if (need != 0) {
    /* Initialize needed symbol versions */
    for (i=0; i<so->verneed_num; ++i) {
      Elf32_Vernaux *aux = (Elf32_Vernaux *) ((char *)need + need->vn_aux);

      for (j=0; j<need->vn_cnt; ++j) {
        /* Initialize versions entry, the index is unique and later used to
           lookup the needed version of a symbol */
        Elf32_Half index = aux->vna_other & BITMASK_LOW15;  
        so->versions[index].hash = aux->vna_hash;
        so->versions[index].hidden = aux->vna_other & BITMASK_16;
        so->versions[index].name = so->string_table + aux->vna_name;
        so->versions[index].filename = so->string_table + need->vn_file;
        
        aux = (Elf32_Vernaux *) ((char *)aux + aux->vna_next);
      }      
      need = (Elf32_Verneed *) ((char *)need + need->vn_next);
    }
  }
  
  if (def != 0) {
    /* Initialize defined symbol versions */
    for (i=0; i<so->verdef_num; ++i) {
      Elf32_Verdaux *aux = (Elf32_Verdaux *) ((char *)def + def->vd_aux);
      
      /* Initialize versions entry, the index is unique and later used to
         lookup the defined version of a symbol */
      Elf32_Half index = def->vd_ndx & BITMASK_LOW15;
      so->versions[index].hash = def->vd_hash;
      so->versions[index].name = so->string_table + aux->vda_name;
      so->versions[index].filename = 0;
      
      def = (Elf32_Verdef *) ((char *)def + def->vd_next); 
    }
  }   
}


/**
 * Initializes the global offset table of the given shared object.
 * We have to set the runtime trampoline which resolves symbols at runtime.
 * @param so The shared object
 */
static void init_got(dso *so) {
  unsigned long *gaddr = (unsigned long *)so->gotplt;
  void (*fp_runt_tramp)() = runtime_trampoline;

  unsigned long *reloc;
  void *value;

  if (gaddr != 0) {
    /* The first entry is the dso pointer we later get as parameter, the
       second is our runtime trampoline which receives the dso pointer and
       resolve the symbol. */
    *(++gaddr) = (unsigned long)so;
    *(++gaddr) = (unsigned long)fp_runt_tramp;
    
    /* These entries should point back to the plt for lazy binding */
    long i=0;
    
    Elf32_Rel *rel = so->pltrel;
    if (rel == 0)
		return;
			
    for (i=0; i<so->pltrelsz; ++i, ++rel) {
	
	   /* Get relocation address */
	  reloc = (unsigned long *) BYTE_STEP(so->base_addr, rel->r_offset);
    
	  if(ELF32_R_TYPE(rel->r_info) == R_386_IRELATIVE) {
		  value = (void*)BYTE_STEP(so->base_addr, *reloc);
		  value = ((void *(*) (void))(value))();
		  *reloc = (unsigned long)value;  
	  } else {
		 value = (void*)BYTE_STEP(so->base_addr, *reloc);
		 *reloc = (unsigned long)value;
	  }
	  
    }
    
  }
}

uint32_t get_aux_value(Elf32_auxv_t *auxv_orig, uint32_t type) {
	
	Elf32_auxv_t *auxv = auxv_orig;
	
	for(; auxv->a_type != AT_NULL; auxv++) {
		if(auxv->a_type == type) {
			return auxv->a_un.a_val;
		}
	}
	
	return 0;
}

dso *load_elf(dso *loader, const char *name, const char *path, long fd,
              unsigned char rt_load, unsigned int rtld_mode, Elf32_auxv_t *auxv) {
#ifdef D_LOAD
  sl_printf("\nLoading elf file: %s (%s)\n", path, name);
#endif

#ifdef SL_STATISTIC
  /* Some statistics */
  loaded_dsos++;
  curr_loaded_dsos++;
  max_loaded_dsos = MAX(max_loaded_dsos, curr_loaded_dsos);    
#endif
  
  /* Get file information */
  struct kernel_stat file_info; 
  if(sl_fstat(fd, &file_info) == -1) {
    /* Close file */
    sl_close(fd);

    /* Signal error (longjmp) if we load at runtime */
    if(rt_load)
      signal_error(0, name, 0, "fstat failed");

    /* Not at runtime -> fail */
    sl_printf("Error load_elf: fstat failed while loading %s.\n", name);
    sl_exit(1);
  }
  
  /* Map entire file in memory */
  void *file_map = sl_mmap(0, file_info.st_size, PROT_READ,
                           MAP_PRIVATE, fd, 0);
  if ((long)file_map == -1) {
    /* Close file */
    sl_close(fd);

    /* Signal error (longjmp) if we load at runtime */
    if(rt_load)
      signal_error(0, name, 0, "mmap failed");

    /* Not at runtime -> fail */
    sl_printf("Error load_elf: mmap of file %s failed.\n", name);
    sl_exit(1);
  }
  
  /* Get ELF header and check file */
  Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) file_map;
  long valid = check_elf(elf_hdr);
  if (valid != 0) {
    /* Invalid elf file */
    sl_close(fd);

    /* Signal error (longjmp) if we load at runtime */
    if(rt_load)
      signal_error(0, name, 0, "invalid ELF file");

    /* Not at runtime -> fail */
    sl_printf("Error load_elf: %s is not a valid ELF file (error: %d).\n",
              name, valid);
    sl_exit(1);
  }

  /* Get program and section header */
  Elf32_Phdr *program_hdr = (Elf32_Phdr *) (file_map + elf_hdr->e_phoff);
  Elf32_Shdr *shdr = (Elf32_Shdr *) (file_map + elf_hdr->e_shoff);
  /* Segments (text and data) which we have to map in memory */
  Elf32_Phdr *load_segments[2];
  long num_load = 0;
  
  /* Create new shared object */
  dso *so = sl_calloc(sizeof(dso), 1);

  /* Iterate over program headers */
  unsigned long i = 0;
  for(i = 0; i < elf_hdr->e_phnum; ++i) {
    switch (program_hdr[i].p_type) {
      
    case PT_DYNAMIC: /* Dynamic Header */
      so->dynamic_section = (Elf32_Dyn *)(program_hdr[i].p_vaddr);
      break;
      
    case PT_LOAD: /* Section must be mapped in memory */
      if (num_load >= 2) {
        sl_printf("Error load_elf: more than two PT_LOAD segments!");
        sl_exit(1);
      }
      load_segments[num_load++] = program_hdr+i;
      break;

    case PT_TLS: /* Thread Local Storage information*/
      if (program_hdr[i].p_memsz == 0)
        break;

      /* Initialize TLS information */
      so->tls_blocksize = program_hdr[i].p_memsz;
      so->tls_align = program_hdr[i].p_align;
      so->tls_initimage_size = program_hdr[i].p_filesz;

      /* TLS image (addr later adjusted) */
      so->tls_initimage = (void *) program_hdr[i].p_vaddr;
      
      /* Assign next module ID */
      so->tls_modid = ++GL(_dl_tls_max_dtv_idx);          
      break;

      /*
    case PT_GNU_STACK:
      if (program_hdr[i].p_flags & PF_X) {
        sl_printf("Warning: executable stack\n");
        sl_exit(1);
      }
      break;
      */
      
    case PT_GNU_RELRO: /* Sections to set readonly after relocation */
      /* Address is later adjusted */
      so->relro = program_hdr[i].p_vaddr;
      so->relro_size = program_hdr[i].p_memsz;
      break;
    }
  }

  /* Map segments into memory and intitialize dso struct */
  if(rtld_mode == 1 && auxv != NULL) {
	  
	  Elf32_Phdr *program_hdr_auxv;
	  Elf32_Phdr *load_segments_auxv[2];
	  
	  program_hdr_auxv = (Elf32_Phdr *)get_aux_value(auxv, AT_PHDR);
	  uint32_t auxv_e_phnum = (uint32_t)get_aux_value(auxv, AT_PHNUM);
	  unsigned long j = 0;
	  unsigned int nr_load = 0;
	  
	  for(j = 0; j < auxv_e_phnum; j++) {
		switch (program_hdr_auxv[j].p_type) {
		   
		case PT_LOAD: /* Section must be mapped in memory */
		  if (nr_load >= 2) {
			sl_exit(1);
		  }
		  load_segments_auxv[nr_load++] = program_hdr_auxv+j;
		  break;

		}
	  }
	  
	  map_segments_RTLD(fd, load_segments, elf_hdr->e_type, so, load_segments_auxv);
	  
  } else {
	  map_segments(fd, load_segments, elf_hdr->e_type, so);
  }
  
  so->ref_count = 1;
  so->deps_count = 0;
  so->name = name;
  so->path = path;
  so->type = elf_hdr->e_type;
  so->entry = (void*) elf_hdr->e_entry;
  so->loader = loader;
  so->dynamic_section = (Elf32_Dyn *) BYTE_STEP(so->dynamic_section,
                                                so->base_addr);
  so->program_header = (Elf32_Phdr *) BYTE_STEP(elf_hdr->e_phoff,
                                                so->text_addr);
  so->program_header_num = elf_hdr->e_phnum;
   
  so->l_real = so;
  
  /* Adjust address of TLS init image and relro address */
  if (so->tls_initimage) {
    so->tls_initimage = (char *)so->tls_initimage + (long)so->base_addr;
  }
  if (so->relro) {
    so->relro = (Elf32_Addr) BYTE_STEP(so->relro, so->base_addr);
  }
  
  /* Iterate over section headers */
  char *strtab = (char *)file_map + shdr[elf_hdr->e_shstrndx].sh_offset;
  char *sname=0;
  for (i=0; i<elf_hdr->e_shnum; ++i) {
    sname = strtab + shdr[i].sh_name;

    /* Save important sections */
    if (sl_strncmp(sname, ".got", 5) == 0) {
      so->got = (char *) (so->base_addr + shdr[i].sh_addr);
      so->got_size = shdr[i].sh_size;
    }
    if (sl_strncmp(sname, ".plt", 5) == 0) {
      so->plt = (char *) (so->base_addr + shdr[i].sh_addr);
      so->plt_size = shdr[i].sh_size;
    }
    if (sl_strncmp(sname, ".got.plt", 9) == 0) {
      so->gotplt = (char *) (so->base_addr + shdr[i].sh_addr);
      so->gotplt_size = shdr[i].sh_size;
    }
  }

  /* Resolve */
  Elf32_Dyn *dyn;
  long rpath=-1;
  for (dyn = so->dynamic_section; dyn->d_tag != DT_NULL; ++dyn) {
    switch (dyn->d_tag) {
      
    case DT_INIT: /* Initialization function */
      so->init = (void (*)(int, char**, char**))
        BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;
      
    case DT_INIT_ARRAY: /* Array of initialization functions */
      so->init_array = (Elf32_Addr *)BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;
      
    case DT_INIT_ARRAYSZ: /* Size of init array */
      so->init_array_sz = (long)dyn->d_un.d_val / sizeof(Elf32_Addr);
      break;
      
    case DT_FINI: /* Finalization function */
      so->fini = (void (*)()) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;

    case DT_FINI_ARRAY: /* Array of finalization functions */
      so->fini_array = (Elf32_Addr *)BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;

    case DT_FINI_ARRAYSZ: /* Size of fini array */
      so->fini_array_sz = (long)dyn->d_un.d_val / sizeof(Elf32_Addr);
      break;
      
    case DT_RUNPATH: /* String with library search paths */
      rpath = dyn->d_un.d_val;
      break;	
      
    case DT_RPATH: /* String with library search paths */
      if (rpath == -1)
        rpath = dyn->d_un.d_val;
      break;

    case DT_PLTGOT: /* Plt part of the global offset table */
      so->gotplt = (char *) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;
      
    case DT_REL: /* Relocation table */
      so->rel = (Elf32_Rel *) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;

    case DT_RELSZ: /* Size of the relocation table */
      so->relsz = (long)dyn->d_un.d_val / sizeof(Elf32_Rel);
      break;
        
    case DT_JMPREL: /* Plt relocations part of relocation table */
      so->pltrel = (Elf32_Rel *) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;
            
    case DT_PLTRELSZ: /* Size of plt relocations part of relocation table */
      so->pltrelsz = (long)dyn->d_un.d_val / sizeof(Elf32_Rel);
      break;

    case DT_HASH: /* ELF hash table */
      so->hash_table = (Elf32_Word *) BYTE_STEP(dyn->d_un.d_ptr,
                                                so->base_addr);
      break;

    case DT_GNU_HASH: /* GNU hash table */
      so->gnu_hash_table = (Elf32_Word *) BYTE_STEP(dyn->d_un.d_ptr,
                                                         so->base_addr);
      break;
      
    case DT_SYMTAB: /* Dynamic symbol table */
      so->symbol_table = (Elf32_Sym *) BYTE_STEP(dyn->d_un.d_ptr,
                                                 so->base_addr);
      break;

    case DT_VERDEF: /* Versions defined in this DSO */
      so->verdef = (Elf32_Verdef *) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;

    case DT_VERDEFNUM: /* Number of versions defined in this DSO */
      so->verdef_num = (unsigned long) dyn->d_un.d_val;
      break;

    case DT_VERNEED: /* Versions needed by this DSO */
      so->verneed = (Elf32_Verneed *) BYTE_STEP(dyn->d_un.d_ptr,
                                                so->base_addr);
      break;

    case DT_VERNEEDNUM: /* Number of versions needed by this DSO */
      so->verneed_num = (unsigned long) dyn->d_un.d_val;
      break;

    case DT_VERSYM: /* Version symbol table */
      so->versym = (Elf32_Half *) BYTE_STEP(dyn->d_un.d_ptr, so->base_addr);
      break;
      
    case DT_STRTAB: /* Dynamic string table */
      so->string_table = (char *) so->base_addr + dyn->d_un.d_ptr;
      break;

    case DT_NEEDED: /* Dependencies on other DSOs */
      /* Count the number of direct dependencies */
      so->deps_count++;
      break;

    case DT_FLAGS: /* Flags */
      so->flags = dyn->d_un.d_val;
      if ((so->flags & DF_SYMBOLIC)
          || (so->flags & DF_TEXTREL)) {
        sl_printf("Error load_elf: not supported flag 0x%x in %s.\n",
                  so->flags, so->name);
        sl_exit(1);
      }
      break;

    case DT_FLAGS_1: /* Flags */
      so->flags_1 = dyn->d_un.d_val;
      if ((so->flags_1 & DF_1_GROUP)
          || (so->flags_1 & DF_1_LOADFLTR)
          || (so->flags_1 & DF_1_DIRECT)
          || (so->flags_1 & DF_1_INTERPOSE)
          || (so->flags_1 & DF_1_NODEFLIB)
          //          || (so->flags_1 & DF_1_NODUMP)
          || (so->flags_1 & DF_1_CONFALT)
          || (so->flags_1 & DF_1_ENDFILTEE)
          || (so->flags_1 & DF_1_DISPRELDNE)
          || (so->flags_1 & DF_1_DISPRELPND)) {
        sl_printf("Error load_elf: not supported flag_1 0x%x in %s.\n",
                  so->flags_1, so->name);
        sl_exit(1);
      }
          
      break;
    }
  }

  /* Initialize the versioning data */
  init_versions(so);
  
  /* Set library search paths */
  if (rpath != -1) 
    so->search_path = decompose_path(so->string_table+rpath,so->name, "RPATH");
  
  /* Allocate memory for deps */
  if (so->deps_count != 0)
    so->deps = sl_malloc(so->deps_count * sizeof(dso *));

  /* Add shared object to chain */
  chain_add(so);

  /* Now that we have the stringtable, iterate a second time over dynamic
     section to get the names of the needed libraries. */
  char *lib_name = 0;
  long num = 0;
  for (dyn = so->dynamic_section; dyn->d_tag != DT_NULL; dyn++) { 
    switch (dyn->d_tag) {
      
    case DT_NEEDED:
      /* Get name of needed lib */
      lib_name = (char *)so->string_table + dyn->d_un.d_val;
      
#ifdef D_LOAD
      sl_printf("Found dependency in %s: %s\n", so->name, lib_name);
#endif

      /* Do not load the linux dynamic loader, because we replace it */
      if (sl_strncmp(lib_name, LINUX_LOADER,
                     sl_strnlen(LINUX_LOADER, MAX_LIB_NAME))==0) {
        so->deps_count--;
        continue;
      }

      /* Check if we already loaded it */
      dso *so_search = chain_search(lib_name); 
      if (so_search == 0) {
        /* Not already loaded, search for it */
        char *lib_path;
        long fd = search_lib(so, lib_name, &lib_path);
        if (fd == -1) {
          /* Not found, signal error (longjmp) if we load at runtime */
          if(rt_load)
            signal_error(0, lib_name, 0, "cannot open shared object file");

          /* Not at runtime -> fail */
          sl_printf("Error load_elf: lib %s not found.\n", lib_name);
          sl_exit(1);
        }

        /* Copy name */
        char *lname = sl_malloc(MAX_LIB_NAME);
        sl_strncpy(lname, lib_name, MAX_LIB_NAME);
        PROT_DATA(lname, MAX_LIB_NAME);

        /* Load it */
        dso *so_loaded = load_elf(so, lname, lib_path, fd, rt_load, 0, NULL);

        /* Increment local scope counter and add to direct deps */
        so->lscope_num += so_loaded->lscope_num;
        so->deps[num] = so_loaded;
        
      } else {
        
        /* Increment reference counter */
        UNPROT(so_search);
        so_search->ref_count++;
        PROT(so_search);

        /* Increment local scope counter and add to direct deps */
        so->lscope_num += so_search->lscope_num;
        so->deps[num] = so_search;        
      }
      
      num++;      
      so->lscope_num++;      
      break;
    }    
  }
  
  so->lscope_num++;
  
  /* Create local scope list. This has to be done in breadth-first order! */
  so->lscope = sl_malloc(so->lscope_num * sizeof(dso *));
  long j,k,l;
  i = 0;

  /* Add object itself */
  so->lscope[i++] = so;
  
  /* First add direct dependencies */
  for (l=0; l<so->deps_count; ++l) {
    so->lscope[i] = so->deps[l];
    ++i;
  }
  
  /* Now add deps recursively */
  for (l=0; l<so->deps_count; ++l) {
    for (k=0; k<so->deps[l]->lscope_num; ++k) {
      dso *dep = so->deps[l]->lscope[k];

      /* Check if already added */
      long found = 0;
      for (j=0; j<i; ++j) {
        if (so->lscope[j] == dep)
          found = 1;
      }

      if (found || !dep)
        continue;
      
      so->lscope[i] = dep;
      ++i;
    }
  }

  /* Initialize Global Offset Table */
  init_got(so);

#if defined(VERIFY_CFTX)
  /* Add object to dso chain if libdetox wants to check the control flow
     transfers */
  add_dso(so, (char *)file_map);
  
  #if defined(CALLBACK_MAIN_DETECTION) || defined(CALLBACK_DATA_SECTION_SEARCH)
  /* Right after loading the dso we need to detect the libc callbacks to main/__libc_csu_init */
  if(so->loader == NULL && dso_chain->next != 0) {
  /* This is the main executable. Get the immediate values pushed on the stack right before __libc_start_main is called. */

	  #if defined(CALLBACK_MAIN_DETECTION)
	  /* This is the main function callback detection hack!
			* - it tries to find the callback pointers passed to libc 
			* - and it adds them to the callback table so CFTX checks will pass */
	  long count = 0;
	  unsigned char *iptr;
	  unsigned long ptr;
	  
	  if(so->type == ET_EXEC)
		iptr = (unsigned char *)so->entry;
	  else /* PIE executable */
	    iptr = (unsigned char *)((unsigned long)so->base_addr + (unsigned long)so->entry);
	  
	  /* TODO: 48? Remove hardcoded value. */
	  while(count<48) {
		
		/* is it a push instruction with a 32bit immediate value? */
		if(*iptr == 0x68) {
				/* add the immediate value pushed to the stack */
				ptr = *((unsigned long*)(++iptr));
				fbt_add_callback(dso_chain->next, (void*)ptr);
				iptr+=4;
				count+=5;
		} else {
			iptr++;
			count++;
		}
		/* in case we reached NOPs we can just stop looking for the pushes */
		if(*iptr == 0x90) break;
	  }
	  #endif /* CALLBACK_MAIN_DETECTION */
	  
	  #if defined(CALLBACK_DATA_SECTION_SEARCH)
	  /* Some x* applications have global function pointers in their .data section.
	   * This are probably widget class objects and their members. It seems that there is no other
	   * way to detect these potential callbacks than scanning through the .data section. This has
	   * only to be done for prelinked executables as we will detect the other callbacks during relocation. */
	   unsigned long *dptr = so->data_addr;
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->data_addr+(unsigned long)so->data_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }
	  
	   /* go through GOT */
	   dptr = (unsigned long*)(so->got);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->got+(unsigned long)so->got_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }
	   
	   /* go through GOT.PLT */
	   dptr = (unsigned long*)(so->gotplt);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->gotplt+(unsigned long)so->gotplt_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }

	   /* go through rodata */
	   dptr = (unsigned long*)(so->dso->rodata);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->rodata+(unsigned long)so->dso->rodata_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	
	   
	   /* go through reldyn */
	   dptr = (unsigned long*)(so->dso->reldyn);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->reldyn+(unsigned long)so->dso->reldyn_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	
	   	   	   
	   /* go through relplt */
	   dptr = (unsigned long*)(so->dso->relplt);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->relplt+(unsigned long)so->dso->relplt_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	   
	  #endif /* CALLBACK_DATA_SECTION_SEARCH */
	  
  }
  #endif /* defined(CALLBACK_MAIN_DETECTION) || defined(CALLBACK_DATA_SECTION_SEARCH) */

  #if defined(CALLBACK_LIBRARIES_DATA_SECTION_SEARCH)
  if(so->loader != NULL) { /* it's a library! */

	   unsigned long *dptr = so->data_addr;
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->data_addr+(unsigned long)so->data_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }
	  
	   /* go through GOT */
	   dptr = (unsigned long*)(so->got);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->got+(unsigned long)so->got_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }
	   
	   /* go through GOT.PLT */
	   dptr = (unsigned long*)(so->gotplt);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->gotplt+(unsigned long)so->gotplt_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }

	   /* go through rodata */
	   dptr = (unsigned long*)(so->dso->rodata);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->rodata+(unsigned long)so->dso->rodata_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	
	   
	   /* go through reldyn */
	   dptr = (unsigned long*)(so->dso->reldyn);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->reldyn+(unsigned long)so->dso->reldyn_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	
	   	   	   
	   /* go through relplt */
	   dptr = (unsigned long*)(so->dso->relplt);
	   if(dptr) {
		  while((unsigned long)dptr < ((unsigned long)so->dso->relplt+(unsigned long)so->dso->relplt_size)) {
	   			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*dptr, so->text_addr, so->text_size)
				|| PTR_IN_REGION(*dptr, so->dso->init, so->dso->init_size)
				|| PTR_IN_REGION(*dptr, so->dso->fini, so->dso->fini_size)) {

				fbt_add_callback(so->dso, (void*)*dptr);
					
			 }
			 dptr++; /* increase by sizeof(unsigned long) = bytes */
		 }
	   }	  
  }
  
  #endif /* CALLBACK_LIBRARIES_DATA_SECTION_SEARCH */

#endif /* VERIFY_CFTX */

  /* All necessary information in memory -> unmap file */
  sl_munmap(file_map, file_info.st_size);
  sl_close(fd);
  
  /* Protect dependencies and local search scope */
  PROT_DATA(so->deps, so->deps_count*sizeof(dso *));
  PROT_DATA(so->lscope, so->lscope_num*sizeof(dso *));
  
  return so;
}


void unload_elf(dso *so) {
  /* Are we allowed to unload? */
  if (so->flags_1 & DF_1_NODELETE) {
    sl_printf("Error unload_elf: NODELETE flag set on %s.\n", so->name);
    sl_exit(1);
  }

#ifdef D_RLOAD
  sl_printf("Unloading %s (%p-%p)\n", so->path, so->text_addr, so->end_addr);
#endif

#ifdef SL_STATISTIC
  curr_loaded_dsos--;
#endif
  
  /* Call fini functions */
  so_fini(so);
  
  /* Remove it from chain and global scope */
  chain_delete(so);
  gscope_remove(so);

#if defined(VERIFY_CFTX)  
  /* Remove it from libdetox dso_chain */
  remove_dso(so);
#endif

  /* Free name and path */
  sl_free((void *)so->name, MAX_LIB_NAME);
  sl_free((void *)so->path, MAX_PATH_LEN);
  
  /* Free dep. and lsope lists */
  if (so->deps)
    sl_free(so->deps, so->deps_count * sizeof(dso *));
  if (so->lscope)
    sl_free(so->lscope, so->lscope_num * sizeof(dso *));

  /* Unmap text and data segments */
  unmap_segments(so);

  /* Free struct */
  sl_free(so, sizeof(dso));
}


void so_init(dso *so, long argc, char **argv, char **envp) {
  /* Check if already initialized */
  if (so->is_init)
    return;

  //sl_printf("Initializing %s\n", so->name);

  so->is_init = 1;

#ifdef MEMORY_PROTECTION
  /* All initialization finished, set object to readonly */
  so->readonly = 1;
  PROT(so);
#endif
    
  if (so->init != 0) {
#ifdef D_LOAD
    sl_printf("\nInvoking init function of %s (argc: %d argv: %p envp %p).\n",
              so->name, argc, argv, envp);
#endif
    /* First call init function */
    so->init(argc, argv, envp);
  }

  if (so->init_array != 0) {
#ifdef D_LOAD
    sl_printf("Invoking init array functions of %s.\n", so->name);
#endif
    /* Now call init functions in array */
    long i;
    for (i=0; i < so->init_array_sz; ++i) {
      ((void (*)(long, char**, char**))so->init_array[i]) (argc, argv, envp);
    }
  }
}


void so_fini(dso *so) {
  /* First call functions in fini array */
  if (so->init_array != 0) {
#ifdef D_LOAD
    sl_printf("Invoking init array functions of %s.\n", so->name);
#endif
    /* Array must be executed in reverse order */
    long i;
    for (i=so->fini_array_sz-1; i>=0; --i) {
      ((void (*)())so->fini_array[i]) ();
    }
  }

  /* Now call fini function */
  if (so->fini != 0) {
#ifdef D_LOAD
    sl_printf("Invoking fini function of %s.\n", so->name);
#endif
    so->fini();
  }
}
