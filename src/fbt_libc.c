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

#include "fbt_libc.h"

/* functions from string.h */
// TODO: optimize!
void *fbt_memset(void *s, const char c, int n)
{
    int i;
    char *target = (char*)s;
    for (i=0; i<n; i++) { target[i]=c; }
    return s;
}

void *fbt_memcpy(void *dest, const void *src, int n)
{
    int i;
    char *target = (char*)dest;
    char *source = (char*)src;
    for (i=0; i<n; i++) { target[i]=source[i]; }
    return (void*)dest;
}

char *fbt_strncpy(char *dest, const char *src, int n)
{
    int i;
    for (i=0; i<n && src[i]!='\0'; i++)
	dest[i]=src[i];
    //for (; i < n ; i++)
    dest[i] = '\0';
    return dest;
}

char *fbt_strcpy(char *dest, const char *src)
{
    int i;
    for (i=0; src[i]!='\0'; i++)
	dest[i]=src[i];
    dest[i]='\0';
    return dest;
}

int fbt_strnlen(const char *s, int maxlen)
{
    int i=0;
    if (maxlen!=0) while (s[i]!=0x0 && i<maxlen) i++;
    else while (s[i]!=0x0) i++;
    return i;
}

int fbt_strcmp(const char *s1, const char *s2)
{
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

int fbt_strncmp(const char *s1, const char *s2, int n)
{
    for(; *s1 == *s2; ++s1, ++s2, --n)
        if(*s1 == 0 || n==0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}


#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4))); \
__syscall_return(type,__res); \
}
