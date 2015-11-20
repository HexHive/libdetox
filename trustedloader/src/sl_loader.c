/**
 * @file sl_loader.c
 * Implements the main functionality of the secuLoader.
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-04-13 13:31:40 +0200 (Fri, 13 Apr 2012) $
 * $LastChangedDate: 2012-04-13 13:31:40 +0200 (Fri, 13 Apr 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1286 $
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


#include "sl_loader.h"
#include "sl_load.h"
#include "sl_reloc.h"
#include "sl_libc.h"
#include "sl_tls.h"
#include "sl_macros.h"
#include "sl_sym_lookup.h"
#include "sl_datatypes.h"
#include "sl_libsearch.h"
#include "sl_gscope.h"
#include "sl_so_chain.h"

#ifdef ATTACH_LIBDETOX
#include "sl_libdetox.h"
extern struct dso_chain *dso_chain;
#endif

#ifdef SL_STATISTIC
char *stat_fname = 0;          /* Name of loaded file */
extern long num_relocs;        /* Number of performed relocations */
extern long loaded_dsos;       /* Number of loaded DSOs*/
extern long max_loaded_dsos;   /* Maximum number of DSOs loaded at same time */
#endif

extern dso *so_chain;

/* Command line arguments of executable */
char **dl_argv;

/* Libc secure mode */
#ifdef SECURE_MODE
long libc_enable_secure = 1;
#else
long libc_enable_secure = 0;
#endif

/* Stack end */
long libc_stack_end = 0;

/* Global loader-specific data (read/write) */
struct rtld_global *rtld_glob;

/* Global loader-specific data (readonly) */
struct rtld_global_ro *rtld_glob_ro;


#ifdef MEMORY_PROTECTION

/**
 * Sets entire .data section of the loader to readonly.
 */
static __attribute__((noinline)) void protect_data_section() {  
  /* Open loader file */
  long fd = sl_open("/proc/self/exe", O_RDONLY);
  if (fd == -1) {
    sl_printf("Error get_loader_information: failed to open /proc/self/exe\n");
    sl_exit(1);
  }

  /* Get file size */
  struct kernel_stat file_info;
  if(sl_fstat(fd, &file_info) == -1) {
    sl_printf("Error get_loader_information: fstat failed\n");
    sl_exit(1);
  }

  /* Map file in memory */
  char *file_map = (char *)sl_mmap(0, file_info.st_size, PROT_READ,
                                   MAP_PRIVATE, fd, 0);
  if ((long)file_map == -1) {
    sl_printf("Error protect_data_section: failed to mmap loader\n");
    sl_exit(1);
  }

  /* Get ELF and section header */
  Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file_map;
  Elf32_Shdr *shdr = (Elf32_Shdr *)(file_map + ehdr->e_shoff);

  /* String table */
  char *strtab = (char *)(file_map + shdr[ehdr->e_shstrndx].sh_offset);

  /* Get address of data section */
  long i;
  for(i = 0; i < ehdr->e_shnum; ++i) {
    char *name = strtab + shdr[i].sh_name;

    /* Set entire .data section to readonly */
    if (sl_strncmp(name, ".data", 6) == 0) {
      PROT_DATA(TRUNC_PAGE(shdr[i].sh_addr), ROUND_PAGE(shdr[i].sh_size));
      break;
    }
  }
  
  /* Unmap file */
  sl_munmap(file_map, file_info.st_size);  
}

#endif /* MEMORY_PROTECTION */


/**
 * This function is called by libc after the loaded program exited.
 */
static void cleanup() {
#ifdef D_LOAD
  sl_printf("\nsecuLoader: Programm exited normally.\n");
#endif

#ifdef SL_STATISTIC
  /* Write some useful statistics to a file */
  char fname_buf[256];
  sl_strncpy(fname_buf, stat_fname, 32);
  sl_strncat(fname_buf, "_sl_statistics.txt", 19);
  
  long fd = sl_open(fname_buf, O_RDONLY);
  while((int)fd != -1) {
    sl_strncat(fname_buf, "#", 2);
    sl_close(fd);
    fd = sl_open(fname_buf, O_RDONLY);
  }
  sl_close(fd);
  fd = sl_creat(fname_buf, S_IREAD|S_IWRITE);
  sl_printf_ext(fd, "Relocations: %d\nLoaded DSOs: %d\nMax Loaded DSOs: %d",
                num_relocs, loaded_dsos, max_loaded_dsos);
#endif  
}


/**
 * Transfers control to the loaded program by pushing the command line arguments and environment variables on the stack and junmping to the entry address.
 * @param entry Entry address of executable
 * @param argc Number of command line arguments
 * @param argv Pointer to command line arguments
 * @param envp Environment variables
 */
static void __attribute__((noinline)) transfer_control(void *entry, long argc,
                                                       char **argv, char **envp) {
#ifdef D_LOAD
  sl_printf("\n\n-------- STARTING -------\n");
  sl_printf("Entrypoint: %p\n", entry);
#endif
  
  /*
    Stack should look like this:
    0(%esp)argc
    4(%esp)argv[0]
    ...
    (4*argc)(%esp)NULL
    (4*(argc+1))(%esp)envp[0]
  */
  
  /* Push environment variables */
  long i = 0;
  for (i = 0; envp[i] != 0; ++i) {
    asm volatile ("pushl %0;"
                 : 
                 : "g"(envp[i])
                 : "memory", "%esp"
                 );
  }

  /* Push arguments on stack */
  i = argc-1;
  for (i = argc; i >= 0; --i) { 
    asm volatile ("pushl %0;"
                  :
                  : "g"(argv[i])
                  : "memory", "%esp"
                  );
  }

  /* Push argc, set exit function address in %edx and jump to entry*/
  asm volatile ("pushl %1;"
                "jmp *%0;"
                : 
                : "g" (entry), "g" (argc), "d" (cleanup)
                : "memory", "%esp"	
                );
}


/**
 * Calls the init functions of all loaded DSOs with the given arguments.
 * @param argc Number of command line arguments.
 * @param argv Pointer to command line arguments.
 * @param envp Pointer to environment variables.
 */
static void invoke_init_functions(int argc, char **argv, char **envp) {
  dso *so = so_chain;
  
 /* dso* lc = so_chain;
  while(lc) {
    if (sl_strncmp("libc.so.6", lc->name, -1) == 0){
      break;
    }
    lc = lc->next;
  }

  so_init(lc, argc, argv, envp); */

  /* We need to call init of last loaded object first */
  while(so->next != 0) {
    if (so->flags_1 & DF_1_INITFIRST) {
      /* We have to init this DSO before the others */
      //if (so != lc)
        so_init(so, argc, argv, envp);
    }
    so = so->next;
  }

  while(so != 0) {  
    //if (so != lc)
      so_init(so, argc, argv, envp);
    so = so->prev;
  }

}


/**
 * Initializes all global, loader-specific datastructures.
 */
static void init_global_data() {
  /* Initialize rtld_glob */
  rtld_glob = sl_calloc(1, sizeof(struct rtld_global));

  /* Initialize function pointers to functions exported by the loader */
  GL(_dl_error_catch_tsd) = &error_catch_tsd;
  GL(_dl_rtld_lock_recursive) = &lock_recursive;
  GL(_dl_rtld_unlock_recursive) = &unlock_recursive;
  GL(_dl_make_stack_executable_hook) = &make_stack_executable_hook;
  GL(_dl_init_static_tls) = &init_static_tls;
  GL(_dl_wait_lookup_done) = &wait_lookup_done;

  /* Stack permissions */
  GL(_dl_stack_flags) = PF_R|PF_W|PF_X;

  /* Initialize rtld_glob_ro */
  rtld_glob_ro = sl_calloc(1, sizeof(struct rtld_global_ro));

  /* Platform */
  GL_RO(_dl_platform) = "i686"; 
  GL_RO(_dl_platformlen) = 4;

  /* Pagesize */
  GL_RO(_dl_pagesize) = 4096;

  /* Set Debug filedescriptor to standard error output */
  GL_RO(_dl_debug_fd) = STDERR_FILENO;

  /* We support lazy binding */
  GL_RO(_dl_lazy) = 1;

  /* Enable pointer guard */
  GL_RO(_dl_pointer_guard) = 1;

  /* Initialize function pointers to functions exported by the loader */
  GL_RO(_dl_sysinfo) = (unsigned long)sysinfo_int80;
  GL_RO(_dl_debug_printf) = &debug_printf;
  GL_RO(_dl_catch_error) = &catch_error;
  GL_RO(_dl_signal_error) = &signal_error;
  GL_RO(_dl_mcount) = &mcount_internal;
  GL_RO(_dl_lookup_symbol_x) = &lookup_symbol_x;
  GL_RO(_dl_check_caller) = &check_caller;
  GL_RO(_dl_open) = &dl_open;
  GL_RO(_dl_close) = &dl_close;
  GL_RO(_dl_tls_get_addr_soft) = &tls_get_addr_soft;
  GL_RO(_dl_discover_osversion) = &discover_osversion;
}


static void print_dso_chain(dso* d){
  while (d != 0){
    sl_printf("Dso: %s\n", d->name);
    d = d->next;
  }
}

static dso* move_libc_to_front(dso* st){
  dso* lc = 0;
  dso* d = st;
  while (1){
    if (d == 0){
      sl_printf("Failed moving front\n");
      return; 
    }
    if (sl_strncmp("libc.so", d->name, 6) == 0){
      //sl_printf("found libc in chain\n");
      lc = d;
      break;
    }
    //sl_printf("tried... %s\n", d->name);
    d = d->next;
  }
  while (lc->prev){
    dso* pp = lc->prev->prev;
    dso* p = lc->prev;
    dso* n = lc->next;

    if (n) n->prev = p;
    p->next = n;

    p->prev = lc;
    lc->next = p;

    lc->prev = pp;
    if (pp) pp->next = lc;
  }
  return lc; // the new front
}



/**
 * Main function of the loader. Sets up all necessary functionality,
 * loads the executable and transfers control.
 */
int main(int argc, char **argv, char **envp) {
  
#ifdef D_LOAD
  /* Debug print command line arguments */
  sl_printf("SecuLoader started with:\n");
  long i=0;
  for (i=1; i<argc; ++i) {
    sl_printf(" %s ", argv[i]);
  }
  sl_printf("\n");
#endif

  /* Check number of arguments */
  if (argc < 2) {
    sl_printf("Error: not enough arguments.\n");
    return 1;
  }

  /* Initialize global datastructures */
  init_global_data();
  
  /* Get path to executable */
  char *path = sl_malloc(MAX_PATH_LEN);
  sl_strncpy(path, argv[1], MAX_PATH_LEN);
  
  /* Open file */
  long fd = sl_open(path, O_RDONLY);
  if(fd == -1) {
    sl_printf("Error: could not open %s.\n", path);
    return 1;
  }

  /* Extract filename and path */
  const char *filename = sl_strrchr(path, '/');
  filename++;
  char *name = sl_malloc(MAX_LIB_NAME);
  sl_strncpy(name, filename, MAX_LIB_NAME);
  path[filename-path] = '\0';
  GL_RO(_dl_origin_path) = path;

  /* Set name and path to readonly */
  PROT_DATA(name, MAX_LIB_NAME);
  PROT_DATA(path, MAX_PATH_LEN);

#ifdef SL_STATISTIC
  stat_fname = name;
#endif

  /* Initialize system dep. library search paths */
  GL_RO(_dl_init_all_dirs) = decompose_path(LIB_SEARCH_PATHS, 0,
                                            "system search path");

#ifdef ATTACH_LIBDETOX
  /* Load libdetox */
#ifdef HIJACKCONTROL
  sl_printf("Error loading libdetox: HIJACKCONTROL flag set.\n");
  sl_exit(1);
#endif
  /* Load the libdetox framework */
  dso *libdetox = load_libdetox();
#endif
  
  /* Load executable and all dependent objects */
  dso *so_main = load_elf(0, name, path, fd, 0);


///////////////////////////////////////////////////
//
// TO FIX: If we move the libc to front then c++
//         exceptions work (try "lib/loader trustedloader/test/exceptions/main")
//         but tls relocations do not work
//         (try "lib/loader trustedloader/test/tls_relocations/main")
//
//         If we do not move libc to front then it is the other way round,
//         exceptions do not work but tls relocations do!!!
//
//         NOTE: To test the loader one has to run ./testloader.sh because
//               make test in the loader directory does NOT! run the loader!
//               
///////////////////////////////////////////////////
  //print_dso_chain(so_chain);
  //so_chain = move_libc_to_front(so_chain);
  //print_dso_chain(so_chain);
///////////////////////////////////////////////////

/* Note that if ATTACH_LIBDETOX is defined then it is loaded before
   so_main. */


  /* Add all loaded objects to the global scope */
  gscope_add_recursive(so_main);

  /* Delete first command line argument (the loader itself) */
  argc--;
  dl_argv = argv + 1;

  /* Set up Thread Local Storage (TLS) data structures */
  thread_control_block *tcb = setup_tls();

  /* Relocate all loaded objects */
  do_relocations();

  /* Map the tls init images and set up dtv (Note: this has to be done _after_
     the relocations) */
  allocate_tls_init(tcb);
  
  /* Try to resolve the libc malloc which is needed in dl_signal_error */
  dso *so_found = 0;
  Elf32_Sym *sym_found = 0;
  libc_malloc = resolve_symbol(0, "malloc", 0, 1, 1, &sym_found, &so_found);

  /* Save pointer to chain of loaded objects */
  GL(_dl_ns[0])._ns_loaded = (struct link_map *)so_chain;

#ifdef ATTACH_LIBDETOX
  /* Remove libdetox from global chain and global scope! */
  chain_delete(libdetox);
  gscope_remove(libdetox);

#ifdef D_LOAD
  sl_printf("Initializing libdetox.\n");
#endif
  /* Get pointer to thread_local_data */
  struct thread_local_data *tld = libdetox_init(0);
  
#if defined(ATTACH_LIBDETOX) && defined(VERIFY_CFTX)
  /* Set dso_objects pointer to our dso_chain */
  tld->dso_objects = dso_chain;
#endif

#ifdef D_LOAD
  sl_printf("Starting libdetox transaction.\n");
#endif
  /* Start libdetox transaction */
  libdetox_start_transaction(tld, libdetox_commit_transaction);
#endif

#ifdef MEMORY_PROTECTION
  /* Set entire .data section to readonly */
  protect_data_section();
#endif
  
  /* Initialize loaded objects (Note: we already transfer control here!) */
  invoke_init_functions(argc, dl_argv, envp);

  /* Transfer control to executable */
  transfer_control(so_main->entry, argc, dl_argv, envp);

#ifdef ATTACH_LIBDETOX
  /* Commit transaction */
  libdetox_commit_transaction();
#endif
  
  return 0;
}


/**
 * Entry function of the loader. Prepare arguments and call main.
 * @param argp Argument pointer
 */
void _start(char *argp) {
  /* Get number of command line arguments */
  long argc;
  asm volatile ("movl 4(%%ebp), %%eax"
                : "=a" (argc));

  /* Get pointer to arguments and environment variables (environment variables
     lie after arguments on the stack) and call main */
  char **argv = &argp;
  char **envp = &argv[argc + 1];
  long ret = main (argc, argv, envp);
  sl_exit(ret);
}


/*
  -----------------------------------------------------------------------------
  Note: The following functions are exported and used e.g. by the libc.
  DO NOT CHANGE THEIR BEHAVIOUR!
  -----------------------------------------------------------------------------
*/

void ** __attribute__ ((const)) error_catch_tsd(void) {
  /* Return some static memory for the catch structure needed to safe state
     in catch_error */
  static void *data;
  return &data;
}


long REG_ARGS(3) catch_error(const char **objname, const char **errstring,
                             long *mallocedp, void (*operate) (void *),
                             void *args) {
  /* Get memory for catch struct, this is later replaced by the thread lib */
  struct catch c, **catchp = (struct catch **) (*GL(_dl_error_catch_tsd))();
  struct catch *volatile old = *catchp;

  /* Set jump for longjump in signal_error */
  long err = sl_setjmp(&c.buf);
  
  if (err == 0) {
    /* We get here after setjmp */  
    UNPROT_DATA(TRUNC_PAGE(catchp), ROUND_PAGE(sizeof(struct catch *)));
    *catchp = &c;
    /* We can not re-protect it because after the thread library is loaded it
       points to memory not owned by us */
    
    /* Call operate which may throw an error */
    (*operate) (args);
    *objname = 0;
    *errstring = 0;
    *mallocedp = 0;
    return 0;
  }

  /* We get here after a longjump */
  UNPROT_DATA(TRUNC_PAGE(catchp), ROUND_PAGE(sizeof(struct catch *)));
  *catchp = old;
    
  *objname = c.objname;
  *errstring = c.errstring;
  *mallocedp = c.malloced;

  return err == -1 ? 0 : err;
}


void REG_ARGS(3) signal_error(long errcode, const char *objname,
                              const char *occasion, const char *errstring) {
#ifdef D_RLOAD
  sl_printf("Signal error: %s, %s\n", objname, errstring);
#endif
  
  struct catch *catch = *(struct catch **) (*GL(_dl_error_catch_tsd))();
  long len_objname = sl_strnlen(objname, 256) + 1;
  long len_errstring = sl_strnlen(errstring, 256) + 1;

  /* This would lead to an endless signal/catch loop */
  if (errcode == 0) {
    errcode = -1;
  }
  
  /* Set errstring and objname. Actually we _have_ to use libc malloc here,
     seems to be a bug in libc.. */
  catch->errstring = (char *) libc_malloc(len_errstring);
  sl_memcpy((char *)catch->errstring, errstring, len_errstring);
  
  catch->objname = (char *) libc_malloc(len_objname);
  sl_memcpy((char *)catch->objname, objname, len_objname);

  /* We use the libc malloc (anyway this flag is ignored, as stated above) */
  catch->malloced = 1;

  /* Longjump to catch_error */
  sl_longjmp(&catch->buf, errcode);
}


void *dl_open(const char *file, long mode, const void *caller_dlopen,
              long nsid, long argc, char *argv[], char *env[]) {

  /* Get caller */
  dso *caller = get_object_at((char *)caller_dlopen);
      
#ifdef D_RLOAD
  sl_printf ("\nDl_open: file: %s, mode: %x, caller: %s, nsid: %d\n",
             file, mode, caller ? caller->path : "0", nsid);
#endif
  
  /* No file specified, return handle to main object */
  if (!sl_strncmp(file, "", 1)) {
    return so_chain;
  }
  
  /* Handle not implemented modes */
  if ((mode & RTLD_NOLOAD)
      || (mode & RTLD_DEEPBIND)
      || (mode & RTLD_NODELETE)) {
    sl_printf("Error dl_open: mode 0x%x not implemented\n", mode);
    sl_exit(1);
  }
  
  char *name = sl_malloc (MAX_LIB_NAME);
  char *path = sl_malloc (MAX_PATH_LEN);
  long fd = -1;

  /* Parse filename and open library */
  if (sl_strchr(file, '$') != 0) {
    /* Filename contains dynamic string tokens (dst) */
    sl_printf("Error dl_open %s: dynamic string tokens not impl.\n", path);
    sl_exit(1);
    
  } else if (sl_strchr(file, '/')) {
    /* Filename contains a path */

    /* Get filename */
    char *fname = sl_strrchr(file, '/');
    fname++;
    
    /* Check if already loaded */
    dso *found = chain_search(fname);
    if (found != 0) {
      /* Increment reference counter and free memory*/
      sl_free((void *)name, MAX_LIB_NAME);
      sl_free((void *)path, MAX_PATH_LEN);
      UNPROT(found);
      found->ref_count++;
      PROT(found);
      return found;
    }

    /* Copy name and path (owner may be unloaded!) */
    sl_strncpy(name, fname, MAX_LIB_NAME);
    sl_strncpy(path, file, MAX_PATH_LEN);

    /* Try to open shared object */
    fd = sl_open(file, O_RDONLY);
  } else {
    /* Filename is only the name */

    /* Check if already loaded */
    dso *found = chain_search(file);
    if (found != 0) {
      /* Increment reference counter and free memory*/
      sl_free((void *)name, MAX_LIB_NAME);
      sl_free((void *)path, MAX_PATH_LEN);
      UNPROT(found);
      found->ref_count++;
      PROT(found);
      return found;
    }

    /* Copy name (owner may be unloaded!) */
    sl_strncpy(name, file, MAX_LIB_NAME);

    /* Search for shared object */
    fd = search_lib(caller ? caller : so_chain, name, &path);
  }
  
  if(fd == -1) {
    /* File not found */
    signal_error(0, file, 0, "cannot open shared object file");
    return 0;
  }  

  /* Protect name and path */
  PROT_DATA(name, MAX_LIB_NAME);
  PROT_DATA(path, MAX_PATH_LEN);

  /* Load the shared object */
  dso *so_loaded = load_elf(caller, name, path, fd, 1);

  /* Check if NOOPEN flag is set */
  if (so_loaded->flags_1 & DF_1_NOOPEN) {
    unload_elf(so_loaded);
    signal_error(0, file, 0, "flag NOOPEN set");
  }
  
  if ((mode & RTLD_GLOBAL)) {
    /* Add to global scope */
    gscope_add_recursive(so_loaded);
  }
    
  /* Iterate over dependencies */
  dso *so = so_loaded;
  for(; so != 0; so = so->next) {
    /* Dynamic binding */
    if ((mode & RTLD_NOW)
        || (so->flags_1 & DF_1_NOW)
        || (so->flags & DF_BIND_NOW)) {
      /* BindNow -> relocate plt now */
      relocate_plt(so);
    }

    /* Relocations */
    relocate(so, 1);

    /* Thread local storage */
    if (so->tls_blocksize != 0) {
      /* Add object to tls slotinfo list */
      add_to_slotinfo(so);
      GL(_dl_tls_generation)++;
    }
  }

  /* Initialize INITFIRST objects */
  for (so = so_loaded; so->next != 0; so = so->next) {
    if (so->flags_1 & DF_1_INITFIRST) {
      /* We have to init this object before the others */
      so_init(so, argc, argv, env);
    }
  }

  /* Initialize remaining objects */
  while(so != so_loaded) {  
    so_init(so, argc, argv, env);
    so = so->prev;
  }
  so_init(so_loaded, argc, argv, env);
 
  return so_loaded;  
}


void dl_close(void *map) {
  /* Get shared object */
  dso *so = (dso *)map;

  /* Never close handle to main program */
  if (map == so_chain)
    return;

  /* Make DSO writable */
  UNPROT(so);
    
  /* Decrement reference counter */
  so->ref_count--;

  if (so->ref_count == 0) {
    /* No more references to object, we can unload it*/
    
    /* First close all dependent objects */
    long i=0;
    for (i=0; i<so->deps_count; ++i) {
      dl_close(so->deps[i]);
    }
    
    /* Now unmap the object itself */
    unload_elf(so);
  } else {
    /* There are other references to the object */
    PROT(so);
  }
}


void lock_recursive(void *lock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *)lock;
  mutex->__data.__count++;
}


void unlock_recursive(void *lock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *)lock;
  mutex->__data.__count--;
}


/*
 ----------------------------------------------------
 Note: The following functions are not yet supported.
 ----------------------------------------------------
*/

void debug_printf(const char *fmt, ...) {
  sl_printf("Error: not implemented debug_printf\n");
  sl_exit(1);
}

void mcount_internal(Elf32_Addr frompc, Elf32_Addr selfpc) {
  sl_printf("Error: not implemented mcount\n");
  sl_exit(1);
}

long check_caller(const void *caller, enum allowmask mask) {
  sl_printf("Error: not implemented check_caller\n");
  sl_exit(1);
  return 0;
}

void *tls_get_addr_soft(struct link_map *l) {
  sl_printf("Error: not implemented get_addr_soft\n");
  sl_exit(1);
  return 0;
}

long discover_osversion(void) {
  sl_printf("Error: not implemented discover_osversion\n");
  sl_exit(1);
  return 0;
}

long REG_ARGS(1) make_stack_executable_hook(void **stack_endp) {
  sl_printf("Error: not implemented make_stack_executable\n");
  sl_exit(1);
  return 0;
}

void init_static_tls(struct link_map *map) {
  sl_printf("Error: not implemented init_static_tls\n");
  sl_exit(1);
}

void wait_lookup_done(void) {
  sl_printf("Error: not implemented wait_lookup_done\n");
  sl_exit(1);
}
