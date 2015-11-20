 /**
 * @file sl_map.c
 * Implements the loading of elf segments into memory.
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

#include <elf.h>
#include "sl_libc.h"
#include "sl_map.h"
#include "sl_macros.h"
#include "sl_datatypes.h"

/**
 * Converts ELF segment protection flags to
 * the appropriate mmap, mprotect flag.
 * @param e_prot ELF protection flag 
 * @return protection flag
 */
static long convert_prot(long e_prot) {
  long prot = 0;
  if (e_prot & PF_R)
    prot |= PROT_READ;
  if (e_prot & PF_W)
    prot |= PROT_WRITE;
  if (e_prot & PF_X)
    prot |= PROT_EXEC;
  return prot;
}

void set_page_protection (dso *so) {

  SET_PROT(so->text_addr, so->text_size, so->text_prot);

  SET_PROT(so->data_addr, so->data_size, so->data_prot);
 
  if (so->bss_size > 0) {
	SET_PROT(so->bss_addr, so->bss_size, so->data_prot);
  }
  
}

void map_segments (long fd, Elf32_Phdr *segs[2], Elf32_Half type, dso *so) {

  /* Adjust text segment addresses to page size */
  Elf32_Off text_offset = TRUNC_PAGE(segs[0]->p_offset);
  Elf32_Addr text_vaddr = TRUNC_PAGE(segs[0]->p_vaddr);  
  Elf32_Addr text_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_memsz);
  unsigned long mapsize = text_vlimit - text_vaddr;

  /* Executable has to be loaded at constant address */
  void *base_addr = 0;
  if (type == ET_EXEC) {
    base_addr = (void *)text_vaddr;
  }

  /* TODO: what if base address lies in already mapped area? E.g. where the loader resides? */

  /* Map text segment into memory */
  char *mapbase = sl_mmap(base_addr, mapsize, convert_prot(segs[0]->p_flags),
                          MAP_PRIVATE, fd, text_offset);
  if ((long)mapbase == -1) {
    sl_close(fd);
    sl_printf("Error map_segments: mapping of text segment failed.\n");
    sl_exit(1);
  }

  /* Adjust data segment addresses to page size */
  Elf32_Off data_offset = TRUNC_PAGE(segs[1]->p_offset);
  Elf32_Addr data_vaddr = TRUNC_PAGE(segs[1]->p_vaddr);
  Elf32_Addr data_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_filesz);
  void *data_addr = mapbase + (data_vaddr - text_vaddr);
  long data_prot = convert_prot(segs[1]->p_flags);

  /* Map data segment into memory */
  if ((long)sl_mmap(data_addr, data_vlimit - data_vaddr, data_prot,
                    MAP_PRIVATE | MAP_FIXED, fd, data_offset) == -1) {
    sl_close(fd);
    sl_printf("Error map_segments: mapping of data segment failed.\n");
    sl_exit(1);
  }
   
  /* Clear BSS part */
  Elf32_Addr clear_vaddr = segs[1]->p_vaddr + segs[1]->p_filesz;
  void *clear_addr = mapbase + (clear_vaddr - text_vaddr);
  void *clear_page = mapbase + (TRUNC_PAGE(clear_vaddr) - text_vaddr);
  unsigned long nclear = data_vlimit - clear_vaddr;

  if (nclear > 0) {
    /* Make sure the end of the segment is writable */
    if ((data_prot & PROT_WRITE) == 0 &&
        sl_mprotect(clear_page, PAGE_SIZE, data_prot|PROT_WRITE) == -1) {
      sl_printf("Error map_segments: mprotect on data segment failed.\n");
      sl_exit(1);
    }
    
    sl_memset(clear_addr, 0, nclear);

    /* Reset the data protection */
    if ((data_prot & PROT_WRITE) == 0) {
      sl_mprotect(clear_page, PAGE_SIZE, data_prot);
    }
  }
  
  /* Allocate remaining part of bss section */
  Elf32_Addr bss_vaddr = data_vlimit;
  Elf32_Addr bss_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_memsz);
  void *bss_addr = mapbase + (bss_vaddr - text_vaddr);
  if (bss_vlimit > bss_vaddr) {
    if ((long)sl_mmap(bss_addr, bss_vlimit - bss_vaddr, data_prot,
                      MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) == -1) {
      sl_printf("Error map_segments: mmap of bss segment failed.\n");
      sl_exit(1);
    }
    
  }
  
  /* Save important information */
  so->base_addr = (type == ET_EXEC) ? 0 : mapbase;
  so->text_addr = mapbase;
  so->text_size = mapsize;  
  so->data_addr = data_addr;
  so->data_size = data_vlimit - data_vaddr;
  so->bss_addr = bss_addr;
  so->bss_size = bss_vlimit - bss_vaddr;
  so->end_addr = bss_addr + so->bss_size;
  so->text_prot = convert_prot(segs[0]->p_flags);
  so->data_prot = data_prot;
  so->bss_prot = data_prot;
}

/* Verifies the kernel provided program header PT_LOAD entries and does the
 * segment mappings only if required. As the kernel already mapped the PT_LOAD
 * segments our RTLD should not map them again.
 */
void map_segments_RTLD (long fd, Elf32_Phdr *segs[2], Elf32_Half type, dso *so, Elf32_Phdr *segs_auxv[2]) {

  /* TODO: improve error handling ;) */
  if(segs[0]->p_offset != segs_auxv[0]->p_offset) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  if(segs[0]->p_vaddr != segs_auxv[0]->p_vaddr) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  if(segs[0]->p_memsz != segs_auxv[0]->p_memsz) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  
  if(segs[1]->p_offset != segs_auxv[1]->p_offset) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  if(segs[1]->p_vaddr != segs_auxv[1]->p_vaddr) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  if(segs[1]->p_memsz != segs_auxv[1]->p_memsz) {
	 sl_printf("map_segments_RTLD: difference in program headers found!\n");
	 sl_exit(1);
  }
  
  /* Adjust text segment addresses to page size */
  //Elf32_Off text_offset = TRUNC_PAGE(segs[0]->p_offset);
  Elf32_Addr text_vaddr = TRUNC_PAGE(segs[0]->p_vaddr);  
  Elf32_Addr text_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_memsz);
  unsigned long mapsize = text_vlimit - text_vaddr;
  
  /* Executable has to be loaded at constant address */
  void *base_addr = 0;
  if (type == ET_EXEC) {
    base_addr = (void *)text_vaddr;
  } else {
	 sl_printf("map_segments_RTLD: first program header entry is not ET_EXEC!\n");
	 sl_exit(1);
  }

  /* TODO: what if base address lies in already mapped area? E.g. where the loader resides? */

  /* Text segment already mapped */
  char *mapbase = base_addr;

  /* Adjust data segment addresses to page size */
  //Elf32_Off data_offset = TRUNC_PAGE(segs[1]->p_offset);
  Elf32_Addr data_vaddr = TRUNC_PAGE(segs[1]->p_vaddr);
  Elf32_Addr data_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_filesz);
  void *data_addr = mapbase + (data_vaddr - text_vaddr);
  //long data_prot = convert_prot(segs[1]->p_flags);
  
  /* Clear BSS part */
  //Elf32_Addr clear_vaddr = segs[1]->p_vaddr + segs[1]->p_filesz;
  //void *clear_addr = mapbase + (clear_vaddr - text_vaddr);
  //void *clear_page = mapbase + (TRUNC_PAGE(clear_vaddr) - text_vaddr);
  //unsigned long nclear = data_vlimit - clear_vaddr;
 
  /* Allocate remaining part of bss section */
  Elf32_Addr bss_vaddr = data_vlimit;
  Elf32_Addr bss_vlimit = ROUND_PAGE(segs[1]->p_vaddr + segs[1]->p_memsz);
  void *bss_addr = mapbase + (bss_vaddr - text_vaddr);
  
  /* Save important information */
  so->base_addr = (type == ET_EXEC) ? 0 : mapbase;
  so->text_addr = mapbase;
  so->text_size = mapsize;  
  so->data_addr = data_addr;
  so->data_size = data_vlimit - data_vaddr;
  so->bss_addr = bss_addr;
  so->bss_size = bss_vlimit - bss_vaddr;
  so->end_addr = bss_addr + so->bss_size;
  so->text_prot = convert_prot(segs[0]->p_flags);
  so->data_prot = convert_prot(segs[1]->p_flags);
  so->bss_prot = convert_prot(segs[1]->p_flags);
}

void unmap_segments(dso *so) {
  /* Unmap text section */
  if ((long)sl_munmap(so->text_addr, so->text_size) == -1) {
    sl_printf("Error unmap_segments: unmapping text segment failed.\n");
    sl_exit(1);
  }
  
  /* Unmap data section */
  if ((long)sl_munmap(so->data_addr, so->data_size) == -1) {
    sl_printf("Error unmap_segments: unmapping data segment failed.\n");
    sl_exit(1);
  }
    
  /* Unmap bss section */
  if (so->bss_size) {
    sl_munmap(so->bss_addr, so->bss_size);
  }
}
