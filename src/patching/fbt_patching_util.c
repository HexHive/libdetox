/**
 * @file fbt_patching_util.c
 * Implementation file for utility functions related to patching
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

#include <assert.h>

#include "fbt_patching_util.h"
#include "../fbt_datatypes.h"
#include "../fbt_libc.h"
#include "../fbt_debug.h"

int fbt_get_executable_path(char *target, long len) {
  int res = 0;
  fbt_readlink("/proc/self/exe", target, len-1, res);
  target[res] = '\0';
  return res;
};

int fbt_get_patch_path(char *target, long len) {
  char executable_path[BUFFER_SIZE];

  int l = fbt_get_executable_path(executable_path, BUFFER_SIZE);
  if (l > 0) {
    long total_len = fbt_strnlen(".patch", len)
                   + fbt_strnlen(executable_path, len);
    if(total_len >= len) {
      return -1;
    }
    fbt_strncpy(target, executable_path, len);
    fbt_strncpy(target + fbt_strnlen(executable_path, len), ".patch", 6);
    target[total_len] = '\0';

    PRINT_DEBUG("Patch path is: %s\n", target);

    return total_len;
  }
  return len;
}

int read_struct(long fd, void *target, int size) {
    long total_read = 0;
    long num_read;
    while (total_read < size) {
      fbt_read(fd, target, size - total_read, num_read);
      total_read += num_read;
    }

    assert(total_read == size);

    return 0;
}
