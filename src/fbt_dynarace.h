/**
 * @file fbt_dynarace.h
 * Header functions for the dynarace subsystem that dynamically removes races
 * through system call analysis. Due to the static keyword this file may only be
 * included in fbt_syscalls.c and fbt_dynarace.c!
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 05:24:05 -0800 (Fri, 30 Dec 2011) $
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
#ifndef FBT_DYNARACE_H
#define FBT_DYNARACE_H

#if defined(__x86_64__)
NOT IMPLEMENTED YET. /* let's crash and burn */
#else  /* i386 mode below */
#define SYS_access        33
#define SYS_stat         106

#define MAX_FILE_LEN 255

struct fstat64 {  /* according to stat manpage */
  long long st_dev;     /* ID of device containing file */
  long unused1;
  long st_ino;     /* inode number */
  long st_mode;    /* protection */
  long st_nlink;   /* number of hard links */
  long st_uid;     /* user ID of owner */
  long st_gid;     /* group ID of owner */
  long unused2[3];
  long long st_rdev;    /* device ID (if special file) */
  long long st_size;    /* total size, in bytes */
  long st_blksize; /* blocksize for file system I/O */
  long long st_blocks;  /* number of 512B blocks allocated */
  long st_atime;   /* time of last access */
  long st_mtime;   /* time of last modification */
  long st_ctime;   /* time of last status change */
  long unused3[3];  
} __attribute__((packed));


enum dynarace_file_state {
  STATE_UPDATE,  /*< file has already been checked (e.g., access'ed) */
  STATE_ENFORCE,  /*< file has been opened using checked data */
  STATE_CLOSE,  /*< file has been closed. might be stale data */
  STATE_NEW,  /*< new, unseen file */
  STATE_DIR_OK,  /*< directory is OK and verified */
  STATE_DIR_ERR  /*< directory state is erroneous */
};

struct dynarace_file {
  enum dynarace_file_state state;  /*< state of the file */
  long nropen; /*< how many times opened in app */
  long fderr;  /*< if !=0 error code (e.g., for illegal files) */
  char name[MAX_FILE_LEN];  /*< absolute filename */
  struct fstat64 stat;  /*< stat of the file */
  struct dynarace_file *dir;  /*< descriptor of the dir */
  struct dynarace_file *next;
};

void fbt_dynarace_init(struct thread_local_data *tld);

#endif  /* i386 mode */
#endif  /* FBT_DYNARACE_H */
