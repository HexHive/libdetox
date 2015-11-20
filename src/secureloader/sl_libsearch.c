/**
 * @file sl_libsearch.c
 * Implements functions to search for needed shared libraries.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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
#include "sl_libsearch.h"

extern struct rtld_global_ro *rtld_glob_ro;


/**
 * Tries to open the file called name in one of the folders specified by path.
 * @param name Name of file 
 * @param search_paths Path structure containing all paths to search
 * @param found_path Path where the file is found
 * @return file descriptor
 */
long try_open_path(const char *name, long name_len,
                          struct r_search_path_elem *search_paths,
                          char **found_path) {
  /* Allocate memory for path */
  char *path = sl_malloc(MAX_PATH_LEN);  
  if (path == 0) {
    sl_printf("Error try_open_path: sl_malloc failed\n");
    sl_exit(1);
  }

  /* Iterate over search paths */
  while (search_paths != 0) {
    /* Build path */
    sl_strncpy(path, search_paths->dirname, search_paths->dirnamelen);
    if (path[search_paths->dirnamelen] != '/')
      sl_strncat(path, "/", 1);
    sl_strncat(path, name, name_len);

    /* Try to open file */
    long fd = sl_open(path, O_RDONLY);
    if(fd != -1) {
      /* File found, protect path and return file descriptor */
      PROT_DATA(path, MAX_PATH_LEN);
      *found_path = path;
      return fd;
    } 
    search_paths = search_paths->next;
  }

  /* Not found, free memory */
  sl_free(path, MAX_PATH_LEN);
  return -1;
}

/* TODO: what about local libraries? I.e. in './'? */

long search_lib(dso *loader, const char *name, char **path) {
  long fd = -1;
  long name_len = sl_strnlen(name, MAX_LIB_NAME);
    
  /* First try search paths of loader and parents */
  dso *so;
  for (so = loader; so != 0; so = so->loader) { 
    fd = try_open_path(name, name_len, so->search_path, path);
    if (fd != -1)
     return fd;
  }

  /* Now try default paths */
  fd = try_open_path(name, name_len, GL_RO(_dl_init_all_dirs), path);
  if (fd != -1)
    return fd; 

  return -1;
}


struct r_search_path_elem *decompose_path(const char *path, const char *where,
                                          const char *what) {

  /* Allocate memory for search paths list */
  struct r_search_path_elem *ret = (struct r_search_path_elem *)
    sl_malloc(sizeof(struct r_search_path_elem));
  struct r_search_path_elem *curr = ret;
  struct r_search_path_elem *last = curr;

  /* Copy path to tokenize it */
  char *buffer = sl_malloc(MAX_PATH_LEN);
  if ((int) buffer == -1) {
    sl_printf("malloc failed\n");
    sl_exit(1);
  }
  sl_strncpy(buffer, path, MAX_PATH_LEN);

  char *tok = 0;
  long i = 0, l = 0;
  long path_len = sl_strnlen(path, MAX_PATH_LEN);

  /* Path has the form "path1:path2:path3..." tokenize it */
  for (i = 0; i <= path_len; ++i) {
    /* End of token? */
    if (buffer[i] == ':' || i == path_len) {
      buffer[i] = '\0';
      tok = buffer+l;
      long len = i-l+1;
      l = i+1;

      /* Resolve dynamic string tokens */
      if (!sl_strncmp(tok, "$ORIGIN", 6)) {
        /* We have to replace this token by the origin path */
        const char *origin = GL_RO(_dl_origin_path);
        long len_origin = sl_strnlen(origin, MAX_PATH_LEN);

        /* Replace it */
        sl_memcpy(tok, tok+7, len-6);
        char *buf = sl_malloc(MAX_PATH_LEN);
        
        sl_memcpy(buf, origin, len_origin);
        sl_memcpy(buf+len_origin-1, tok, len-6);

        /* Update token */
        tok = buf;
        len = sl_strnlen(tok, MAX_PATH_LEN);
      } else if (!sl_strncmp(tok, "$PLATFORM", 8)) {
        sl_printf("Error decompose_path: dst $PLATFORM not implemented.\n");
        sl_exit(1);
      } else if (!sl_strncmp(tok, "$LIB", 3)) {
        sl_printf("Error decompose_path: dst $LIB not implemented.\n");
        sl_exit(1);
      }

      /* Initialize search paths list */
      curr->what = what;
      curr->where = where;
      curr->dirname = tok;
      curr->dirnamelen = len;
      
      last = curr;

      /* More tokens? */
      if (i <= path_len) {
        /* Prepare next list element */
        curr = (struct r_search_path_elem *)
          sl_malloc(sizeof(struct r_search_path_elem));
        last->next = curr;
      }
    }
  }
  
  return ret;
}
