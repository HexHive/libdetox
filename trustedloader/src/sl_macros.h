/**
 * @file sl_macros.h
 * Defines some useful macros for the secuLoader.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
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

#ifndef SL_MACROS_H
#define SL_MACROS_H

/* Memory protection macros */
#ifdef MEMORY_PROTECTION

/* Set data to readonly */
#define PROT_DATA(data, size) if (size) sl_mprotect((void *)(data), size, PROT_READ)

/* Set struct shared_object to readonly */
#define PROT(so) if(so && so->readonly) sl_mprotect(so, sizeof(struct shared_object), PROT_READ) 

/* Set data to read/write */
#define UNPROT_DATA(data, size) if (size) sl_mprotect((void *)(data), size, PROT_READ|PROT_WRITE)

/* Set struct shared_object to read/write */
#define UNPROT(so) if (so) sl_mprotect(so, sizeof(struct shared_object), PROT_READ|PROT_WRITE)

#else

/* No memory protection, just ignore */
#define PROT_DATA(data, size)
#define UNPROT_DATA(data, size)
#define PROT(so)
#define UNPROT(so)

#endif /* MEMORY_PROTECTION */

/* GNU IFUNC extension (not defined by older libc versions) */
#ifndef STT_GNU_IFUNC
#define STT_GNU_IFUNC (10)
#endif

/* Access to global loader-specific data */
#define GL(elem) (rtld_glob->elem)
#define GL_RO(elem) (rtld_glob_ro->elem)

/* Different calling convenction which uses registers to pass arguments
   instead of the stack */
#define REG_ARGS(num) __attribute__ ((regparm (num)))

/* Standard file descriptors */
#define STDIN_FILENO 0    /* Standard input */
#define STDOUT_FILENO 1   /* Standard output */
#define STDERR_FILENO 2   /* Standard error output */

/* Symbol versioning masks */
#define BITMASK_LOW15 0x7fff  /* Get lower 15 bits */
#define BITMASK_16 0x8000     /* Get bit 16 */

/* Maximum symbol name length */ 
#define MAX_SYM_NAME 512
#define MAX_VERS_LEN 512
#define MAX_PATH_LEN 512
#define MAX_LIB_NAME 512

/* The page size */
#define PAGE_SIZE (0x1000)
#define PAGE_MASK (PAGE_SIZE - 1)

/* dl_open modes */
#define RTLD_LAZY 0x00001       /* Do lazy binding. */
#define RTLD_NOW 0x00002        /* Do eager binding.  */
#define RTLD_GLOBAL 0x00100     /* Put object in global scope. */
#define RTLD_NODELETE 0x01000   /* Do not delete object. */
#define RTLD_NOLOAD 0x00004     /* Do not load object. */
#define RTLD_DEEPBIND 0x00008   /* Put ahead of global scope. */

/* Round/trunc value to page size */ 
#define ROUND_PAGE(x) ((((long)x) + PAGE_MASK) & ~PAGE_MASK)
#define TRUNC_PAGE(x) (((long)x) & ~PAGE_MASK)

/* Add "step" bytes to pointer */
#define BYTE_STEP(pointer, step) ((unsigned long)((char *)(pointer) + (int)(step)))

/* Useful numeric macros */
#define ROUND(a,b) ((((a)+((b)-1)) / (b))*(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)>(b)?(b):(a))
#define ABS(a) ((a>>31)^(a+(a>>31)))

/* Memory protection flags */
#define PROT_NONE 0x0   /* Memory cannot be accessed */
#define PROT_READ 0x1   /* Memory can be read */
#define PROT_WRITE 0x2  /* Memory can be written to */
#define PROT_EXEC 0x4   /* Memoryy can be executed */

/* mmap syscall flags */
#define MAP_SHARED 0x01       /* Changes are shared */
#define MAP_PRIVATE 0x02      /* Changes are private */
#define MAP_FIXED 0x10        /* Use given address exactly */
#define MAP_ANONYMOUS 0x20   

/* File permissions */
#define O_RDONLY 0    /* Readonly */
#define O_WRONLY 1    /* Writeonly */
#define O_RDWR 2      /* Read/Write */

/* Signals */
#define SIGTERM 15
#define SIGKILL 9

/* Syscall numbers */
#define SYSCALL_EXIT 1
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_OPEN 5
#define SYSCALL_CLOSE 6
#define SYSCALL_CREAT 8
#define SYSCALL_KILL 37
#define SYSCALL_READLINK 85
#define SYSCALL_MMAP 90
#define SYSCALL_MUNMAP 91
#define SYSCALL_FSTAT 108
#define SYSCALL_MPROTECT 125
#define SYSCALL_SET_THREAD_AREA 243

/* Symbol lookup flags */
#define DL_LOOKUP_ADD_DEPENDENCY  1
#define DL_LOOKUP_RETURN_NEWEST  2
#define DL_LOOKUP_GSCOPE_LOCK  4

/* TLS specific macros */
#define TLS_STATIC_SURPLUS 64 + DL_NNS * 100
#define TLS_TCB_SIZE sizeof (thread_control_block)
#define TLS_TCB_ALIGN 16

/* Number of additional entries in the slotinfo array */
#define TLS_SLOTINFO_SURPLUS (62) 

/* Number of additional slots in the dtv */
#define DTV_SURPLUS (14)

/* Marks DTS as unallocated */
#define TLS_DTV_UNALLOCATED ((void *) -1l)

/* Get the Dynamic Thread Vector (DTV) */
#define GET_DTV(dtv)  \
asm volatile ("movl %%gs:4,%0" \
     : "=q" (dtv) \
     );

/* Get the Thread Control Block (TCB) */
#define GET_TCB(tcb)  \
asm volatile ("movl %%gs:0,%0" \
     : "=q" (tcb) \
     );


/* The following is for the printing functions from libdetox */

/**
 * small buffer size for the low-level printing functions.
 * The output string cannot be longer than BUFSIZE_S -1. Used for decimal and
 * hexadecimal numbers.
 */
#define BUFSIZE_S 16

/**
 * large buffer size for the low-level printing functions.
 * The output string cannot be longer than BUFSIZE_L -1. Used for formatted
 * strings.
 */
#define BUFSIZE_L 512

#endif
