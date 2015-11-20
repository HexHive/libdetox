/**
 * @file trueRTLD.c
 * Entry point to the shared library.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

#include "secureloader/sl_loader.h"
#include "secureloader/sl_libc.h"
#include "fastbt/fbt_llio.h"
#include <elf.h>

/* TODO: correctly pass information to loader -> executable already loaded so no need to load! */

/**
 * Entry point to the ELF RTLD. Prepare arguments and call sl_mainRTLD.
 * @param argp Argument pointer
 */
void _entry(/*long argc,*/ char *argp) {
  /* Get number of command line arguments */
  long argc;

  asm volatile (/*"push %%ebp ;"
				"mov %%esp, %%ebp ;"
				"push %%ebx ;"*/
				"movl 4(%%ebp), %%eax"
                : "=a" (argc));

  /* Get pointer to arguments and environment variables (environment variables
     lie after arguments on the stack) and call main */
  char **argv = &argp;
  char **envp = &argv[argc + 1];
  char **aux = &argv[argc + 1];
  Elf32_auxv_t *auxv;
  //Elf32_auxv_t *auxv2;
  
  while(*aux++ != NULL);
  auxv = (Elf32_auxv_t *)aux;
  
  /* Note: this is how to print the auxiliary vector. */
  /*auxv2 = auxv;
  for(; auxv2->a_type != AT_NULL; auxv2++) {
	    sl_printf("a_type a_val = %d %x\n", auxv2->a_type, (unsigned int)auxv2->a_un.a_val);
  }*/
	
  long ret = sl_mainRTLD(argc, argv, envp, auxv);
  
  sl_exit(ret);
}
