/**
 * @file sl_reloc.c
 * Implements the relocating of ELF files.
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

#include "sl_reloc.h"
#include "sl_libc.h"
#include "sl_macros.h"
#include "sl_sym_lookup.h"
#include "sl_datatypes.h"
#include "sl_libsearch.h"
#include "sl_map.h"
#include "fbt_libc.h"
#include "fbt_datatypes.h"

extern dso *so_chain;

#ifdef SL_STATISTIC
/* Number of relocations */
long num_relocs = 0;
#endif


void do_relocations() {
  dso *so;

  /* Do this in reverse order because of COPY relocations! */
  for (so = so_chain; so->next != 0; so = so->next);

  for (; so != 0; so = so->prev) {
    relocate(so, 0);
    /* TODO: this is just for testing! */
    //relocate_plt(so);
  }
}


void relocate(dso *so, unsigned char rt_load) {
#ifdef D_REL
  sl_printf("\nProcessing relocations for %s:\n", so->name);
#endif

  /* Resolve address and relocation address */
  Elf32_Addr *addr = 0;
  unsigned long *reloc = 0;
  //void *value;

  /* Get relocation table */
  Elf32_Rel *rel = so->rel;
  if (rel == 0)
    return;
        
#ifdef SL_STATISTIC
  num_relocs += so->relsz;
#endif

	/* Make text writable for relocations */
	SET_PROT(so->text_addr, so->text_size-1, so->text_prot|PROT_WRITE);
  
  /* Iterate over relocations */
  long i;
  for (i = 0; i < so->relsz; ++i, ++rel) {
    /* Get relocation address */
    reloc = (unsigned long *) BYTE_STEP(so->base_addr, rel->r_offset);

    /* Type and symbol table index */
    long type = ELF32_R_TYPE(rel->r_info);
    long symtab_index = ELF32_R_SYM(rel->r_info);

    Elf32_Sym *sym = 0;
    Elf32_Sym *sym_found = 0;
    dso *so_found = 0;
    
    if (symtab_index) {
      /* Get symbol from symbol_table and name */
      sym = so->symbol_table + symtab_index;
      const char *name = so->string_table + sym->st_name;

      /* Symbol versioning */
      struct version_info *version = 0;
      if (so->versym != 0) {
        /* DSO defines versions, get version index of symbol */
        Elf32_Half index = so->versym[symtab_index] & BITMASK_LOW15;
        /* Get version_info entry */
        version = &so->versions[index];
        if (version->hash == 0)
          version = 0;
      }
        
#ifdef D_REL
      sl_printf("\nFound relocation of type %d for symbol \"%s\" with type %d.\n", type, name, ELF32_ST_TYPE(sym->st_info));
#endif
      
      /* Resolve the symbol */
      addr = resolve_symbol(so, name, version, 1, (type==R_386_COPY),
                            &sym_found, &so_found, NULL);

      /* Check result */
      if (addr == 0 && ELF32_ST_BIND(sym->st_info) == STB_GLOBAL &&
          ELF32_ST_TYPE(sym->st_info)!= STT_TLS) {
        sl_printf("Error relocate: symbol \"%s\" for so %s not found.\n", name,
                  so->name);
        sl_exit(1);
      }
    } else {
#ifdef D_REL
      sl_printf("Found relocation of type %d\n", type);
#endif
    }

	/* TODO: verify all the signed/unsigned relocation pitfalls 
	 * One bug for sharedlibPIE test case was fixed in case of
	 * R_386_PC32 relocations the relocation has to be signed!
	 * */

    /* Relocate */
    switch(type) {
      
    case R_386_NONE:
      break;
      
    case R_386_32:
      /* Non-GOT reference to a symbol defined in another library */
	  *reloc += (unsigned long)addr;
      break;
      
    case R_386_PC32:
      /* Same as R_386_32 but add PC relative offset */
      *reloc += (signed long)((signed long)addr - (signed long)reloc);
	  
      break;
      
    case R_386_COPY:
      /* Move initialized data from library into app data space */
      sl_memcpy (reloc, (void *) addr, sym->st_size);
      break;
      
    case R_386_GLOB_DAT:
      /* Got reference to a symbol defined in another library */
      *reloc = (unsigned long) addr;
      break;
      
    case R_386_RELATIVE:
      /* For relocatable data references (locally defined static data) */
      *reloc = BYTE_STEP(so->base_addr, *reloc);
		
		#if defined(VERIFY_CFTX) && defined(CALLBACK_RELOC_DETECTION)
			/* check if the obtained address points to executable memory (potential callback target) */
			if (PTR_IN_REGION(*reloc, so->text_addr, so->text_size)
						/*|| PTR_IN_REGION(*reloc, so->dso->init_, so->dso->init_size)
						|| PTR_IN_REGION(*reloc, so->dso->fini, so->dso->fini_size)*/) {
				fbt_add_callback(so->dso, (void*)*reloc);
			 }
		#endif
		
      break;
      
    case R_386_TLS_DTPOFF32:
      /* General Dynamic: We need to calculate the TLS offset relative to the
         TLS block. This is just the symbol value. */
      *reloc = sym_found->st_value;
      break;
      
    case R_386_TLS_DTPMOD32:
      /* General Dynamic and Local Dynamic: We need to calculate the module ID
         of the object containing the symbol. */
      if (so_found != 0) {
        /* Destination is in another DSO's TLS block */
        *reloc = so_found->tls_modid;
      } else {
        /* Destination is in our own TLS block */
        *reloc = so->tls_modid;
      }
      break;
      
    case R_386_TLS_DESC:
      /* Not supported */
      sl_printf("Error: relocation R_386_TLS_DESC not implemented\n");
      sl_exit(1);
      break;
      
    case R_386_TLS_TPOFF32:
      /* Initial Executable: Reloc points to an entry in the GOT with a
         _positive_ TLS offset relative to static TLS block. It has to be
         relative to the Thread Pointer. So _add_ TLS offset. */
      if (rt_load) {
        /* We do not support static TLS in DSOs loaded at runtime */
        sl_printf("Error: static TLS in dynamic loaded objects\n");
        sl_exit(1);
      }

      if (sym != 0) {
        /* Destination is in another DSO's TLS block, add TLS offset to
           offset relative to the TLS block of this DSO */
        *reloc = -sym_found->st_value + so_found->tls_offset;
      } else {
        /* Destination is in our own TLS block, just add TLS offset */
        *reloc += so->tls_offset;
      }
      break;
      
    case R_386_TLS_TPOFF:
      /* Initial Executable: Reloc points to an entry in the GOT with a
         _negative_ TLS offset relative to static TLS block. It has to be
         relative to the Thread Pointer. So _subtract_ TLS offset. */
      if (rt_load && sym == 0) {
        /* We do not support static TLS in DSOs loaded at runtime */
        sl_printf("Error: static TLS in DSO loaded at runtime.\n");
        sl_exit(1);
		//struct tls_index tlsind;
		//tlsind.ti_module = so_found->tls_modid;
		//tlsind.ti_offset = so_found->tls_offset;
        //*reloc = tls_get_addr(&tlsind);
        //*reloc = sym_found->st_value - so_found->tls_offset;
        //break;
      }
      
      if (sym != 0) {
        /* Destination is in another DSO's TLS block, subtract TLS offset from
           offset relative to the TLS block of this DSO */ 
        *reloc = sym_found->st_value - so_found->tls_offset;
      } else {
        /* Destination is in our own TLS block, just subtract TLS offset */
        *reloc -= so->tls_offset;
      }
      break;
      
    /*case R_386_IRELATIVE:
      sl_printf("\nR_386_IRELATIVE %s:\n", so->name);
      value = BYTE_STEP(so->base_addr, *reloc);
	  value = ((void *(*) (void))(value))();
      *reloc = value;*/
	  /*value = map->l_addr + reloc->r_addend;
	  value = ((Elf32_Addr (*) (void)) value) ();
	  value = ((void *(*) (void))(value))();
	  *reloc_addr = value;*/
	  /*break;*/
	  
    default:
      sl_printf("Error: unhandled relocation entry of type %d.\n", type);
      sl_exit(1);
    }
    
  }

    //if(so->ispie) relocate_pie_plt(so);
    
    /* Set correct protection */
	set_page_protection (so);
	
#ifdef MEMORY_PROTECTION
  /* Set .dynamic and .got segments to readonly (plt is already RX). This is
     only possible if a PT_GNU_RELRO program segment exists, because otherwise
     they share a page with the data section. */
  if (so->relro) {
    char *rstart = (char *)TRUNC_PAGE(so->relro);
    char *rend = (char *)TRUNC_PAGE(so->relro + so->relro_size);
    if (rstart != rend) {
      if(sl_mprotect(rstart, rend-rstart, PROT_READ) == -1) {
        sl_printf("Error init_got: memory protection failed.\n");
        sl_exit(1);
      }      
    } 
  }
#endif
  
}


void relocate_plt(dso *so) {
  /* Resolve address and relocation address */
  Elf32_Addr *addr = 0;
  unsigned long *reloc = 0;
  //void *value;
  
  /* Get relocation table */
  Elf32_Rel *rel = so->pltrel;
  if (rel == 0)
    return;

#ifdef SL_STATISTIC
  num_relocs += so->pltrelsz;
#endif

  /* Iterate over relocations */
  long i;
  for (i = 0; i < so->pltrelsz; ++i, ++rel) {
    /* Get relocation address */
    reloc = (unsigned long *) BYTE_STEP(so->base_addr, rel->r_offset);
    
    /* Type and symbol table index */
    //long type = ELF32_R_TYPE(rel->r_info);
    /* Symbol table index */
    long symtab_index = ELF32_R_SYM(rel->r_info);

    Elf32_Sym *sym = 0;
    Elf32_Sym *sym_found = 0;
    dso *so_found = 0;
   
    if (symtab_index) {
      /* Get symbol from symbol_table and name */
      sym = so->symbol_table + symtab_index;
      const char *name = so->string_table + sym->st_name;
      
      /* Symbol versioning */
      struct version_info *version = 0;
      if (so->versym != 0) {
        /* DSO defines versions, get version index of symbol */
        Elf32_Half index = so->versym[symtab_index] & BITMASK_LOW15;
        /* Get version_info entry */
        version = &so->versions[index];
        if (version->hash == 0)
          version = 0;
      }

      /* Resolve symbol */
      addr = resolve_symbol(so, name, version, 1, 0, &sym_found, &so_found, NULL);

      /* Check result */
      if (addr == 0 && ELF32_ST_BIND(sym->st_info) == STB_GLOBAL &&
          ELF32_ST_TYPE(sym->st_info)!= STT_TLS) {
        sl_printf("Error relocate_plt: symbol \"%s\" for so %s not found.\n",
                  name, so->name);
        sl_exit(1);
      }

      /* Relocate */
      *reloc = (unsigned long) addr;
    } /*else {

	  if(type == R_386_IRELATIVE) {
		  value = BYTE_STEP(so->base_addr, *reloc);
		  //value = ((void *(*) (void))(value))();
		  //reloc = value;
		  value = map->l_addr + reloc->r_addend;
		  value = ((Elf32_Addr (*) (void)) value) ();
		  value = ((void *(*) (void))(value))();
		  *reloc_addr = value;
		}
	  
	}*/
	
  }    
}

//void relocate_pie_plt(dso *so) {
  ///* Resolve address and relocation address */
  //Elf32_Addr *addr = 0;
  //unsigned long *reloc = 0;

  ///* Get relocation table */
  //Elf32_Rel *rel = so->pltrel;
  //if (rel == 0)
    //return;

  ///* Iterate over relocations */
  //long i;
  //for (i = 0; i < so->pltrelsz; ++i, ++rel) {
    ///* Get relocation address */
    //reloc = (unsigned long *) BYTE_STEP(so->base_addr, rel->r_offset);
    ///* Relocate */
    //reloc = so->base_addr + *reloc;
  //}    
//}
