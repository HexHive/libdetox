/* This file contains the shared datasctructures betweeen the libc and the loader. */
/* TODO: Revise this file and implement a method to import the required
 * datastructures from the eglibc sources to TRuE. */

/* NOT USED */
struct r_scope_elem {
  /* Array of maps for the scope */
  struct link_map **r_list;
  
  /* Number of entries in the scope */
  unsigned long r_nlist;
};

/* NOT USED */
struct r_search_path_struct {
  struct r_search_path_elem **dirs;
  long malloced;
};

/* This is the linux loader internal representation of a DSO */
struct link_map {
  Elf32_Addr l_addr;    /* Base address shared object is loaded at.  */
  char *l_name;         /* Absolute file name object was found in.  */
  Elf32_Dyn *l_ld;      /* Dynamic section of the shared object.  */
  struct link_map *l_next, *l_prev; /* Chain of loaded objects.  */

  /* This is a hack to fill the space of link_map we do not use because it is
   * linux loader internal stuff. But we need it as we have to support rtld_global
   * and rtld_global_ro (see below) and without this the offsets in these structures
   * would be wrong... */
  //char placeholder[576];
  char placeholder[580];
};

/* NOT USED */
struct r_debug {
  long r_version;
  struct link_map *r_map;
  Elf32_Addr r_brk;
  enum {
    RT_CONSISTENT,
    RT_ADD,
    RT_DELETE
  } r_state;

  Elf32_Addr r_ldbase;
};

struct rtld_global
{
  /* Don't change the order of the following elements.  'dl_loaded'
     must remain the first element.  Forever.  */

/* Non-shared code has no support for multiple namespaces.  */
/* MANUAL CHANGE */
# define DL_NNS 16
# define EXTERN
# define internal_function
# define ElfW(Addr) int
# define bool long
# define fpu_control_t unsigned int
# define lookup_t struct link_map
# define Lmid_t long
# define r_found_version version_info

  EXTERN struct link_namespaces
  {
    /* A pointer to the map for the main map.  */
    struct link_map *_ns_loaded;
    /* Number of object in the _dl_loaded list.  */
    unsigned int _ns_nloaded;
    /* Direct pointer to the searchlist of the main object.  */
    struct r_scope_elem *_ns_main_searchlist;
    /* This is zero at program start to signal that the global scope map is
       allocated by rtld.  Later it keeps the size of the map.  It might be
       reset if in _dl_close if the last global object is removed.  */
    size_t _ns_global_scope_alloc;
    /* Search table for unique objects.  */
    struct unique_sym_table
    {
      __rtld_lock_recursive_t lock;
      struct unique_sym
      {
	uint32_t hashval;
	const char *name;
	const ElfW(Sym) *sym;
	const struct link_map *map;
      } *entries;
      size_t size;
      size_t n_elements;
      void (*free) (void *);
    } _ns_unique_sym_table;
    /* Keep track of changes to each namespace' list.  */
    struct r_debug _ns_debug;
  } _dl_ns[DL_NNS];
  /* One higher than index of last used namespace.  */
  EXTERN size_t _dl_nns;

  /* During the program run we must not modify the global data of
     loaded shared object simultanously in two threads.  Therefore we
     protect `_dl_open' and `_dl_close' in dl-close.c.

     This must be a recursive lock since the initializer function of
     the loaded object might as well require a call to this function.
     At this time it is not anymore a problem to modify the tables.  */
/* MANUAL CHANGE */
  //__rtld_lock_define_recursive (EXTERN, _dl_load_lock)
  /* This lock is used to keep __dl_iterate_phdr from inspecting the
     list of loaded objects while an object is added to or removed
     from that list.  */
/* MANUAL CHANGE */
  //__rtld_lock_define_recursive (EXTERN, _dl_load_write_lock)

/* MANUAL CHANGE */
  __rtld_lock_recursive_t _dl_load_lock;
  __rtld_lock_recursive_t _dl_load_write_lock;
  
  /* Incremented whenever something may have been added to dl_loaded.  */
  EXTERN unsigned long long _dl_load_adds;

  /* The object to be initialized first.  */
  EXTERN struct link_map *_dl_initfirst;

/* MANUAL */
  /* Start time on CPU clock.  */
  //EXTERN hp_timing_t _dl_cpuclock_offset;
  unsigned long long _dl_cpuclock_offset;

  /* Map of shared object to be profiled.  */
  EXTERN struct link_map *_dl_profile_map;

  /* Counters for the number of relocations performed.  */
  EXTERN unsigned long int _dl_num_relocations;
  EXTERN unsigned long int _dl_num_cache_relocations;

  /* List of search directories.  */
  EXTERN struct r_search_path_elem *_dl_all_dirs;

/* MANUAL*/
  //EXTERN void **(*_dl_error_catch_tsd) (void) __attribute__ ((const));
void **(*_dl_error_catch_tsd) (void) __attribute__ ((const));

  /* Structure describing the dynamic linker itself.  We need to
     reserve memory for the data the audit libraries need.  */
  EXTERN struct link_map _dl_rtld_map;
  
/* MANUAL */
//struct auditstate audit_data[DL_NNS];

  EXTERN void (*_dl_rtld_lock_recursive) (void *);
  EXTERN void (*_dl_rtld_unlock_recursive) (void *);

  /* If loading a shared object requires that we make the stack executable
     when it was not, we do it by calling this function.
     It returns an errno code or zero on success.  */
  EXTERN int (*_dl_make_stack_executable_hook) (void **) internal_function;

  /* Prevailing state of the stack, PF_X indicating it's executable.  */
  EXTERN ElfW(Word) _dl_stack_flags;

  /* Flag signalling whether there are gaps in the module ID allocation.  */
  EXTERN bool _dl_tls_dtv_gaps;
  /* Highest dtv index currently needed.  */
  EXTERN size_t _dl_tls_max_dtv_idx;
  /* Information about the dtv slots.  */
  EXTERN struct dtv_slotinfo_list
  {
    size_t len;
    struct dtv_slotinfo_list *next;
    struct dtv_slotinfo
    {
      size_t gen;
      struct link_map *map;
    } slotinfo[0];
  } *_dl_tls_dtv_slotinfo_list;
  /* Number of modules in the static TLS block.  */
  EXTERN size_t _dl_tls_static_nelem;
  /* Size of the static TLS block.  */
  EXTERN size_t _dl_tls_static_size;
  /* Size actually allocated in the static TLS block.  */
  EXTERN size_t _dl_tls_static_used;
  /* Alignment requirement of the static TLS block.  */
  EXTERN size_t _dl_tls_static_align;

/* Number of additional entries in the slotinfo array of each slotinfo
   list element.  A large number makes it almost certain take we never
   have to iterate beyond the first element in the slotinfo list.  */
#define TLS_SLOTINFO_SURPLUS (62)

/* Number of additional slots in the dtv allocated.  */
#define DTV_SURPLUS	(14)

  /* Initial dtv of the main thread, not allocated with normal malloc.  */
  EXTERN void *_dl_initial_dtv;
  /* Generation counter for the dtv.  */
  EXTERN size_t _dl_tls_generation;

  EXTERN void (*_dl_init_static_tls) (struct link_map *);

  EXTERN void (*_dl_wait_lookup_done) (void);

  /* Scopes to free after next THREAD_GSCOPE_WAIT ().  */
  EXTERN struct dl_scope_free_list
  {
    size_t count;
    void *list[50];
  } *_dl_scope_free_list;
};

struct rtld_global_ro
{

  /* If nonzero the appropriate debug information is printed.  */
  EXTERN int _dl_debug_mask;
#define DL_DEBUG_LIBS	    (1 << 0)
#define DL_DEBUG_IMPCALLS   (1 << 1)
#define DL_DEBUG_BINDINGS   (1 << 2)
#define DL_DEBUG_SYMBOLS    (1 << 3)
#define DL_DEBUG_VERSIONS   (1 << 4)
#define DL_DEBUG_RELOC      (1 << 5)
#define DL_DEBUG_FILES      (1 << 6)
#define DL_DEBUG_STATISTICS (1 << 7)
#define DL_DEBUG_UNUSED	    (1 << 8)
#define DL_DEBUG_SCOPES	    (1 << 9)
/* These two are used only internally.  */
#define DL_DEBUG_HELP       (1 << 10)
#define DL_DEBUG_PRELINK    (1 << 11)

  /* OS version.  */
  EXTERN unsigned int _dl_osversion;
  /* Platform name.  */
  EXTERN const char *_dl_platform;
  EXTERN size_t _dl_platformlen;

  /* Cached value of `getpagesize ()'.  */
  EXTERN size_t _dl_pagesize;

  /* Copy of the content of `_dl_main_searchlist' at startup time.  */
  EXTERN struct r_scope_elem _dl_initial_searchlist;

  /* CLK_TCK as reported by the kernel.  */
  EXTERN int _dl_clktck;

  /* If nonzero print warnings messages.  */
  EXTERN int _dl_verbose;

  /* File descriptor to write debug messages to.  */
  EXTERN int _dl_debug_fd;

  /* Do we do lazy relocations?  */
  EXTERN int _dl_lazy;

  /* Nonzero if runtime lookups should not update the .got/.plt.  */
  EXTERN int _dl_bind_not;

  /* Nonzero if references should be treated as weak during runtime
     linking.  */
  EXTERN int _dl_dynamic_weak;

  /* Default floating-point control word.  */
  EXTERN fpu_control_t _dl_fpu_control;

  /* Expected cache ID.  */
  EXTERN int _dl_correct_cache_id;

  /* Mask for hardware capabilities that are available.  */
  EXTERN uint64_t _dl_hwcap;

  /* Mask for important hardware capabilities we honour. */
  EXTERN uint64_t _dl_hwcap_mask;

  /* Get architecture specific definitions.  */
/* MANUAL */
//#define PROCINFO_DECL
//#ifndef PROCINFO_CLASS
//# define PROCINFO_CLASS EXTERN
//#endif
//#include <dl-procinfo.c>
/* MANUAL */
  const char _dl_x86_cap_flags[32][8];
  const char _dl_x86_platforms[4][5];

  /* Names of shared object for which the RPATH should be ignored.  */
  EXTERN const char *_dl_inhibit_rpath;

  /* Location of the binary.  */
  EXTERN const char *_dl_origin_path;

  /* -1 if the dynamic linker should honor library load bias,
     0 if not, -2 use the default (honor biases for normal
     binaries, don't honor for PIEs).  */
  EXTERN ElfW(Addr) _dl_use_load_bias;

  /* Name of the shared object to be profiled (if any).  */
  EXTERN const char *_dl_profile;
  /* Filename of the output file.  */
  EXTERN const char *_dl_profile_output;
  /* Name of the object we want to trace the prelinking.  */
  EXTERN const char *_dl_trace_prelink;
  /* Map of shared object to be prelink traced.  */
  EXTERN struct link_map *_dl_trace_prelink_map;

  /* All search directories defined at startup.  */
  EXTERN struct r_search_path_elem *_dl_init_all_dirs;

/* MANUAL */
  /* Overhead of a high-precision timing measurement.  */
   unsigned long long _dl_hp_timing_overhead;

  /* Syscall handling improvements.  This is very specific to x86.  */
  EXTERN uintptr_t _dl_sysinfo;

  /* The vsyscall page is a virtual DSO pre-mapped by the kernel.
     This points to its ELF header.  */
  EXTERN const ElfW(Ehdr) *_dl_sysinfo_dso;

  /* At startup time we set up the normal DSO data structure for it,
     and this points to it.  */
  EXTERN struct link_map *_dl_sysinfo_map;

  /* We add a function table to _rtld_global which is then used to
     call the function instead of going through the PLT.  The result
     is that we can avoid exporting the functions and we do not jump
     PLT relocations in libc.so.  */
  void (*_dl_debug_printf) (const char *, ...)
       __attribute__ ((__format__ (__printf__, 1, 2)));
  int (internal_function *_dl_catch_error) (const char **, const char **,
					    bool *, void (*) (void *), void *);
  void (internal_function *_dl_signal_error) (int, const char *, const char *,
					      const char *);
  void (*_dl_mcount) (ElfW(Addr) frompc, ElfW(Addr) selfpc);
  lookup_t (internal_function *_dl_lookup_symbol_x) (const char *,
						     struct link_map *,
						     const ElfW(Sym) **,
						     struct r_scope_elem *[],
						     const struct r_found_version *,
						     int, int,
						     struct link_map *);
  int (*_dl_check_caller) (const void *, enum allowmask);
  void *(*_dl_open) (const char *file, int mode, const void *caller_dlopen,
		     Lmid_t nsid, int argc, char *argv[], char *env[]);
  void (*_dl_close) (void *map);
  void *(*_dl_tls_get_addr_soft) (struct link_map *);
  int (*_dl_discover_osversion) (void);

  /* List of auditing interfaces.  */
  struct audit_ifaces *_dl_audit;
  unsigned int _dl_naudit;

  /* 0 if internal pointer values should not be guarded, 1 if they should.  */
  EXTERN int _dl_pointer_guard;
};
