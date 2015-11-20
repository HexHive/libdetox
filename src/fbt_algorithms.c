/**
 * @file fbt_algorithms.c
 * Implementation file for generic algorithms
 *
 * Copyright (c) 2011 ETH Zurich
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


#include <stdint.h>

#include "fbt_algorithms.h"
#include "fbt_llio.h"

void fbt_print_int_array(int *begin, size_t num, int *highlight) {
  llprintf("[  ");
  int *cur = begin;
  size_t i;
  for (i = 0; i < num; ++i) {
    if (cur == highlight) {
      llprintf("%d* ", *cur);
    } else {
      llprintf("%d  ", *cur);
    }
    ++cur;
  } 
  llprintf("]\n");
}

void fbt_swap_mem(void *a, void *b, size_t length) {
  /* We use integer swaps as long as possible */
  while (length >= sizeof(int)) {
    int *a_int = (int *)a;
    int *b_int = (int *)b;
    
    int tmp = *a_int;
    *a_int++ = *b_int;
    *b_int++ = tmp;
    
    a = a_int;
    b = b_int;
    
    length -= sizeof(int);
  }

  /* For the remaining bytes, we use byte swaps */
  while (length > 0) {
    char *a_char = (char *)a;
    char *b_char = (char *)b;
    
    char tmp = *a_char;
    *a_char++ = *b_char;
    *b_char++ = tmp;
    
    a = a_char;
    b = b_char;
    
    length -= 1;
  }
}

void *fbt_partition(void *begin, size_t num, size_t pivot, size_t size, int (*comparator)(const void *, const void *)) {
  /* Move pivot to begin of array */
  fbt_swap_mem(begin, (char *)begin + (size * pivot), size);
  
  void *insert_pos = (char *)begin + size;
  size_t cur = 1;
  
  /* Iterate over all elements and swap elements smaller than the pivot to the
   * left side */
  while (cur < num) {
    void *cur_ptr = (char *)begin + cur * size;
    
    if (comparator(cur_ptr, begin) < 0) {
      fbt_swap_mem(cur_ptr, insert_pos, size);
      insert_pos = (char *)insert_pos + size;
    }
    
    cur += 1;
  }
  
  /* Swap back pivot */
  fbt_swap_mem(begin, (char *)insert_pos - size, size);
  
  return (char *)insert_pos - size;
}

void fbt_qsort(void *base, size_t num, size_t size, int (*comparator)(const void *, const void *)) {
  size_t begin = 0;
  size_t end = num;

  while (end > begin) {  
    // TODO: use insertion sort for small array sizes
  
    size_t mid = (end - begin) / 2;  
    void *pivot = fbt_partition(base, num, mid, size, comparator);
    
    //fbt_print_int_array((int *)base, num, (int *)pivot); 
    
    mid = ((char *)pivot - (char *)base) / size;
    
    /* We determine the smaller partition and solve it recursively, and solve
     the larger partition iteratively to keep the stack usage as small as 
     possible*/
    if (mid >= num-mid-1) {    
      /* Recursion */
      fbt_qsort((char *)base + (mid + 1)*size, num-mid-1, size, comparator);    
      
      /* Iteration */
      num = mid;
      begin = 0;
      end = num;    
    } else  {
      /* Recursion */
      fbt_qsort(base, mid, size, comparator);          
      
      /* Iteration */
      base = (char *)base + (mid + 1)*size;
      num = num - mid - 1;
      begin = 0;
      end = num;    
    }
  }

}

void *fbt_binary_search(void *base, size_t num, size_t size, int (*predicate)(const void *, const void *), const void *context) {
  size_t begin = 0;
  size_t end = num;
  
  while (end > begin) {
    size_t mid = begin + (end - begin) / 2;
    
    void *ptr = (char *)base + (mid*size);
    int comp = predicate(ptr, context);
    
    if (comp == 0) {
      return ptr;
    } else if (comp > 0) {
      begin = mid + 1;  
    } else {
      end = mid;
    }
  }  

  return (char *)base + (end*size) ;
}

/* -----------------------------------------------------------------------------
 The hashing code below is taken from http://burtleburtle.net/bob/c/lookup3.c 
 which is in the Public Domain.
 * -----------------------------------------------------------------------------
 */

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k)))) 
 
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

/*
--------------------------------------------------------------------
 This works on all machines.  To be useful, it requires
 -- that the key be an array of uint32_t's, and
 -- that the length be the number of uint32_t's in the key

 The length has to be measured in uint32_ts.  
--------------------------------------------------------------------
*/
uint32_t fbt_hash(
const uint32_t *k,                   /* the key, an array of uint32_t values */
size_t          length,               /* the length of the key, in uint32_ts */
uint32_t        initval)         /* the previous hash, or an arbitrary value */
{
  uint32_t a,b,c;

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + (((uint32_t)length)<<2) + initval;

  /*------------------------------------------------- handle most of the key */
  while (length > 3)
  {
    a += k[0];
    b += k[1];
    c += k[2];
    mix(a,b,c);
    length -= 3;
    k += 3;
  }

  /*------------------------------------------- handle the last 3 uint32_t's */
  switch(length)                     /* all the case statements fall through */
  { 
  case 3 : c+=k[2];
  case 2 : b+=k[1];
  case 1 : a+=k[0];
    final(a,b,c);
  case 0:     /* case 0: nothing left to add */
    break;
  }
  /*------------------------------------------------------ report the result */
  return c;
}

