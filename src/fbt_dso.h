/**
 * @file fbt_dso.h
 * Interface to the shared library loader
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
#ifndef FBT_DSO_H
#define FBT_DSO_H

/* forward declare struct */
struct thread_local_data;

/**
 * This enum defines the possible bindings of a symbol in a DSO
 */
enum sh_sym_binding {
  SB_LOCAL, 
  SB_GLOBAL
};


/**
 * Defines a single symbol that is part of the DSO chain.
 * The symbol array per DSO contains the following features.
 */
struct sh_symbol {
  const char* name;              /* name of the symbol */
  unsigned char *start;          /* start pointer of the symbol */
  unsigned long size;            /* size / length of the symbol */
  enum sh_sym_binding binding;   /* binding of the symbol */
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
  
  unsigned char stripped;     /* is the DSO stripped? */
  unsigned char exec;         /* is this the executable? */
  
  struct code_page *pages;    /* list of pages belonging to this DSO */
  long nr_pages;              /* number of pages */
  
  struct sh_symbol *isymbols;   /* array of symbols imported in this object */
  long nr_isymbols;             /* number of imported symbols */

  struct dso_chain *next;     /* next DSO in chain or NULL */
  struct dso_chain *prev;     /* prev DSO in chain or NULL */
};


#define SYMBOL_CACHE_SIZE 12
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
void fbt_check_transfer(struct thread_local_data *tld, unsigned char *src,
                        unsigned char *dst, enum cftx_type type);

/**
 * Interface to the DSO updating mechanism.
 * This function rescans all loaded DSOs using the dl_iterate_phdr interface of
 * the glibC. All DSOs are checked and the current (thread-local) data-structure
 * is updated according to all loaded DSOs and symbols.
 * @param tld Pointer to thread local data.
 */
void fbt_rescan_dsos(struct thread_local_data *tld);

#endif  /* FBT_DSO_H */
