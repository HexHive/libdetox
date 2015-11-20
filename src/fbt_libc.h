/**
 * This file contains reimplementations of some library functions
 * to remove dependencies on these libs.
 *
 * Copyright (c) 2010 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
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
#define PROT_READ       0x1             /* Page can be read.  */
#define PROT_WRITE      0x2             /* Page can be written.  */
#define PROT_EXEC       0x4             /* Page can be executed.  */
#define PROT_NONE       0x0             /* Page can not be accessed.  */

#define MAP_SHARED      0x01            /* Share changes.  */
#define MAP_PRIVATE     0x02            /* Changes are private.  */
#define MAP_ANONYMOUS   0x20            /* Don't use a file.  */

#define MAP_FAILED      ((void *) -1)

/* Syscall numbers needed here */
#define SYS_exit    1
#define SYS_write   4
#define SYS_open    5
#define SYS_close   6
#define SYS_lseek  19
#define SYS_mmap   90
#define SYS_munmap 91
#define SYS_mprotect 0x7d

/* functions from string.h */
void *fbt_memset(void *s, const char c, int n);
void *fbt_memcpy(void *dest, const void *src, int n);
char *fbt_strncpy(char *dest, const char *src, int n);
char *fbt_strcpy(char *dest, const char *src);
int fbt_strnlen(const char *s, int maxlen);
int fbt_strcmp(const char *s1, const char *s2);
int fbt_strncmp(const char *s1, const char *s2, int n);

#define ENTER_KERNEL_INT "int $0x80\n\t"
#define ENTER_KERNEL_SYSENTER "call *%%gs:0x10\n\t"

#define ENTER_KERNEL ENTER_KERNEL_SYSENTER

/* kill program */
#define fbt_suicide()			   \
    __asm__ volatile("pushl %%ebx\n\t"	   \
		     "movl $139,%%ebx\n\t" \
		     ENTER_KERNEL	   \
		     "popl %%ebx"	   \
		     :			   \
		     : "a"(SYS_exit)	   \
		     : "memory" )

#define fbt_suicide_str(str)	do {		\
    fllwrite(STDOUT_FILENO, str);		\
    fbt_suicide(); } while (0)

/* file management and I/O */
#define fbt_open(pathname, flags, mode, res)				\
    __asm__ volatile("pushl %%ebx\n\t"					\
		     "movl %2,%%ebx\n\t"				\
		     ENTER_KERNEL					\
		     "popl %%ebx"					\
		     : "=a"(res)					\
		     : "0"(SYS_open), "g"(pathname), "c"(flags), "d"(mode) \
		     : "memory" )

#define fbt_write(fd, buf, count, res)					\
    __asm__ volatile("pushl %%ebx\n\t"					\
		     "movl %2,%%ebx\n\t"				\
		     ENTER_KERNEL					\
		     "popl %%ebx"					\
		     : "=a"(res)					\
		     : "0"(SYS_write), "g"(fd), "c"(buf), "d"(count)	\
		     : "memory" )

#define fbt_close(fd, res)			\
    __asm__ volatile("pushl %%ebx\n\t"		\
		     "movl %2,%%ebx\n\t"	\
		     ENTER_KERNEL		\
		     "popl %%ebx"		\
		     : "=a"(res)		\
		     : "0"(SYS_close), "g"(fd)	\
		     : "memory" )

#define fbt_lseek(fd, offset, whence, res)				\
    __asm__ volatile("pushl %%ebx\n\t"					\
		     "movl %2,%%ebx\n\t"				\
		     ENTER_KERNEL					\
		     "popl %%ebx"					\
		     : "=a"(res)					\
		     : "0"(SYS_lseek), "g"(fd), "c"(offset), "d"(whence) \
		     : "memory" )


/* memory mapping */
#define fbt_mmap(addr, length, prot, flags, fd, offset, res)		\
    __asm__ volatile("pushl %%ebx\n\t"					\
		     "pushl %2\n\t"					\
		     "pushl %3\n\t"					\
		     "pushl %4\n\t"					\
		     "pushl %5\n\t"					\
		     "pushl %6\n\t"					\
		     "pushl %7\n\t"					\
		     "movl %%esp, %%ebx\n\t"				\
		     ENTER_KERNEL					\
		     "addl $24, %%esp\n\t"				\
		     "popl %%ebx"					\
		     : "=a"(res)					\
		     : "0"(SYS_mmap), "g"(offset), "g"(fd), "g"(flags), \
                       "g"(prot), "g"(length), "g"(addr)                \
		     : "memory")

#define fbt_munmap(addr, length, res)				\
    __asm__ volatile("pushl %%ebx\n\t"				\
		     "movl %2,%%ebx\n\t"			\
		     ENTER_KERNEL				\
		     "popl %%ebx"				\
		     : "=a"(res)				\
		     : "0"(SYS_munmap), "g"(addr), "c"(length)	\
		     : "memory" )

#define fbt_mprotect(addr, len, prot, res)				\
    __asm__ volatile("pushl %%ebx\n\t"					\
		     "movl %2,%%ebx\n\t"				\
		     ENTER_KERNEL					\
		     "popl %%ebx"					\
		     : "=a"(res)					\
		     : "0"(SYS_mprotect), "g"(addr), "c"(len), "d"(prot) \
		     : "memory" )
