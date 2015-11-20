/**
 * @file sl_libc.c
 * Reimplementations of some useful libc functions.
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

#include "sl_libc.h"
#include "sl_macros.h"
#include "sl_datatypes.h"
#include "fbt_libc.h"

#if !defined(DEBUG)
#define DEBUG
#include "fbt_llio.h"
#undef DEBUG
#else
#include "fbt_llio.h"
#endif

/**
 * Checks if an error occurred while executing the syscall.
 * @param ret Return value of syscall
 * @return return value of syscall on success, -1 otherwise
 */
static long check_return (long ret) {
  if (((long)ret < 0) && ((long)ret > -4096)) {
    return -1;
  } else {
    return ret;
  }
}


void sl_exit(long mode) {
  /* This stops the program immediately */
//  asm volatile("hlt");

  /* We use signal SIGTERM instead of exit because there
     may be several threads */

  asm volatile("int  $0x80"
               :  
               : "a" (SYSCALL_EXIT), "b" (0)
               );

}


unsigned long sl_read(long fd, void *buf, unsigned long count) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_READ), "b" (fd), "c" (buf), "d" (count)
               : "memory");
  return check_return(ret);
}


long sl_creat(const char *filename, long mode) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_CREAT), "b" (filename), "c" (mode)
               : "memory");
  return check_return(ret);
}


long sl_open(const char *filename, long flags) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_OPEN), "b" (filename), "c" (flags), "d" (0)
               : "memory");
  return check_return(ret);
}


long sl_close(long fd) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_CLOSE), "b" (fd)
               : "memory");
  return check_return(ret);
}


void * sl_mmap(void *addr, unsigned long len, long prot, long flags, long fd,
               unsigned long offset) {
  void *ret;

  asm volatile("pushl %%ebx;"
               "pushl %2;"
               "pushl %3;"
               "pushl %4;"
               "pushl %5;"
               "pushl %6;"
               "pushl %7;"
               "movl %%esp, %%ebx;"
               "int  $0x80;"
               "addl $24, %%esp;"
               "popl %%ebx"
               : "=a"(ret)
               : "0"(SYSCALL_MMAP), "g"(offset), "g"(fd), "g"(flags),
               "g"(prot), "g"(len), "g"(addr) 
               : "memory");
  return (void *)check_return((long)ret);
}


long sl_munmap(void *start, unsigned long length) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_MUNMAP), "b" (start), "c" (length)
               : "memory");
  return check_return(ret);
}


void *sl_malloc(unsigned long size) {
  return sl_mmap(0, ROUND_PAGE(size), PROT_READ|PROT_WRITE,
                 MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}


void sl_free(void *ptr, unsigned long size) {
  sl_munmap(ptr, ROUND_PAGE(size));
}


void *sl_calloc(unsigned long nmemb, unsigned long size) {
  void *ret = sl_malloc(nmemb*size);
  if ((long)ret == -1)
    return (void *)-1;
  sl_memset(ret, 0, nmemb*size);
  return ret;
} 


long sl_mprotect(const void *addr, unsigned long len, long prot) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_MPROTECT), "b" (addr), "c" (len), "d" (prot)
               : "memory");
  return check_return(ret);
}


void *sl_memcpy(void *dest, const void *src, long n) {
  long i;
  char *destination = (char*)dest;
  char *source = (char*)src;
  for (i=0; i<n; i++) {
    destination[i] = source[i];
  }
  return (void*)dest;
}


void *sl_memset(void *ptr, long value, unsigned long num) {
  unsigned char *dest = (unsigned char *)ptr;
  while (num-- > 0)
    *dest++ = value;

  return ptr;
}


long sl_set_thread_area(struct user_desc *u_info) {
  long ret;
  asm volatile("int $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_SET_THREAD_AREA), "b" (u_info)
               : "memory"
               );
  return check_return(ret);
}


long sl_fstat(long fd, struct kernel_stat *buf) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_FSTAT), "b" (fd), "c" (buf)
               : "memory");
  return check_return(ret);
}


long sl_readlink(const char *path, char *buf, long bufsize) {
  long ret;
  asm volatile("int  $0x80"
               : "=a" (ret)
               : "0" (SYSCALL_READLINK), "b" (path), "c" (buf), "d"(bufsize)
               : "memory");
  return check_return(ret);
}

/* sl_setjmp */
asm ("sl_setjmp:;"
     "movl (%esp), %ecx;"    /* return address -> ecx */
     "movl 4(%esp), %edx;"   /* jump_buffer -> eax */
     "movl %ebx, 0(%edx);"   /* Save registers */
     "movl %esi, 4(%edx);"
     "movl %edi, 8(%edx);"   
     "movl %ebp, 12(%edx);"   
     "movl %esp, 16(%edx);"   
     "movl %ecx, 20(%edx);"  /* Save return address */
     "xorl %eax, %eax;"
     "jmpl *%ecx;"
     );

/* sl_setjmp */
asm ("sl_longjmp:;"
     "leal 4(%esp), %esp;"  /* return address not needed */
     "popl %edx;"
     "popl %eax;"
     "movl 0(%edx), %ebx;"  /* Restore registers */
     "movl 4(%edx), %esi;"
     "movl 8(%edx), %edi;"
     "movl 12(%edx), %ebp;"
     "movl 16(%edx), %esp;"
     "jmpl *20(%edx);"
     );


char *sl_strchr(const char *s, long character) {
  while (*s != (char)character)
    if (!*s++)
      return 0;
  return (char *)s;
}


char *sl_strrchr(const char *s, long character) {
  char *ret = 0;

  do {
    if (*s == character)
      ret = (char *) s;
  } while (*s++);

  return ret;
}


/*
 * ########################################### 
 *  THE FOLLOWING FUNCTIONS ARE FROM LIBDETOX 
 * ########################################### 
 */


char *sl_strncpy(char *dest, const char *src, long n) {
  return fbt_strncpy(dest, src, n);
}


char *sl_strncat(char *dest, const char *src, unsigned long num) {
  sl_strncpy(dest + sl_strnlen(dest, 1024), src, num);
  return dest;
}


long sl_strnlen(const char *s, long maxlen) {
  return fbt_strnlen(s, maxlen);
}


long sl_strncmp(const char *s1, const char *s2, long n) {
  return fbt_strncmp(s1, s2, n);
}


long sl_printf_ext(long fd, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  long ret = fllprintfva(fd, format, ap);
  va_end(ap);
  return ret;
}

