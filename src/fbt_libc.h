/**
 * @file fbt_libc.h
 * This file contains reimplementations of some library functions
 * to remove dependencies on these libs.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-03-18 19:09:01 +0100 (Fri, 18 Mar 2011) $
 * $LastChangedDate: 2011-03-18 19:09:01 +0100 (Fri, 18 Mar 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 428 $
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
#ifndef FBT_LIBC_H
#define FBT_LIBC_H

/* include ugliness of system call definition */
#include "fbt_syscalls_impl.h"

/** definition of NULL */
#define NULL ((void *)0)

/* copied from linux/stat.h to remove deps */
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IROTH 00004
#define S_IWOTH 00002

#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define O_CREAT         00000100
#define O_TRUNC         00001000

#define SEEK_END 2

/* taked from sys/mman.h */
#define PROT_READ       0x1             /* Page can be read. */
#define PROT_WRITE      0x2             /* Page can be written. */
#define PROT_EXEC       0x4             /* Page can be executed. */
#define PROT_NONE       0x0             /* Page can not be accessed. */

#define MAP_SHARED      0x01            /* Share changes. */
#define MAP_PRIVATE     0x02            /* Changes are private. */
#define MAP_FIXED       0x10            /* Map memory to a fixed address */
#define MAP_ANONYMOUS   0x20            /* Don't use a file. */

#define MAP_FAILED      ((void *)-1)

/* taken from linux/sched.h */
#define CLONE_VM        0x00000100      /* VM shared between processes */
#define CLONE_FS        0x00000200      /* fs info shared between procs */
#define CLONE_FILES     0x00000400      /* open files shared between proc */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers and blocked
                                           signals shared */
#define CLONE_PTRACE    0x00002000      /* set if we want to let tracing
                                           continue on the child too */
  
/* taken from asm/signal.h */
#define SIG_DFL (0)     /* default signal handling */
#define SIG_IGN (1)     /* ignore signal */
#define SIG_ERR (-1)    /* error return from signal */

/**
 * Local implementation of memcpy. Copies one memory region to another address.
 * @param dest Destination memory region
 * @param src Source region
 * @param n number of bytes that are copied
 * @return Pointer to the target region.
 */
void *fbt_memcpy(void *dest, const void *src, int n);

/**
 * Local implementation of strncpy. Copies one string into another buffer.
 * @param dest Destination memory region.
 * @param src Source region.
 * @param n Max. number of copied characters.
 * @return Pointer to the copied string.
 */
char *fbt_strncpy(char *dest, const char *src, int n);

/**
 * Local implementation of strnlen. Calculates the length of a string.
 * @param s Source string
 * @param maxlen Maximum length
 * @return Length of the string or maxlen.
 */
int fbt_strnlen(const char *s, int maxlen);

/**
 * Local implementation of strncmp. Compares two strings.
 * @param s1 First string.
 * @param s2 Second string.
 * @param n Maximum length that is compared.
 * @return Returns 0 if they are equal.
 */
int fbt_strncmp(const char *s1, const char *s2, int n);

/** useful macro that checks if a pointer is in a certain region */
#define PTR_IN_REGION(ptr, regionptr, size)             \
  ((ulong_t)(ptr) >= (ulong_t)(regionptr) &&            \
   (ulong_t)(ptr) <= ((ulong_t)(regionptr)+(size)))

/** kill program */
#define fbt_suicide_str(str)	do {		\
    fllwrite(STDOUT_FILENO, str);		\
    fbt_suicide(255); } while (0)

#endif  /* FBT_LIBC_H */
