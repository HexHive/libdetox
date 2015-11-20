/**
 * @file fbt_algorithms.h
 * Header file for generic algorithms
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
#ifndef FBT_ALGORITHMS_H
#define FBT_ALGORITHMS_H

#include <stdint.h>
#include <stddef.h>

#define LEN(x) (sizeof(x)/sizeof((x)[0]))

#ifdef __cplusplus
extern "C" {
#endif

/** Prints an integer array to stdout
 *
 * @param begin     Pointer to the first element of the array to be printed
 * @param num       The number of elements in the array
 * @param highlight Pointer to an element to be highlighted, or NULL for
 *                  no highlighting.
 */
void fbt_print_int_array(int *begin, size_t num, int *highlight);

/** Swaps two areas of memory with the same length
 * 
 * @param a Start of first memory region
 * @param b Start of second memory region 
 * @param length Number of bytes to swap
 */
void fbt_swap_mem(void *a, void *b, size_t length);

/** Partitions an array into two groups:
 *    - elements less-or-equal than the pivot
 *    - elements greater-or-equal than the pivot
 *
 * @param begin Pointer to the first element in the array
 * @param num Number of elements in the array
 * @param pivot Index of the pivot 
 * @param size Size in bytes of each of the elements
 * @param comparator Comparator function that compares two elements and returns
 *                   -1: first smaller than second,
 *                   0:  first equal to second,
 *                   1:  first greater than second
 * @result Pointer to the pivot's final position. All elements before the pivot
 *         will be smaller-or-equal and all elements after the pivot will be 
 *         greater-or-equal than the pivot
 */
void *fbt_partition(void *begin, size_t num, size_t pivot, size_t size, int (*comparator)(const void *, const void *));

/** Sorts the elements of an array.
 * 
 * @param base Pointer to the first element in the array
 * @param num  Number of elements in the array
 * @param size Size in bytes of each element
 * @param comparator Comparator method that compares to elements.
 *                   The return value of this function represents whether the
 *                   first elements is smaller, equal to, or greater than
 *                   the second element by returning, a negative value, zero or
 *                   a positive value. 
 */
void fbt_qsort(void *base, size_t num, size_t size, int (*comparator)(const void *, const void *));

/** Searches for an element in a sorted array.
 *  
 * @param base Pointer to the first element of the array
 * @param num Number of elements in the array
 * @param size Size in bytes of each element in the array
 * @param predicate Function that compares elements to the query. This function
 *                  shall follow this prototype:
 *                  
 *                  int predicate(const void *elem, const void* context)
 *                  
 *                  The return value of this function represents whether the
 *                  element pointed to by elem is considered smaller-than,
 *                  equal to or greater than the queried item by returning
 *                  -1, zero or +1, respectively.
 * @param context The context used by the predicate; usually this is a pointer
 *                to the query item type-cast as void *.
 * @return Pointer to the searched element if it was found, otherwise the
 *         position in the array before which the element would have to be
 *         inserted.
 * @note The returned pointer might point beyond the valid bounds of the array.
 */
void *fbt_binary_search(void *base, size_t num, size_t size, int (*predicate)(const void *, const void *), const void *context);

/** Hashes an array of uint32_t members with the specified length and initial 
    value
    
    @param k Pointer to the start of the array to be hashed
    @param length Length of the array counted in uint32_ts
    @param initval Initial value used to seed the hash computation. Has to be 
                   the same among calls for fbt_hash to produce
                   consistent results
    @return Hash value of the array
*/
uint32_t fbt_hash(const uint32_t *k, size_t length, uint32_t initval);

#ifdef __cplusplus
}
#endif

#endif

