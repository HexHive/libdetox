/**
 * @file sl_libc.h
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

#ifndef SL_LIBC_H
#define SL_LIBC_H

#include <stdarg.h>
#include "sl_datatypes.h"

#define sl_printf(...) (sl_printf_ext(1, __VA_ARGS__))


void sl_exit(long mode);

unsigned long sl_read(long fd, void *buf, unsigned long count);

unsigned long sl_write(long fd, const void *buf, unsigned long count);

long sl_creat(const char *filename, long mode);

long sl_open(const char *filename, long flags);

long sl_close(long fd);

void *sl_mmap(void *addr, unsigned long len, long prot, long flags, long fd,
              unsigned long offset);

long sl_munmap(void *start, unsigned long length);

void *sl_malloc(unsigned long size);

long sl_set_thread_area(struct user_desc *u_info);

void sl_free(void *ptr, unsigned long size);

void *sl_calloc(unsigned long nmemb, unsigned long size);

long sl_mprotect(const void *addr, unsigned long len, long prot);

long sl_fstat(long fd, struct kernel_stat *buf);

long sl_readlink(const char *path, char *buf, long bufsize);

long sl_setjmp(struct jump_buffer *buf);

void sl_longjmp(struct jump_buffer *buf, long val);

/*
 * ########################################### 
 *  THE FOLLOWING FUNCTIONS ARE FROM LIBDETOX 
 * ########################################### 
 */

void *sl_memcpy(void *dest, const void *src, long n);

void *sl_memset(void *dest, long value, unsigned long num);

char *sl_strncpy(char *dest, const char *src, long n);

char *sl_strncat(char *dest, const char *src, unsigned long num);

long sl_strnlen(const char *s, long maxlen);

long sl_strncmp(const char *s1, const char *s2, long n);

char *sl_strchr(const char *s, long character);

char *sl_strrchr(const char *s, long character);

long sl_printf_ext(long fd, const char *format, ...);

long sl_printfva(long fd, const char *format, va_list app);


#endif
