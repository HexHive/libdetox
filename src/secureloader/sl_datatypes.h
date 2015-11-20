/**
 * @file sl_datatypes.h
 * Defines the datatypes used in the secuLoader.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * @author Tobias Hartmann <tobi.hartmann@gmail.com>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
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

#ifndef SL_DATATYPES_H
#define SL_DATATYPES_H

#include <pthread.h>
#include <elf.h>

struct link_map;

/* Library search path */
struct r_search_path_elem {
  /* Next search path */
  struct r_search_path_elem *next;  

  /* Where does the definition come from? */
  const char *what;  
  const char *where; 

  /* Path (ends with slash) */
  const char *dirname;
  unsigned long dirnamelen;
};

/* NOT USED */
struct r_search_path_struct {
  struct r_search_path_elem **dirs;
  long malloced;
};

/* Version of a symbol */
struct version_info {
  const char *name;      /* Name of version */
  Elf32_Word hash;       /* Hash value for fast compare */

  long hidden;           /* Hidden version */
  const char *filename;  /* Name of DSO the version is defined in */
};

/* Dl_info struct used in dladdr */
typedef struct {
  const char *dli_fname;    /* Name of DSO */
  void *dli_fbase;          /* Base address of DSO */
  const char *dli_sname;    /* Symbol name */
  void *dli_saddr;          /* Symbol address */
} Dl_info;

/* Used in iterate_phdr */
struct dl_phdr_info {
  Elf32_Addr dlpi_addr;        /* DSO base address */
  const char *dlpi_name;       /* DSO name */
  const Elf32_Phdr *dlpi_phdr; /* Program header of DSO */
  Elf32_Half dlpi_phnum;       /* Number of entries in program header */
};

/* NOT USED */
struct r_scope_elem {
  /* Array of maps for the scope */
  struct link_map **r_list;
  
  /* Number of entries in the scope */
  unsigned long r_nlist;
};

/* A data structure for a simple single linked list of strings.  */
struct libname_list
  {
    const char *name;           /* Name requested (before search).  */
    struct libname_list *next;  /* Link to next name for this object.  */
    int dont_free;              /* Flag whether this element should be freed
                                   if the object is not entirely unloaded.  */
  };

struct link_map_machine
  {
    /* empty by default */
  };

/* NOT USED */
struct auditstate {
  uintptr_t cookie;
  unsigned long bindflags;
};

/*

DT_SOME_VALUE = DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGNUM
		      + DT_EXTRANUM + DT_VALNUM + DT_ADDRNUM
*/
/* seems to be 76! -> if this is wrong the entire shared_object structure will not match link_map of libc */
#define DT_SOME_VALUE 76

/* This type represents a Dynamic Shared Object */
struct shared_object {

  /* DO NOT CHANGE: libc depends on these first 5 members */  

  void *base_addr;               /* Base address (0 for main file) */
  const char *name;	         /* File name of the object */
  Elf32_Dyn *dynamic_section;    /* Dynamic section */
  struct shared_object *next;    /* Next object in chain */
  struct shared_object *prev;    /* Previous object in chain  */

  /* We still need more elements that correspond to the specific link_map structure in libc. libc uses direct calls to dl* functions
   * like dl_iterate_phdr. This function expect a certain structure that goes beyond the 5 elements here. */
  
  /* this was missing and caused a segfault */
  struct shared_object *l_real;
    
  /* (The following may change) */
		/* Number of the namespace this link map belongs to.  */
		long l_ns;
		struct libname_list *l_libname;
		Elf32_Dyn *l_info[DT_SOME_VALUE];
		
		//const Elf32_Phdr *l_phdr;	/* Pointer to program header table in core.  */
  /* Program header and number of entries */
  Elf32_Phdr *program_header;

		Elf32_Addr l_entry;		/* Entry point location.  */
		
		//Elf32_Half l_phnum;		/* Number of program header entries.  */
  Elf32_Half program_header_num;

		Elf32_Half l_ldnum;		/* Number of dynamic segment entries.  */
		struct r_scope_elem l_searchlist;
		struct r_scope_elem l_symbolic_searchlist;
  
  /* DSO which loaded this object (NULL for main) */
  struct shared_object *loader;
    
		struct r_found_version *l_versions;
		unsigned int l_nversions;
        /* Symbol hash table.  */
		unsigned int l_nbuckets;
		Elf32_Word l_gnu_bitmask_idxbits;
		Elf32_Word l_gnu_shift;
		const Elf32_Addr *l_gnu_bitmask;
		union
		{
		  const Elf32_Word *l_gnu_buckets;
		  const unsigned int *l_chain;
		};
		union
		{
		  const Elf32_Word *l_gnu_chain_zero;
		  const unsigned int *l_buckets;
		};
		unsigned int l_direct_opencount; /* Reference count for dlopen/dlclose.  */
		enum			/* Where this object came from.  */
		  {
			lt_executable,		/* The main executable program.  */
			lt_library,		/* Library needed by main executable.  */
			lt_loaded		/* Extra run-time loaded shared object.  */
		  } l_type:2;
		unsigned int l_relocated:1;	/* Nonzero if object's relocations done.  */
		unsigned int l_init_called:1; /* Nonzero if DT_INIT function called.  */
		unsigned int l_global:1;	/* Nonzero if object in _dl_global_scope.  */
		unsigned int l_reserved:2;	/* Reserved for internal use.  */
		unsigned int l_phdr_allocated:1; /* Nonzero if the data structure pointed
						to by `l_phdr' is allocated.  */
		unsigned int l_soname_added:1; /* Nonzero if the SONAME is for sure in
						  the l_libname list.  */
		unsigned int l_faked:1;	/* Nonzero if this is a faked descriptor
					   without associated file.  */
		unsigned int l_need_tls_init:1; /* Nonzero if GL(dl_init_static_tls)
						   should be called on this link map
						   when relocation finishes.  */
		unsigned int l_auditing:1;	/* Nonzero if the DSO is used in auditing.  */
		unsigned int l_audit_any_plt:1; /* Nonzero if at least one audit module
						   is interested in the PLT interception.*/
		unsigned int l_removed:1;	/* Nozero if the object cannot be used anymore
					   since it is removed.  */
		unsigned int l_contiguous:1; /* Nonzero if inter-segment holes are
						mprotected or if no holes are present at
						all.  */
		unsigned int l_symbolic_in_local_scope:1; /* Nonzero if l_local_scope
							 during LD_TRACE_PRELINKING=1
							 contains any DT_SYMBOLIC
							 libraries.  */

		/* Collected information about own RPATH directories.  */
		struct r_search_path_struct l_rpath_dirs;

		/* Collected results of relocation while profiling.  */
		struct reloc_result
		{
		  void* addr;
		  struct link_map *bound;
		  unsigned int boundndx;
		  uint32_t enterexit;
		  unsigned int flags;
		} *l_reloc_result;
		/* Pointer to the version information if available.  */
		Elf32_Versym *l_versyms;

		/* String specifying the path where this object was found.  */
		const char *l_origin;

		/* Start and finish of memory map for this object.  l_map_start
		   need not be the same as l_addr.  */
		Elf32_Addr l_map_start, l_map_end;
		/* End of the executable part of the mapping.  */
		Elf32_Addr l_text_end;

		/* Default array for 'l_scope'.  */
		struct r_scope_elem *l_scope_mem[4];
		/* Size of array allocated for 'l_scope'.  */
		size_t l_scope_max;
		/* This is an array defining the lookup scope for this link map.
		   There are initially at most three different scope lists.  */
		struct r_scope_elem **l_scope;

		/* A similar array, this time only with the local scope.  This is
		   used occasionally.  */
		struct r_scope_elem *l_local_scope[2];

		/* This information is kept to check for sure whether a shared
		   object is the same as one already loaded.  */
		//dev_t l_dev;
		unsigned int l_dev;
		//ino64_t l_ino;
		long long l_ino;
		
		/* Collected information about own RUNPATH directories.  */
		struct r_search_path_struct l_runpath_dirs;

		/* List of object in order of the init and fini calls.  */
		struct link_map **l_initfini;
		/* The init and fini list generated at startup, saved when the
		   object is also loaded dynamically.  */
		struct link_map **l_orig_initfini;

		/* List of the dependencies introduced through symbol binding.  */
		struct link_map_reldeps
		  {
			unsigned int act;
			struct link_map *list[];
		  } *l_reldeps;
		unsigned int l_reldepsmax;

		/* Nonzero if the DSO is used.  */
		unsigned int l_used;

		/* Various flag words.  */
		Elf32_Word l_feature_1;
		Elf32_Word l_flags_1;
		Elf32_Word l_flags;

		/* Temporarily used in `dl_close'.  */
		int l_idx;

		struct link_map_machine l_mach;

		struct
		{
		  const Elf32_Sym *sym;
		  int type_class;
		  struct link_map *value;
		  const Elf32_Sym *ret;
		} l_lookup_cache;

  /* TLS specific data */  
  void *tls_initimage;                /* Address of tls init image */
  unsigned long tls_initimage_size;   /* Size of tls init image */
  unsigned long tls_blocksize;        /* Size of the entire tls block */
  unsigned long tls_align;            /* Alignment of the TLS block */
  
		/* Thread-local storage related info.  */
		/* Start of the initialization image.  */
		//void *l_tls_initimage;
		/* Size of the initialization image.  */
		//unsigned int l_tls_initimage_size;
		/* Size of the TLS block.  */
		//unsigned int l_tls_blocksize;
		/* Alignment requirement of the TLS block.  */
		//unsigned int l_tls_align;
		/* Offset of first byte module alignment.  */
		unsigned int l_tls_firstbyte_offset;
		/* For objects present at startup time: offset in the static TLS block.  */
		//unsigned int l_tls_offset;
		/* Index of the module in the dtv array.  */
		//unsigned int l_tls_modid;

  unsigned long tls_offset;           /* Offset in the static TLC block */
  unsigned long tls_modid;            /* Index of the module in the dtv */
  
		/* Information used to change permission after the relocations are
		   done.  */
		Elf32_Addr l_relro_addr;
		unsigned int l_relro_size;

		unsigned long long int l_serial;

		/* Audit information.  This array apparent must be the last in the
		   structure.  Never add something after it.  */
		struct auditstate l_audit[0];
    		
  /* Path where this DSO was found */
  const char *path;

  /* Program header and number of entries */
  //Elf32_Phdr *program_header;
  //Elf32_Half program_header_num;

  /* Number of references from other DSOs */
  unsigned long ref_count;

  /* Direct dependend DSOs */
  struct shared_object **deps;
  unsigned long deps_count;

  /* Library search paths */
  struct r_search_path_elem *search_path;

  /* Local search scope */
  struct shared_object **lscope;
  long lscope_num;

  /* Global search scope */
  struct shared_object *gs_next;
  struct shared_object *gs_prev;  

  /* Text section */
  void *text_addr;
  long text_size;

  /* Data section */
  void *data_addr;
  long data_size;

  /* Bss section */
  void *bss_addr;
  long bss_size;

  /* End address of DSO */
  void *end_addr;

  /* Entry point address */
  void *entry;

  /* Type of DSO (e_type of ELF header) */
  Elf32_Half type;

  /* Dynamic symbol_table */
  Elf32_Sym *symbol_table;

  /* Dynamic string table */
  const char *string_table;

  /* Hash tables */
  Elf32_Word *hash_table;
  Elf32_Word *gnu_hash_table;

  /* Symbol version definitions in this DSO */
  Elf32_Verdef *verdef;
  unsigned long verdef_num;

  /* Symbol versions needed by this DSO */
  Elf32_Verneed *verneed;
  unsigned long verneed_num;

  /* Symbol version table */
  Elf32_Half *versym;

  /* Versions array */
  struct version_info *versions;

  /* Relocation table */
  Elf32_Rel *rel;
  long relsz;

  /* PLT relocations */
  Elf32_Rel *pltrel;
  long pltrelsz;

  /* GNU_RELRO */
  Elf32_Addr relro;
  long relro_size;

  /* Procedure Linkage Table */
  char *plt;
  long plt_size;

  /* Global Offset Table */
  char *got;
  long got_size;

  /* PLT part of GOT */
  char *gotplt;
  long gotplt_size;
  
  /* Initialization and finalization functions */
  void (*init)(int, char**, char**);
  Elf32_Addr *init_array;
  long init_array_sz;
  long is_init;

  void (*fini)();
  Elf32_Addr *fini_array;
  long fini_array_sz;

  /* Flags */
  Elf32_Word flags;
  Elf32_Word flags_1;

  ///* TLS specific data */  
  //void *tls_initimage;                /* Address of tls init image */
  //unsigned long tls_initimage_size;   /* Size of tls init image */
  //unsigned long tls_blocksize;        /* Size of the entire tls block */
  //unsigned long tls_align;            /* Alignment of the TLS block */

  //unsigned long tls_offset;           /* Offset in the static TLC block */
  //unsigned long tls_modid;            /* Index of the module in the dtv */
  
  /* "Static" string table */
  char *st_strtab;
  unsigned long st_strtab_size;  
  /* Libdetox internal chain of DSOs */
  struct dso_chain *dso;

#ifdef MEMORY_PROTECTION
  long readonly;
#endif

  long text_prot;
  long data_prot;
  long bss_prot;
  int ispie;
};

typedef struct shared_object dso;

/* Dynamic Thread Vector (DTV) */
typedef union {
  unsigned long counter;         /* Generation of this dtv array */
  struct {
    void *val;                   /* Pointer to tls block */
    long is_static;              /* Is this a static block? */
    long size;                   /* Size of tls block */
  } pointer;
} dynamic_thread_vector;

/* Thread Control Block (TCB) */
typedef struct {
  void *tcb;                     /* Pointer to this structure */
  dynamic_thread_vector *dtv;    /* Array of dynamic thread vectors */
  void *self;                    /* Thread's control port */
  long multiple_threads;         /* Do we have more than one thread? */
  unsigned long sysinfo;         /* Syscall trampoline to int80 */
  unsigned long stack_guard;     /* Random value for gnu stack canary */
  unsigned long point_guard;     /* Random value for gnu pointer guard */
  long gscope_flag;              
  long unused1;
  void *__private_tm[5];
} thread_control_block;

/* Information about tls entry */
struct tls_index {
  unsigned long ti_module;       /* Module index */
  unsigned long ti_offset;       /* Offset in tls block */
};

/* Segment descriptor for the tls segment */
struct user_desc {
  unsigned long entry_number;    
  unsigned long base_addr;
  unsigned long limit;
  unsigned long seg_32bit:1;
  unsigned long contents:2;
  unsigned long read_exec_only:1;
  unsigned long limit_in_pages:1;
  unsigned long seg_not_present:1;
  unsigned long useable:1;
  unsigned long empty:25;
};

/* Used to save registers in sl_longjmp and sl_setjmp */
struct jump_buffer {
  long regs[6];
};

/* This structure communicates state between _dl_catch_error and
   _dl_signal_error */
struct catch {
  const char *objname;     /* Name of DSO the error occured in */
  const char *errstring;   /* Error string */
  long malloced;           /* String malloced by libc ? */
  struct jump_buffer buf;  /* Longjmp to this address on error */
};

/* This is the linux loader internal representation of a DSO */
struct link_map {
  Elf32_Addr l_addr;    /* Base address shared object is loaded at.  */
  char *l_name;         /* Absolute file name object was found in.  */
  Elf32_Dyn *l_ld;      /* Dynamic section of the shared object.  */
  struct link_map *l_next, *l_prev; /* Chain of loaded objects.  */

  /* This is a hack to fill the space of link_map we do not use because it is linux loader internal stuff. But we need it as we have to support rtld_global and rtld_global_ro (see below) and without this the offsets in these structures would be wrong... */
#ifdef LIBC2_9
  char placeholder[576];
#else
  char placeholder[580];
#endif
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

/* Bit masks for the objects which valid callers can come from to
   functions with restricted interface */
enum allowmask {
  allow_libc = 1,
  allow_libdl = 2,
  allow_libpthread = 4,
  allow_ldso = 8
};

/* Libc lock */
typedef struct {
  pthread_mutex_t mutex;
} __rtld_lock_recursive_t;

/* Represents a function exported by secuLoader */
struct function {
  char *name;  
  void *addr;  
};

/* Represents a variable exported by secuLoader */
struct variable {
  char *name;
  void **addr;
  unsigned char deref; /* Do we have to dereference once? */
};

/* This struct is normally linux loader internal but needed by the libc, libdl and others, so we have to support it. See ldsodefs.h of glibc. */
struct rtld_global {
# define DL_NNS 16

  struct link_namespaces {
    /* A pointer to the map for the main map. */
    struct link_map *_ns_loaded;
    /* Number of object in the _dl_loaded list. */
    unsigned long _ns_nloaded;
    /* Direct pointer to the searchlist of the main object. */
    struct r_scope_elem *_ns_main_searchlist;
    /* This is zero at program start to signal that the global scope map is
       allocated by rtld. Later it keeps the size of the map. It might be
       reset if in _dl_close if the last global object is removed. */
    unsigned long _ns_global_scope_alloc;

#ifndef LIBC2_9
    /* Search table for unique objects. */
    struct unique_sym_table {
      __rtld_lock_recursive_t lock;
      struct unique_sym {
	uint32_t hashval;
	const char *name;
	const Elf32_Sym *sym;
	struct link_map *map;
      } *entries;
      unsigned long size;
      unsigned long n_elements;
      void (*free) (void *);
    } _ns_unique_sym_table;

#endif /* LIBC2_9 */

    /* Keep track of changes to each namespace' list. */
    struct r_debug _ns_debug;
  } _dl_ns[DL_NNS];

#ifndef LIBC2_9
  /* One higher than index of last used namespace. */
  unsigned long _dl_nns;
#endif /* LIBC2_9 */

  __rtld_lock_recursive_t _dl_load_lock;
  
#ifndef LIBC2_9
  __rtld_lock_recursive_t _dl_load_write_lock;
#endif /* LIBC2_9 */
  
  /* Incremented whenever something may have been added to dl_loaded. */
  unsigned long long _dl_load_adds;

  /* The object to be initialized first. */
  struct link_map *_dl_initfirst;

  /* Start time on CPU clock. */
  unsigned long long _dl_cpuclock_offset;

  /* Map of shared object to be profiled. */
  struct link_map *_dl_profile_map;
  
  /* Counters for the number of relocations performed. */
  unsigned long _dl_num_relocations;
  unsigned long _dl_num_cache_relocations;

  /* List of search directories. */
  struct r_search_path_elem *_dl_all_dirs;

  void **(*_dl_error_catch_tsd) (void) __attribute__ ((const));

  /* Structure describing the dynamic linker itself. We need to
     reserve memory for the data the audit libraries need. */
  struct link_map _dl_rtld_map;
  
  struct auditstate audit_data[DL_NNS];

  void (*_dl_rtld_lock_recursive) (void *);
  void (*_dl_rtld_unlock_recursive) (void *);

  /* If loading a shared object requires that we make the stack executable
     when it was not, we do it by calling this function.
     It returns an errno code or zero on success. */
  long (*_dl_make_stack_executable_hook) (void **) __attribute__((regparm (1)));

  /* Prevailing state of the stack, PF_X indicating it's executable. */
  Elf32_Word _dl_stack_flags;

  /* Flag signalling whether there are gaps in the module ID allocation. */
  long _dl_tls_dtv_gaps;
  /* Highest dtv index currently needed. */
  unsigned long _dl_tls_max_dtv_idx;
  /* Information about the dtv slots. */
  struct dtv_slotinfo_list {
    unsigned long len;
    struct dtv_slotinfo_list *next;
    
    struct dtv_slotinfo {
      unsigned long gen;
      struct shared_object *map;
    } slotinfo[0];
    
  } *_dl_tls_dtv_slotinfo_list;

  /* Number of modules in the static TLS block. */
  unsigned long _dl_tls_static_nelem;
  /* Size of the static TLS block. */
  unsigned long _dl_tls_static_size;
  /* Size actually allocated in the static TLS block. */
  unsigned long _dl_tls_static_used;
  /* Alignment requirement of the static TLS block. */
  unsigned long _dl_tls_static_align;

  /* Initial dtv of the main thread, not allocated with normal malloc. */
  void *_dl_initial_dtv;
  /* Generation counter for the dtv. */
  unsigned long _dl_tls_generation;

  void (*_dl_init_static_tls) (struct link_map *);

  void (*_dl_wait_lookup_done) (void);

  /* Scopes to free after next THREAD_GSCOPE_WAIT (). */
  struct dl_scope_free_list
  {
    unsigned long count;
    void *list[50];
  } *_dl_scope_free_list;

};

/* This struct is normally linux loader internal but needed by the libc, libdl and others, so we have to support it. See ldsodefs.h of glibc. */
struct rtld_global_ro {

  /* If nonzero the appropriate debug information is printed. */
  long _dl_debug_mask;

  /* OS version. */
  unsigned long _dl_osversion;

  /* Platform name. */
  const char *_dl_platform;
  unsigned long _dl_platformlen;

  /* Cached value of `getpagesize ()'. */
  unsigned long _dl_pagesize;

  /* Copy of the content of `_dl_main_searchlist' at startup time. */
  struct r_scope_elem _dl_initial_searchlist;

  /* CLK_TCK as reported by the kernel. */
  long _dl_clktck;

  /* If nonzero print warnings messages. */
  long _dl_verbose;

  /* File descriptor to write debug messages to. */
  long _dl_debug_fd;

  /* Do we do lazy relocations? */
  long _dl_lazy;

  /* Nonzero if runtime lookups should not update the .got/.plt. */
  long _dl_bind_not;

  /* Nonzero if references should be treated as weak during runtime
     linking. */
  long _dl_dynamic_weak;

  /* Default floating-point control word. */
  unsigned long _dl_fpu_control;

  /* Expected cache ID. */
  long _dl_correct_cache_id;

  /* Mask for hardware capabilities that are available. */
  unsigned long long int _dl_hwcap;

  /* Mask for important hardware capabilities we honour. */
  unsigned long long int _dl_hwcap_mask;

  /* Get architecture specific definitions. */
  const char _dl_x86_cap_flags[32][8];
  const char _dl_x86_platforms[4][5];

  /* Names of shared object for which the RPATH should be ignored. */
  const char *_dl_inhibit_rpath;

  /* Location of the binary. */
  const char *_dl_origin_path;

  /* -1 if the dynamic linker should honor library load bias,
     0 if not, -2 use the default (honor biases for normal
     binaries, don't honor for PIEs). */
  Elf32_Addr _dl_use_load_bias;

  /* Name of the shared object to be profiled (if any). */
  const char *_dl_profile;

  /* Filename of the output file. */
  const char *_dl_profile_output;

  /* Name of the object we want to trace the prelinking. */
  const char *_dl_trace_prelink;

  /* Map of shared object to be prelink traced. */
  struct link_map *_dl_trace_prelink_map;

  /* All search directories defined at startup. */
  struct r_search_path_elem *_dl_init_all_dirs;

  /* Overhead of a high-precision timing measurement. */
  unsigned long long _dl_hp_timing_overhead;

  /* Syscall handling improvements. This is very specific to x86. */
  uintptr_t _dl_sysinfo;

  /* The vsyscall page is a virtual DSO pre-mapped by the kernel.
     This points to its ELF header. See dl-sysdep.h */
  const Elf32_Ehdr *_dl_sysinfo_dso;

  /* At startup time we set up the normal DSO data structure for it,
     and this points to it. */
  struct link_map *_dl_sysinfo_map;


  /* We add a function table to _rtld_global which is then used to
     call the function instead of going through the PLT.  The result
     is that we can avoid exporting the functions and we do not jump
     PLT relocations in libc.so. */
  void (*_dl_debug_printf) (const char *, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));

  long (*_dl_catch_error) (const char **,
                          const char **,
                          long *,
                          void (*) (void *),
                          void *) __attribute__ ((regparm (3)));
  
  void (*_dl_signal_error) (long,
                            const char *,
                            const char *,
                            const char *) __attribute__ ((regparm (3)));
  
  void (*_dl_mcount) (Elf32_Addr frompc, Elf32_Addr selfpc);
  
  struct link_map * (*_dl_lookup_symbol_x) (const char *,
                                   struct link_map *,
                                   const Elf32_Sym **,
                                   struct r_scope_elem *[],
                                   const struct version_info *,
                                   long,
                                   long,
                                   struct link_map *)
    __attribute__ ((regparm (3)));
  
  long (*_dl_check_caller) (const void *, enum allowmask);
  
  void *(*_dl_open) (const char *file,
                     long mode,
                     const void *caller_dlopen,
		     long nsid,
                     long argc,
                     char *argv[],
                     char *env[]);
  
  void (*_dl_close) (void *map);

  void *(*_dl_tls_get_addr_soft) (struct link_map *);

  long (*_dl_discover_osversion) (void);
  
  /* List of auditing interfaces. */
  struct audit_ifaces *_dl_audit;
  unsigned long _dl_naudit;

  /* 0 if internal pointer values should not be guarded, 1 if they should. */
  long _dl_pointer_guard;
};

/* This is needed for the fstat syscall */
struct my_timespec {
  long tv_sec; 
  long tv_nsec;
};

struct kernel_stat {
  unsigned short int st_dev;
  unsigned short int __pad1;
  unsigned long int st_ino;
  unsigned short int st_mode;
  unsigned short int st_nlink;
  unsigned short int st_uid;
  unsigned short int st_gid;
  unsigned short int st_rdev;
  unsigned short int __pad2;
  unsigned long int st_size;
  unsigned long int st_blksize;
  unsigned long int st_blocks;
  struct my_timespec st_atim;
  struct my_timespec st_mtim;
  struct my_timespec st_ctim;
  unsigned long int __unused4;
  unsigned long int __unused5;
};

#endif
