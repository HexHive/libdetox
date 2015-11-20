/**
 * @file fbt_dso.h
 * Interface to the shared library loader.
 * Some constants and file names are defined here.
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * 
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
#ifndef FBT_DSO_H
#define FBT_DSO_H

#include "fbt_syscalls_impl.h"

/* forward declare struct */
struct thread_local_data;

/**
 * This enum defines the possible bindings of a symbol in a DSO
 */
enum sh_sym_binding {
  SB_LOCAL, 
  SB_GLOBAL
};

/* required for a special case check in fbt_is_special_cft_case */
#define RTLD_LOCAL      0
#define RTLD_GLOBAL     0x00100

/**
 * Defines a single symbol that is part of the DSO chain.
 * The symbol array per DSO contains the following features.
 */
struct sh_symbol {
  const char* name;              /* name of the symbol */
  unsigned char *start;          /* start pointer of the symbol */
  unsigned long size;            /* size / length of the symbol */
  enum sh_sym_binding binding;   /* binding of the symbol */
  struct dso_chain* dso;
};


/* Page Macros to set the page size and get
   the page where a symbol is defined in. */
#define PSHIFT 12
#define PSIZE (1<<PSHIFT)
#define PMASK (~(PSIZE-1))

/**
 * Defines a page with executable code that belongs to a DSO.
 */
struct code_page {
  long addr;                  /* Start address of page */
  long nr_symbols;            /* Number of symbols defined in this page */
  struct sh_symbol **syms;    /* Array of symbols */
};


/**
 * This struct is used to define all loaded DSOs that are scanned by libdetox.
 * The linked list is stored in the thread local data and filled during the
 * scan of the DSOs (triggered whenever there is an unknown symbol).
 * TODO: could this be a security bug? (exploit changes data structures of the
 * ld-linux loader and exploits the fact that we call dl_iterate_phdr, this
 * moves dl_iterate_phdr into the trusted computing base).
 */
struct dso_chain {
  const char *name;     /* name of the DSO */
  char *baseaddr;       /* base address of the object */
  char *endaddr;        /* end address of the object */

  char *text;           /* pointer to the text section */
  long text_size;       /* size of the text section */
  char *init;           /* pointer to init section */
  long init_size;       /* size of init section */
  char *fini;           /* pointer to fini section */
  long fini_size;       /* size of fini section */
  
  char *entry;           /* entry point of DSO */

  unsigned char stripped;     /* is the DSO stripped? */
  unsigned char exec;         /* is this the executable? */
  
  struct code_page *pages;    /* list of pages belonging to this DSO */
  long nr_pages;              /* number of pages */
  
  struct sh_symbol *isymbols;   /* array of symbols imported in this object */
  long nr_isymbols;             /* number of imported symbols */

  /* Global Offset Table */
  char *got;
  long got_size;
  
  /* PLT part of GOT */
  char *gotplt;
  long gotplt_size;
  
  /* PLT location and size requiref for CFI checks */
  char *plt;
  long plt_size;

  /* .rodata */
  char *rodata;
  long rodata_size;
  
  /* .rel.dyn */
  char *reldyn;
  long reldyn_size;
  
  /* .rel.plt */
  char *relplt;
  long relplt_size;  
    
  char *callbacks; 				/* array of potential callback targets into this dso */
  long nr_callbacks;			/* nr of potential callback targets */

  int dloaded; /* was this dso dynamically loaded? */
  long dload_mode; /* the mode passed to dlopen when loading this dso */

#if defined(SECURITY_METRICS_AND_STATS)
#define ICF_TABLES_MAX_ENTRIES 100000
  unsigned char **icalls;
  unsigned char **ijmps;
  unsigned char **retinstrs;
  long nr_icalls;
  long nr_ijmps;
  long nr_retinstrs;
  char *callbacks_stats;
#endif

/* The CFT cache will be inlined before each fbt_check_transfer call to verify
 * if the destination is within the cache. This is per indirect branch instruction. */
/* TODO: tune values. */
#if defined(VERIFY_CFTX_ENABLE_CACHE)
  #define CFT_CACHE_SIZE 65536 //16384 /* in bytes -> 4096 entries */
  #define CFT_CACHE_NR_ENTRIES_PER_INSTR 1 /* 1, 2 or 4 */
  char *cft_cache_pool; /* ptr to the cache pool memory chunk */
  char *cft_cache_pool_next; /* ptr to the next free entry */
#endif

/* The CFT fastlookup will use hashtables to verify if a specific destination is a valid target or not. 
 * By using such a table we do not need to go through the entire dso/symbol search for an already validated target. */
#if defined(VERIFY_CFTX_ENABLE_FASTLOOKUP)
  char *cft_valid_call_targets;
  char *cft_valid_jmp_targets;
#endif

#if defined(PRINT_CFTX_CACHE_STATS)
  long nr_cache_hits;
#endif

#if defined(PRINT_CFTX_FASTLOOKUP_STATS)
  long nr_fastlookups;
#endif

#if defined(PRINT_CFTX_CHECK_TRANSFER_STATS)
  long nr_check_transfers_total;
  long nr_check_transfers_slowpath;
#endif

  struct dso_chain *next;     /* next DSO in chain or NULL */
  struct dso_chain *prev;     /* prev DSO in chain or NULL */
};

#if defined(PRINT_CFTX_TYPE_STATS)
long nr_total_cftx_checks;
long nr_icalls_intra_dso;
long nr_icalls_inter_dso;
long nr_icalls_callback;
long nr_icalls_special_case;
long nr_ijmps_intra_dso;
long nr_ijmps_inter_dso;
long nr_ijmps_intra_symbol;
long nr_ijmps_inter_symbol;
long nr_ijmps_callback;
long nr_ijmps_special_case;
long nr_ijmps_not_verified;
long nr_ijmps_within_stripped_symbol;

/* added */
long nr_icalls_not_verified;
long nr_icalls_unimported_symbols;
long nr_icalls_local_symbol;

void fbt_print_cftx_type_stats();
#endif

/* The constants for the call/jmp hash tables. */
/* 403.gcc requires at least >= 13 */
#define CFT_MAPPINGTABLE_NRBITS 15 // nr of bits to address entire table
#define CFT_MAPPINGTABLE_SIZE (0x1<<CFT_MAPPINGTABLE_NRBITS) // size in bytes
#define CFT_MAPPINGTABLE_MAXENTRIES (CFT_MAPPINGTABLE_SIZE>>2) // 4 bytes per entry
#define CFT_MAPPING_PATTERN ((CFT_MAPPINGTABLE_SIZE-1)^0x3) // zero first 2 bits
#define C_CFT_MAPPING_FUNCTION(addr)  ((addr<<2) & CFT_MAPPING_PATTERN)

/* table nrbits has to be >12, nano already needs more then nrbits 12 */
/* table nrbits has to be >13, vim already needs more then nrbits 13 */
/* table nrbits has to be >16, 445.gobmk already needs more then nrbits 16 */
/* TODO: we need a better mapping function */

/* nr of bits to address entire table */
#define CALLBACK_MAPPINGTABLE_NRBITS 23 

#define CALLBACK_MAPPINGTABLE_SIZE (0x1<<CALLBACK_MAPPINGTABLE_NRBITS) // size in bytes
#define CALLBACK_MAPPINGTABLE_MAXENTRIES (CALLBACK_MAPPINGTABLE_SIZE>>2) // 4 bytes per entry
#define CALLBACK_MAPPING_PATTERN ((CALLBACK_MAPPINGTABLE_SIZE-1)^0x3) // zero first 2 bits
#define C_CALLBACK_MAPPING_FUNCTION(addr)  ((addr<<2) & CALLBACK_MAPPING_PATTERN)

#define SYMBOL_CACHE_SIZE 4
struct symbol_cache {
  void *addr;
  struct sh_symbol *symbol;
  struct dso_chain *dso;
};

/**
 * Defines the type of control transfer between two code locations.
 * These types are then used in \ref fbt_actions.c and the emitted code to
 * verify individual transfers. The checker can then decide on this information
 * and according to the transition rules if this transfer is allowed or not.
 */
enum cftx_type {
  CFTX_CALL,  /**< call instruction (relative to IP) */
  CFTX_JMP,  /**< jmp instruction (relative to IP) */
  CFTX_JMP_IND,  /**< indirect jmp instruction through memory pointer */
  CFTX_CALL_IND,  /**< indirect call instruction through memory pointer */
  CFTX_RET  /**< return instruction, control transfer to eip on stack */
};

#if defined(DUMP_CFTX)
/** Data structure used to log control flow transfers */
struct control_flow_transfer_dump {
  /** Type of the control flow transfer */
  enum cftx_type type;
  /** Source address of control flow transfer */
  unsigned long src;
  /** Target address of control flow transfer */
  unsigned long dst;
};
#endif /* DUMP_CFTX */

/* return sh_symbol pointer of the symbol where src resides */
struct sh_symbol* fbt_find_symbol(struct thread_local_data *tld, unsigned char *src);
struct dso_chain* fbt_find_dso(struct thread_local_data *tld, unsigned char *src);
struct sh_symbol* fbt_find_nsymbol(struct thread_local_data *tld, unsigned char *src);

void set_symbol_address(char *so_name, char *name, void *value, void *origvalue, struct dso_chain *dso_objects);

int fbt_addr_is_surrounded_by_symbols(struct dso_chain *srcdso, unsigned char *src, unsigned char *dst);

/**
 * This function checks a specific control transfer from src to dst.
 * This function is called from different translation functions (static control
 * flow transfers) in \ref fbt_actions.c and embedded into the emitted code (for
 * dynamic control flow transfers).
 * @param tld Thread local data
 * @param src Source address of control transfer
 * @param dst Destination address for control transfer
 * @param type Type of control flow transfer
 */
#if defined(ENABLE_TRANSLATION_TIME_SYMBOL_LOOKUP)          
int fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type, struct sh_symbol *src_symbol);
#else
int fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type);
#endif

/* special case handling function */
int fbt_is_special_cft_case(unsigned char *src, struct dso_chain *srcdso, struct sh_symbol *src_symbol,
unsigned char *dst, struct dso_chain *dstdso, struct sh_symbol *dst_symbol, enum cftx_type type);

/**
 * Interface to the DSO updating mechanism.
 * This function rescans all loaded DSOs using the dl_iterate_phdr interface of
 * the glibC. All DSOs are checked and the current (thread-local) data-structure
 * is updated according to all loaded DSOs and symbols.
 * @param tld Pointer to thread local data.
 */
void fbt_rescan_dsos(struct thread_local_data *tld);

/* Memory protection macros */
#ifdef MEMORY_PROTECTION

/* Memory protection flags */
#define FBT_PROT_NONE 0x0   /* Memory cannot be accessed */
#define FBT_PROT_READ 0x1   /* Memory can be read */
#define FBT_PROT_WRITE 0x2  /* Memory can be written to */
#define FBT_PROT_EXEC 0x4   /* Memoryy can be executed */

/* Set data to readonly */
#define FBT_PROT_DATA(data, size, ret, msg) if (size) fbt_mprotect((void *)(data), size, FBT_PROT_READ, ret, msg)
/* Set data to read/write */
#define FBT_UNPROT_DATA(data, size, ret, msg) if (size) fbt_mprotect((void *)(data), size, FBT_PROT_READ|FBT_PROT_WRITE, ret, msg)
#else
/* No memory protection, just ignore */
#define FBT_PROT_DATA(data, size, ret, msg)
#define FBT_UNPROT_DATA(data, size, ret, msg)
#endif /* MEMORY_PROTECTION */

/* callback handling functions */
unsigned long fbt_find_callback(struct dso_chain *dso, void *orig_address);
void fbt_add_callback(struct dso_chain *dso, void *orig_address);

struct sh_symbol* fbt_get_symbol_at_address(struct dso_chain *dso, unsigned char* address);
/* check transfer test function */
int fbt_check_transfer_test(unsigned char* src, struct dso_chain* srcdso, unsigned char* dst, struct dso_chain* dstdso, enum cftx_type type);

#if defined(PRINT_CALLBACKS)
void fbt_print_all_callbacks();
#endif

#if defined(SECURITY_METRICS_AND_STATS)
#define SECMETRICS_FILE_NAME "secmetrics_and_stats.txt"
void fbt_print_security_metrics_and_stats();
unsigned long fbt_get_valid_icall_targets(unsigned char *icall, struct dso_chain* srcdso);
unsigned long fbt_get_valid_ijmp_targets(unsigned char *ijmp, struct dso_chain* srcdso);
unsigned long fbt_get_valid_ret_targets(unsigned char *retinstr, struct dso_chain* dso);	
#endif

#if defined(LOG_VALID_JMP_TARGETS)
void fbt_log_valid_jmp_targets(unsigned char *ijmp);
unsigned long nr_of_logged_jmps;
#define VALID_JMP_TARGETS_FILE_NAME "/tmp/valid_jmp_targets.txt"
#define VALID_JMP_TARGETS_DUMP_FILE_NAME "/tmp/valid_jmp_targets_dump.bin"
#define DUMP_SIZE_JMP_TARGETS 256
#endif

#if defined(LOG_VALID_CALL_TARGETS)
void fbt_log_valid_call_targets(unsigned char *icall);
unsigned long nr_of_logged_calls;
#define VALID_CALL_TARGETS_FILE_NAME "/tmp/valid_call_targets.txt"
#define VALID_CALL_TARGETS_DUMP_FILE_NAME "/tmp/valid_call_targets_dump.bin"
#define DUMP_SIZE_CALL_TARGETS 256
#endif

#if defined(LOG_VALID_RET_TARGETS)
void fbt_log_valid_ret_targets(struct thread_local_data *tld, unsigned char *src, unsigned char *dst, enum cftx_type type);
unsigned long nr_of_logged_rets;
#define VALID_RET_TARGETS_FILE_NAME "/tmp/valid_ret_targets.txt"
#define VALID_RET_TARGETS_DUMP_FILE_NAME "/tmp/valid_ret_targets_dump.bin"
#define DUMP_SIZE_RET_TARGETS 256
#endif

#endif  /* FBT_DSO_H */
