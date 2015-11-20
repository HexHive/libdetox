/**
 * @file fbt_dynarace.c
 * Implementation of the dynarace subsystem that dynamically removes races
 * through system call analysis.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-01-06 14:54:16 +0100 (Fri, 06 Jan 2012) $
 * $LastChangedDate: 2012-01-06 14:54:16 +0100 (Fri, 06 Jan 2012) $
 * $LastChangedBy: payerm $
 * $Revision: 1148 $
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
/* include fbt_dynarace.h is not needed as this file is included in
   fbt_syscalls.h */

#include <asm-generic/fcntl.h>

static enum syscall_auth_response dynarace_open(struct thread_local_data *tld,
                                                ulong_t syscall_nr, ulong_t arg1,
                                                ulong_t arg2, ulong_t arg3,
                                                ulong_t arg4, ulong_t arg5,
                                                ulong_t *arg6,
                                                ulong_t is_sysenter,
                                                ulong_t *retval);

static enum syscall_auth_response dynarace_fstat(struct thread_local_data *tld,
                                                 ulong_t syscall_nr, ulong_t arg1,
                                                 ulong_t arg2, ulong_t arg3,
                                                 ulong_t arg4, ulong_t arg5,
                                                 ulong_t *arg6,
                                                 ulong_t is_sysenter,
                                                 ulong_t *retval);

static enum syscall_auth_response dynarace_creat(struct thread_local_data *tld,
                                                 ulong_t syscall_nr, ulong_t arg1,
                                                 ulong_t arg2, ulong_t arg3,
                                                 ulong_t arg4, ulong_t arg5,
                                                 ulong_t *arg6,
                                                 ulong_t is_sysenter,
                                                 ulong_t *retval);

static enum syscall_auth_response dynarace_stat(struct thread_local_data *tld,
                                                ulong_t syscall_nr, ulong_t arg1,
                                                ulong_t arg2, ulong_t arg3,
                                                ulong_t arg4, ulong_t arg5,
                                                ulong_t *arg6,
                                                ulong_t is_sysenter,
                                                ulong_t *retval);

static enum syscall_auth_response dynarace_close(struct thread_local_data *tld,
                                                 ulong_t syscall_nr, ulong_t arg1,
                                                 ulong_t arg2, ulong_t arg3,
                                                 ulong_t arg4, ulong_t arg5,
                                                 ulong_t *arg6,
                                                 ulong_t is_sysenter,
                                                 ulong_t *retval);


/**
 * Returns true (1) if both structs point to the same file. Same files are
 * identified if the st_dev and st_ino are the same.
 * The size of the file, mode, number of links, uid, gid, and timestamps can
 * change and are not compared.
 * @param stat1 first file
 * @param stat2 second file
 * @return 1 if they are equal, 0 otherwise.
 */
static inline long same_file(struct fstat64 *stat1, struct fstat64 *stat2);
static inline long same_file_notime(struct fstat64 *stat1, struct fstat64 *stat2);

/**
 * Open the specified file and verify that all the stat data is still the
 * same. Files are opened read-only. This function either fails and terminates
 * the program or returns a valid file descriptor.
 * @param file file name that is about to be opened
 * @return File descriptor of the open file
 */
static inline long open_verify(struct dynarace_file *file);

/**
 * Resolves a relative or absolute path. This function reads the path and
 * resolves relative paths to absolute paths and then calls
 * dynarace_getapbspath.
 * @param tld pointer to thread local data
 * @param path absolute or relative path
 * @return Struct that contains the information about the given path.
 */
static struct dynarace_file *dynarace_getpath(struct thread_local_data *tld,
                                              char *path);

/**
 * Resolves an absolute path and adds information to the global mapping cache.
 * @param tld pointer to thread local data
 * @param path absolute path
 * @return Struct that contains the information about the given path.
 */
static struct dynarace_file *dynarace_getabspath(struct thread_local_data *tld,
                                                 char *path);

/**
 * Checks the parameters of a specific file atom relative to a specified
 * directory and adds the information to the file mapping cache.
 * If no information is found then a new dynarace_file is generated and added to
 * the cache. Depending on the current state and the next_state the parameters
 * of the file and its consistency are checked.
 * @param tld thread local data
 * @param filename file atom relative to the current directory
 * @param fd file descriptor for filename, if filename is not a legal file than
 * fd contains the error return value from the open system call. If fd is an
 * open file then we close it in this function.
 * @param dyndir directory that filename is in
 * @param next_state the next state that is stored in the dynfile object.
 * @return a struct that contains all information about the filename
 */
static struct dynarace_file *dynarace_file_atom(struct thread_local_data *tld,
                                                char *filename, long fd,
                                                struct dynarace_file *dyndir,
                                                enum dynarace_file_state next_state) {
  long res;
  struct dynarace_file *dynfile = tld->dynfile_cache;

  //llprintf("Checking file '%s'\n", filename);
  while (dynfile != NULL) {
    if (dyndir == dynfile->dir &&
        fbt_strncmp(dynfile->name, filename, MAX_FILE_LEN) == 0) {
      /* we found the file in the cache, let's see if it is still the same */
      if (dynfile->fderr < 0 && dynfile->fderr == fd) {
        /* illegal file, is the error code still the same? */
        break;
      }
      /* ok, legal file so let's check the stat */
      struct fstat64 fstat;
      fbt_fstat64(fd, &fstat, res, "Unable to check fstat (file_atom).\n");
      
      if (!same_file_notime(&fstat, &(dynfile->stat))) {
        if (dynfile->state == STATE_ENFORCE || next_state == STATE_ENFORCE) {
          fbt_suicide_str("Cached and current stat are not the same "
                          "(file_atom).\n");
        } else {
          llprintf("Warning: cached and current stat not the same for %s (file_atom)\n",
                   dynfile->name);
          fbt_memcpy(&(dynfile->stat), &(fstat), sizeof(struct fstat64));
        }
      }

      /* we found the file in the cache and it was OK */

      /* update the state according to the next state */
      if (next_state == STATE_ENFORCE) {
        dynfile->state = STATE_ENFORCE;
      }
      if (next_state == STATE_UPDATE) {
        if (dynfile->state != STATE_ENFORCE)
          dynfile->state = STATE_UPDATE;
      }
      //llprintf("found in cache: %s %d\n", dynfile->name, dynfile->state);
      return dynfile;
    }  /* end: found cache entry for current file */
    dynfile = dynfile->next;
  }

  /* file not found in cache */
  
  /* add file to cache for future opening */
  if (tld->dynfile_free == NULL) {
    fbt_allocate_new_dynarace_files(tld);
  }
  dynfile = tld->dynfile_free;
  tld->dynfile_free = tld->dynfile_free->next;
  
  fbt_strncpy(dynfile->name, filename, MAX_FILE_LEN);
  dynfile->dir = dyndir;
  
  dynfile->next = tld->dynfile_cache;
  tld->dynfile_cache = dynfile;
  
  if (fd >= 0) {
    fbt_fstat64(fd, &(dynfile->stat), res,
                "Illegal file permissions (fstat, file_atom).\n");
    dynfile->fderr = 0;
    dynfile->nropen = 0;
  } else {
    dynfile->nropen = 0;
    dynfile->fderr = fd;  /* store error code */
    dynfile->stat.st_dev = 0;
    dynfile->stat.st_ino = 0;
  }
  //llprintf("new file: %s %d\n", dynfile->name, dynfile->state);
  return dynfile;
}

static enum syscall_auth_response dynarace_open(struct thread_local_data *tld,
  ulong_t syscall_nr __attribute__((unused)), ulong_t arg1, ulong_t arg2,
  ulong_t arg3, ulong_t arg4 __attribute__((unused)),
  ulong_t arg5 __attribute__((unused)), ulong_t *arg6 __attribute__((unused)),
  ulong_t is_sysenter __attribute__((unused)), ulong_t *retval) {
  char buf[MAX_FILE_LEN];
  fbt_strncpy((char*)&buf, (char*)arg1, MAX_FILE_LEN);

  char *dirname = ".", *filename = buf;
  /* split filename and dirname */
  long i;

  /* if path ends with / then we remove last / */
  if (buf[fbt_strnlen(buf, MAX_FILE_LEN)-1] == '/')
    buf[fbt_strnlen(buf, MAX_FILE_LEN)-1] = 0;

  /* split path into dir path and file */
  for (i = fbt_strnlen(buf, MAX_FILE_LEN)-1; i >= 0; --i) {
    if (buf[i] == '/') {
      buf[i] = 0;
      dirname = buf;
      filename = &(buf[i+1]);
      break;
    }
  }

  struct dynarace_file *dyndir = dynarace_getpath(tld, dirname);
  *retval = -9;
  if (dyndir != NULL) {
    long fdir = open_verify(dyndir);
    long fd, res;

    // if directory is erroneous bail out
    if (fdir < 0) return SYSCALL_AUTH_FAKE;
    
    fbt_openatE(fdir, filename, arg2, arg3, fd);
    *retval = fd;  /* store fd or error code */
    
    fbt_close(fdir, res, "Unable to close check file.\n");
    
    struct dynarace_file *dynafile;
    dynafile = dynarace_file_atom(tld, filename, fd, dyndir, STATE_ENFORCE);
    /* fd res is kept open because we return this to the application */
    
    /* increase the 'open' counter for the application */
    dynafile->nropen++;
  }
  
  return SYSCALL_AUTH_FAKE;
}

static enum syscall_auth_response dynarace_fstat(struct thread_local_data *tld,
  ulong_t syscall_nr, ulong_t arg1, ulong_t arg2,
  ulong_t arg3 __attribute__((unused)), ulong_t arg4 __attribute__((unused)),
  ulong_t arg5 __attribute__((unused)), ulong_t *arg6 __attribute__((unused)),
  ulong_t is_sysenter __attribute__((unused)), ulong_t *retval) {
  /* check all system calls that we handle */
  if (syscall_nr != SYS_access && syscall_nr != SYS_fstat64 && syscall_nr != SYS_stat64) {
    fbt_suicide_str("Illegal system call in dynarace file check.\n");
  }
  
  char buf[MAX_FILE_LEN];
  fbt_strncpy((char*)&buf, (char*)arg1, MAX_FILE_LEN);

  char *dirname = ".", *filename = buf;
  /* split filename and dirname */
  long i;

  /* if path ends with / then we remove last / */
  if (buf[fbt_strnlen(buf, MAX_FILE_LEN)-1] == '/')
    buf[fbt_strnlen(buf, MAX_FILE_LEN)-1] = 0;

  /* split path into dir path and file */
  for (i = fbt_strnlen(buf, MAX_FILE_LEN)-1; i >= 0; --i) {
    if (buf[i] == '/') {
      buf[i] = 0;
      dirname = buf;
      filename = &(buf[i+1]);
      break;
    }
  }

  struct dynarace_file *dyndir = dynarace_getpath(tld, dirname);

  long fd = 0, dirfd = -1;
  if (dyndir != NULL) {
    dirfd = open_verify(dyndir);
    /* bail out if open_verify was unsuccessful */
    if (dirfd < 0) {
      *retval = dirfd;
      return SYSCALL_AUTH_FAKE;
    }
    fbt_openatE(dirfd, filename, O_RDONLY, 0, fd);
  }

  /* if there is an error -> bail out */
  /* two possible error cases: illegal file or illegal file and illegal dir */
  if (fd <= 0 || dyndir == NULL) {
    /* file does not exist or error */
    long ret = 0;
    if (dyndir != NULL) {
      switch (syscall_nr) {
        case SYS_access:
          fbt_faccessatE(dirfd, filename, arg2, 0, ret);
          break;
        case SYS_fstat64:
          fbt_fstat64E(arg1, arg2, ret);
          break;
        case SYS_stat64:
          fbt_stat64E(arg1, arg2, ret);
          break;
      }
    } else {
      llprintf("Illegal path to file '%s'\n", dirname);
      switch (syscall_nr) {
        case SYS_access:
          fbt_accessE(arg1, arg2, ret);
          break;
        case SYS_fstat64:
          fbt_fstat64E(arg1, arg2, ret);
          break;
        case SYS_stat64:
          fbt_stat64E(arg1, arg2, ret);
          break;
      }
    }

    if (dirfd >= 0) {
      long res;
      fbt_close(dirfd, res, "Could not close directory (dynarace_fstat).\n");
    }
    /* we could not open the file, so there must be some problem, let's just
       open the file with accessat and forward that problem to the app */
    if (ret != 0) {
      *retval = ret;
      return SYSCALL_AUTH_FAKE;
    } else {
      /* something is fishy, stop app */
      fbt_suicide_str("Fishy system call (1, dynarace_access).\n");
      return SYSCALL_AUTH_FAKE;
    }
  }  /* fd <= 0 */

  /* add file atom to the cache / check available atom */
  struct dynarace_file *dynfile;
  dynfile = dynarace_file_atom(tld, filename, fd, dyndir, STATE_UPDATE);

  long res;
  switch (syscall_nr) {
    case SYS_access:
      fbt_faccessatE(dirfd, filename, arg2, 0, res);
      *retval = res;
      break;
    case SYS_fstat64:
      fbt_memcpy((char*)arg2, (char*)&(dynfile->stat), sizeof(struct fstat64));
      *retval = 0;
      break;
    case SYS_stat64:
      fbt_memcpy((char*)arg2, (char*)&(dynfile->stat), sizeof(struct fstat64));
      *retval = 0;
      break;
  }

  /* close all open files */
  fbt_close(dirfd, res, "Could not close directory (dynarace_access).\n");
  fbt_close(fd, res, "Could not close file (dynarace_access).\n");
  return SYSCALL_AUTH_FAKE;
}


static enum syscall_auth_response dynarace_creat(struct thread_local_data *tld,
  ulong_t syscall_nr, ulong_t arg1, ulong_t arg2,
  ulong_t arg3 __attribute__((unused)), ulong_t arg4 __attribute__((unused)),
  ulong_t arg5 __attribute__((unused)), ulong_t *arg6 __attribute__((unused)),
  ulong_t is_sysenter __attribute__((unused)), ulong_t *retval) {
  /* forward to dynarace_open */
  return dynarace_open(tld, syscall_nr, arg1, O_CREAT|O_WRONLY|O_TRUNC, arg2, 0, 0, NULL,
                       is_sysenter, retval);
}


static enum syscall_auth_response dynarace_stat(
  struct thread_local_data *tld __attribute__((unused)),
  ulong_t syscall_nr __attribute__((unused)),
  ulong_t arg1 __attribute__((unused)), ulong_t arg2 __attribute__((unused)),
  ulong_t arg3 __attribute__((unused)), ulong_t arg4 __attribute__((unused)),
  ulong_t arg5 __attribute__((unused)), ulong_t *arg6 __attribute__((unused)),
  ulong_t is_sysenter __attribute__((unused)),
  ulong_t *retval __attribute__((unused))) {
  llprintf("stat not handled\n");
  return SYSCALL_AUTH_GRANTED;
}


static enum syscall_auth_response dynarace_close(struct thread_local_data *tld,
  ulong_t syscall_nr __attribute__((unused)), ulong_t arg1,
  ulong_t arg2 __attribute__((unused)), ulong_t arg3 __attribute__((unused)),
  ulong_t arg4 __attribute__((unused)), ulong_t arg5 __attribute__((unused)),
  ulong_t *arg6 __attribute__((unused)),
  ulong_t is_sysenter __attribute__((unused)), ulong_t *retval) {

  struct fstat64 curfile;
  long res;
  fbt_fstat64(arg1, &curfile, res, "Unable to check fstat (dynarace_close).\n");

  struct dynarace_file *dynfile = tld->dynfile_cache;
  while (dynfile != NULL) {
    if (same_file_notime(&(dynfile->stat), &curfile)) {
      /* we found the file in the cache, let's update the cache and close the
         file */ 
      dynfile->nropen--;
      if (dynfile->nropen == 0) {
        dynfile->state = STATE_CLOSE;
      }
      fbt_closeE(arg1, res);
      *retval = res;
      return SYSCALL_AUTH_FAKE;
    }
    dynfile = dynfile->next;
  }
  // TODO: ApacheHack
  // loader has already a couple of files opened, ensure that this is OK
  //fbt_suicide_str("Closing a file without data in the cache (dynarace_close).\n");
  //return SYSCALL_AUTH_FAKE;
  //llprintf("Closing file without data in the cache (dynarace_close).\n");
  return SYSCALL_AUTH_GRANTED;
}

static inline long same_file_notime(struct fstat64 *stat1, struct fstat64 *stat2) {
  /* if inode, device, permissions, and user/group information are the same
     then the file is the same as well */
  return (stat1->st_dev == stat2->st_dev && stat1->st_ino == stat2->st_ino &&
          stat1->st_mode == stat2->st_mode && stat1->st_uid == stat2->st_uid &&
          stat1->st_gid == stat2->st_gid);
}

static inline long same_file(struct fstat64 *stat1, struct fstat64 *stat2) {
  /* if inode, device, permissions, and user/group information are the same
     then the file is the same as well */
  return same_file_notime(stat1, stat2);// &&
  //    (stat1->st_ctime == stat2->st_ctime);
}

static inline long open_verify(struct dynarace_file *file) {
  long fd;

  //llprintf("opening %s\n", file->name);
  // open files can fail, return error code (for loader)
  fbt_openE(file->name, 0, 0, fd);
  // bail out if file cannot be opened
  if (fd < 0) return fd;
  
  struct fstat64 fstat;
  long res;
  fbt_fstat64(fd, &fstat, res, "Unable to check fstat (open_verify).\n");

  if (!same_file_notime(&fstat, &(file->stat)))
    llprintf("Cached and current stat are not the same "
                    "(open_verify).\n");

  /* we found the file in the cache and it was OK */
  return fd;
}

static struct dynarace_file *dynarace_getpath(struct thread_local_data *tld,
                                              char *path) {
  char dirbuf[MAX_FILE_LEN];
  dirbuf[0] = 0;
  long i = 0;
  /* make relative path absolute */
  if (path[0] != '/') {
    fbt_getcwd(dirbuf, MAX_FILE_LEN-2, i, "Error in getcwd\n");
    i = fbt_strnlen(dirbuf, MAX_FILE_LEN);
    dirbuf[i++] = '/';
    dirbuf[i] = 0;
  }
  /* add dir path to it */
  char *ptr = (char*)(&dirbuf[i]);
  fbt_strncpy(ptr, path, MAX_FILE_LEN-i);
  
  return dynarace_getabspath(tld, dirbuf);
}

static struct dynarace_file *dynarace_getabspath(struct thread_local_data *tld,
                                                 char *path) {
  //llprintf("Resolving: %s\n", path);
  
  struct dynarace_file *loop = tld->dynfile_cache;
  while (loop != NULL) {
    if (fbt_strncmp(loop->name, path, MAX_FILE_LEN) == 0)
      return loop;
    loop = loop->next;
  }

  long res = fbt_strnlen(path, MAX_FILE_LEN);
  
  char *curdir = path + res;
  /* loop entry requirement: path ends with '/' */
  if (*(curdir-1) != '/')
    *curdir++ = '/';
  
  struct dynarace_file *lastdir = NULL;
  struct dynarace_file *first = NULL;
  
  while (curdir != path) {
    curdir--;
    if (*curdir == '/') {
      *curdir = 0;
      if (fbt_strnlen(path, MAX_FILE_LEN) == 0) {
        /* special case for root directory */
        *curdir = '/';
        *(curdir+1) = 0;
      }
      
      /* just an /./ in the loop, let's continue */
      if (*(curdir-1) == '.' && *(curdir-2) == '/') {
        curdir--;
        continue;
      }
      
      loop = tld->dynfile_cache;
      while (loop != NULL) {
        /* shortcut? if so then return full path */
        if (fbt_strncmp(loop->name, path, MAX_FILE_LEN) == 0) {
          return first == NULL ? loop : first;
        }
        loop = loop->next;
      }
      
      llprintf("DynaRace: adding dir to cache (%s)\n", path);

      long fd;
      fbt_openE(path, O_DIRECTORY, 0, fd);
      if (fd < 0) {
        // cannot access path at all, return ERROR
        if (first == NULL) return NULL;
        // otherwise: fail
        fbt_suicide_str("Could not open directory.\n");
      }

      if (tld->dynfile_free == NULL) {
        fbt_allocate_new_dynarace_files(tld);
      }
      struct dynarace_file *dynadir = tld->dynfile_free;
      tld->dynfile_free = tld->dynfile_free->next;
      dynadir->next = tld->dynfile_cache;
      tld->dynfile_cache = dynadir;
      fbt_strncpy(dynadir->name, path, MAX_FILE_LEN);
      /* fill stat data structure */
      fbt_fstat64(fd, &(dynadir->stat), res,
                  "Could not check current directory.\n");
      fbt_close(fd, res, "Could not close current directory.\n");
      
      if (lastdir != NULL) {
        /* check that . == olddir/.. */
        struct fstat64 dir1;
        dir1.st_dev = 0;
        dir1.st_ino = 0;
        dir1.st_mode = 0;
        
        char namebuf[MAX_FILE_LEN];
        fbt_strncpy(namebuf, lastdir->name, MAX_FILE_LEN-3);
        long tmp = fbt_strnlen(namebuf, MAX_FILE_LEN);
        namebuf[tmp] = '/';
        namebuf[tmp+1] = '.';
        namebuf[tmp+2] = '.';
        namebuf[tmp+3] = 0;
        
        long fdl;
        fbt_open(namebuf, O_DIRECTORY, 0, fdl, "Could not open directory.\n");

        unsigned long i;
        for (i = 0; i < sizeof(struct fstat64)/sizeof(long); i++)
          ((long*)&dir1)[i] = 0;
        
        fbt_fstat64(fdl, &dir1, res, "Could not check child directory.\n");

        if (!same_file_notime(&dir1, &(dynadir->stat))) {
          fbt_suicide_str(". and parent/.. are not the same.\n");
        }

        lastdir->dir = dynadir;
        fbt_close(fdl, res, "Could not close dir.\n");
      } else { /* lastdir != NULL */
        first = dynadir;
      }  /* lastdir == NULL */
      
      lastdir = dynadir;
    }  /* handle directory */
  }  /* while more characters in path */

  /* we constructed the complete path, return first element */
  return first;
}

/* static char *getDir(char *path); */

/* struct dynarace_file *fbt_dr_auth_dir(struct thread_local_data *tld, */
/*                                       char *path) { */
/*   struct dynarace_file *dir = NULL; */
/*   long ret;  /\* return from syscalls *\/ */

/*   struct fstat64 dir1; */
/*   dir1.st_dev = 0; */
/*   dir1.st_ino = 0; */
/*   dir1.st_mode = 0; */

/*   /\* search cache for path *\/ */
/*   struct dynarace_file *loop = tld->dynfile_cache; */
/*   while (loop != NULL) { */
/*     if (fbt_strncmp(loop->name, path, MAX_FILE_LEN) == 0) { */
/*       dir = loop; */
/*       break; */
/*     } */
/*     loop = loop->next; */
/*   } */

/*   if (dir != NULL) { */
/*     if (dir->state == STATE_DIR_OK) { */
/*       fbt_stat64(dir->name, &dir1, ret, "Could not verify dir"); */
/*       if (!same_file(&dir1, &(dir->stat))) { */
/*         fbt_suicide_str(". and parent/.. are not the same.\n"); */
/*       } */
/*     } */
/*     /\* old error or OK *\/ */
/*     return dir; */
/*   } */

/*   /\* new entry *\/ */
/*   if (tld->dynfile_free == NULL) { */
/*     fbt_allocate_new_dynarace_files(tld); */
/*   } */
/*   dir = tld->dynfile_free; */
/*   tld->dynfile_free = tld->dynfile_free->next; */
/*   dir->next = tld->dynfile_cache; */
/*   tld->dynfile_cache = dir; */
/*   fbt_strncpy(dir->name, path, MAX_FILE_LEN); */
/*   long fd; */
/*   fbt_open(path, O_DIRECTORY, 0, fd, "Could not open directory.\n"); */
/*   /\* fill stat data structure *\/ */
/*   fbt_fstat64(fd, &(dir->stat), ret, */
/*               "Could not check current directory.\n"); */
/*   fbt_close(fd, ret, "Could not close current directory.\n"); */
/*   dir->state = STATE_DIR_OK; */

/*   /\* TODO: if dir is not OK then we have to return an error *\/ */

/*   /\* we are at the end of the recursion *\/ */
/*   if (fbt_strnlen(path, MAX_FILE_LEN) == 1 && path[0] == '/') { */
/*     return dir; */
/*   } */

/*   /\* check parent recursivly *\/ */
/*   char *ppath = getDir(path); */
/*   ppath[0] = 0; */
/*   struct dynarace_file *parent = fbt_dr_auth_dir(tld, path); */

/*   if (parent->state != STATE_DIR_OK) { */
/*     /\* handle error in parent, the chain is not authenticated if */
/*        there is an error. *\/ */
/*     return dir; */
/*   } */

/*   /\* check link *\/ */
/*   ppath[0] = '/'; */
/*   ppath[1] = '.'; */
/*   ppath[2] = '.'; */
/*   ppath[3] = 0; */
/*   fbt_stat64(path, &dir1, ret, "Could not verify dir"); */
/*   if (!same_file(&dir1, &(dir->stat))) { */
/*     fbt_suicide_str(". and parent/.. are not the same.\n"); */
/*   } */

/*   /\* return OK file *\/ */
/*   dir->dir = parent; */
/*   return dir; /\* all OK *\/ */
/* } */

/* struct dynarace_file *fbt_dr_update_state(struct thread_local_data *tld, */
/*                                           struct dynarace_file *dir, */
/*                                           char *fname, struct fstat64 *stat, */
/*                                           int ret, */
/*                                           enum dynarace_file_state next_state) { */
/*   struct dynarace_file *file = NULL; */
  
/*   /\* search cache for file & dir *\/ */
/*   struct dynarace_file *loop = tld->dynfile_cache; */
/*   while (loop != NULL) { */
/*     if (fbt_strncmp(loop->name, fname, MAX_FILE_LEN) == 0 && loop->dir == dir) { */
/*       file = loop; */
/*       break; */
/*     } */
/*     loop = loop->next; */
/*   } */

/*   /\* CACHE HIT *\/ */
/*   if (file != NULL) { */
/*     enum dynarace_file_state cur_state = file->state; */
/*     /\* state: ENFORCE *\/ */
/*     if (cur_state == STATE_ENFORCE || */
/*         (cur_state == STATE_UPDATE && next_state == STATE_ENFORCE)) { */
/*       file->state = STATE_ENFORCE; */
/*       if (ret >= 0) { */
/*         /\* are the files equal? *\/ */
/*         if (!same_file(stat, &(file->stat))) */
/*           fbt_suicide_str("Cache mismatch in enforce state"); */
/*       } else { */
/*         if (file->fderr != ret) */
/*           llprintf("Cache mismatch for enforced file (%s)\n", fname); */
/*         file->fderr = ret;  /\* store err *\/ */
/*       } */
/*       return file; */
/*     }  /\* state: ENFORCE *\/ */

/*     /\* state: UPDATE || CLOSE -> ENFORCE *\/ */
/*     if (next_state == STATE_UPDATE || */
/*         (cur_state == STATE_CLOSE && next_state == STATE_ENFORCE)) { */
/*       file->state = STATE_UPDATE; */
/*       if (ret >=0 ) { */
/*         /\* are the files equal? *\/ */
/*         if (!same_file(stat, &(file->stat))) { */
/*           llprintf("Cache mismatch in update state (%s)\n", fname); */
/*           fbt_memcpy(&(file->stat), stat, sizeof(struct fstat64)); */
/*         } */
/*       } else { */
/*         file->fderr = ret; /\* store err *\/ */
/*       } */
/*       return file; */
/*     }  /\* state: UPDATE || CLOSE -> ENFORCE *\/ */
/*   }  /\* CACHE HIT *\/ */

/*   /\* NEW FILE ATOM *\/ */
/*   if (next_state == STATE_ENFORCE) */
/*     llprintf("Unchecked file modified (%s)\n", fname); */

/*   if (tld->dynfile_free == NULL) { */
/*     fbt_allocate_new_dynarace_files(tld); */
/*   } */
/*   file = tld->dynfile_free; */
/*   tld->dynfile_free = tld->dynfile_free->next; */
/*   file->next = tld->dynfile_cache; */
/*   tld->dynfile_cache = file; */
  
/*   fbt_strncpy(dir->name, fname, MAX_FILE_LEN);  /\* store name *\/ */
/*   file->dir = dir;  /\* link directory *\/ */
/*   file->state = next_state; */
/*   fbt_memcpy(&(file->stat), stat, sizeof(struct fstat64)); */
/*   file->fderr = ret;  /\* store error or 0 *\/ */
  
/*   return file; */
/* } */

                                          
/* static char *getDir(char *path) { */
/*   char *cur = NULL; */
/*   do { */
/*     if (*path == '/') */
/*       cur = path; */
/*   } while (*(path++)); */
  
/*   return cur; */
/* } */

/* resolves path to an absolute path which is stored in buf. returns a pointer
   to the final file atom */
/* static char *getAbsolutePath(char *path, char *buf) { */
/*   char dirbuf[MAX_FILE_LEN+2]; */
/*   long i = 0; */
/*   /\* make relative path absolute *\/ */
/*   if (path[0] != '/') { */
/*     fbt_getcwd(dirbuf, MAX_FILE_LEN-2, i, "Error in getcwd\n"); */
/*     i = fbt_strnlen(dirbuf, MAX_FILE_LEN); */
/*     dirbuf[i++] = '/'; */
/*     dirbuf[i] = 0; */
/*   } */
/*   /\* add dir path to it *\/ */
/*   char *ptr = (char*)(&dirbuf[i]); */
/*   fbt_strncpy(ptr, path, MAX_FILE_LEN-i); */

/*   /\* copy cleaned string to buffer *\/ */
/*   long pos = 0; */
/*   i = 0; */
/*   buf[0] = '/'; */
/*   buf[1] = 0; */
/*   while (dirbuf[i] != 0) { */
/*     if (dirbuf[i] == '/' && dirbuf[i+1] == '.' && dirbuf[i+2] == '.' && dirbuf[i+3] == '/') { */
/*       i+=3; */
/*       while (buf[pos] != '/') { */
/*         pos--; */
/*         if (pos == 0) */
/*           fbt_suicide_str("Illegal path."); */
/*       } */
/*     } */
/*     buf[pos++] = dirbuf[i++]; */
/*   } */
/*   buf[pos] = 0; */

/*   char *ret = &(buf[pos]); */
/*   while (*ret != '/' && ret != buf) */
/*     ret--; */
/*   ret++; */
  
/*   return ret; */
/* } */

/* static enum syscall_auth_response fbt_dr_open(struct thread_local_data *tld, */
/*   ulong_t syscall_nr __attribute__((unused)), ulong_t arg1, ulong_t arg2, */
/*   ulong_t arg3, ulong_t arg4 __attribute__((unused)), */
/*   ulong_t arg5 __attribute__((unused)), ulong_t *arg6 __attribute__((unused)), */
/*   ulong_t is_sysenter __attribute__((unused)), ulong_t *retval) { */

/*   char path[MAX_FILE_LEN]; */
/*   char atom[MAX_FILE_LEN]; */

/*   char *atomp = getAbsolutePath((char*)arg1, (char*)&path); */
/*   if (*atomp == '0') { */
/*     atom[0] = '.'; */
/*     atom[1] = 0; */
/*   } else { */
/*     fbt_strncpy((char*)&atom, atomp, MAX_FILE_LEN); */
/*     atomp[0] = 0; */
/*   } */
  
/*   struct dynarace_file *dr_path = fbt_dr_auth_dir(tld, (char*)&path); */

/*   if (dr_path->fderr < 0) { */
/*     *retval = -9;  /\* EACCESS *\/ */
/*     return SYSCALL_AUTH_FAKE; */
/*   } */

/*   long lflags = arg2 & (~O_TRUNC); */
/*   long ret; */
/*   fbt_openatE(dr_path->fderr, atom, lflags, arg3, ret); */
/*   fbt_close(dr_path->fderr, ret, "Unable to close path fd"); */
/* } */

void fbt_dynarace_init(struct thread_local_data *tld) {
  /* grab working directory first and work down to root */
  long res;
  char buf[MAX_FILE_LEN];
  /* -3 because we add /.. to it in the traversal */
  fbt_getcwd(buf, MAX_FILE_LEN-3, res,
             "Could not determine current working directory.\n");
  llprintf("DynaRace: Opening root-chain: %s, res: %d\n", buf, res);
  dynarace_getabspath(tld, buf);
}
