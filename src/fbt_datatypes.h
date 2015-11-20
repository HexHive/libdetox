/**
 * @file fbt_datatypes.h
 * Datatypes used in the BT
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-01-21 13:23:02 +0100 (Sat, 21 Jan 2012) $
 * $LastChangedDate: 2012-01-21 13:23:02 +0100 (Sat, 21 Jan 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1201 $
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
#ifndef FBT_DATATYPES_H
#define FBT_DATATYPES_H

#if defined(SEL_DEBUG)
  #include "ipc.h"
#endif

typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long ulong_t;

/* forward declare these structs */
struct mem_info;
struct trampoline;
struct ia32_opcode;
struct dso_chain;
#if defined(SHARED_DATA)
struct shared_data;
#include "fbt_mutex.h"
#endif

#if defined(TRACK_BASIC_BLOCKS)
struct basic_block_node;
struct thread_local_data;
typedef void stop_thread_callback_t(struct thread_local_data *tld, void *context);
#endif

#if defined(TRACK_CFTX)
struct control_flow_transfer;
#endif /* TRACK_CFTX */

#ifdef ONLINE_PATCHING
struct patching_information;
#endif

#if defined(VERIFY_CFTX)
struct symbol_cache;
#endif

#if defined(ICF_PREDICT)
struct icf_prediction;
#endif  /* ICF_PREDICT */
#if defined(DYNARACE)
struct dynarace_file;
#endif  /* DYNARACE */

/**
 * This struct is used when a new instruction is parsed and translated.
 * The struct gets updated through the disassembling function and the
 * information is then consumed by the action function (that handles the
 * opcode).
 */
struct translate {
  /** points to the current position in the code cache */
  unsigned char *transl_instr;
  /** points to the end of the code cache */
  unsigned char *code_cache_end;
  /** list of unused trampolines */
  struct trampoline *trampos;
  /** pointer to the instruction that is currently being translated */
  unsigned char *cur_instr;
  /** information about the current instruction (or NULL) */
  const struct ia32_opcode *cur_instr_info;
  /** pointer into the instruction to the first byte of the data/imm values */
  unsigned char *first_byte_after_opcode;
  /** number of prefixes for this instruction */
  unsigned char num_prefixes;
  /** operand sizes (dest, src, and aux) for this instruction */
  unsigned char dest_operand_size;
  unsigned char src_operand_size;
  unsigned char aux_operand_size;
  /** pointer to the next instruction (only valid after decoding) */
  unsigned char *next_instr;
#if defined(INLINE_CALLS)
  /** Pointer to the return address of the upper call if we are currently
      inlining, NULL otherwise. */
  void *inline_call_RIP;
#endif

#if defined(ONLINE_PATCHING)
  /** The original address at which we are replacing code during online patching */
  unsigned char *original_addr;

  /** Stores the next instruction, which might be the address of a patched instruction */
  unsigned char *next_patch_instr;
#endif
  /** pointer back to tld (for action functions) */
  struct thread_local_data *tld;
};

#if defined(HANDLE_SIGNALS)
#define MAX_NR_SIGNALS 64 /**< max number of signals */

struct fbt_siginfo;

struct fbt_sigaction {
  void     (*sigaction)(int, struct fbt_siginfo *, void *);
  ulong_t    mask;
  int        flags;
  void     (*restorer)(void);
};

typedef signed int fbt_pid_t;
typedef unsigned int fbt_uid_t;

union fbt_sigval {
  int sival_int;
  void *sival_ptr;
};

typedef struct fbt_siginfo {
  int signo;
  int code;
  int errnox;
  fbt_pid_t pid;
  fbt_uid_t uid;
  union fbt_sigval value;
} fbt_siginfo_t;

#endif  /* HANDLE_SIGNALS */
  
#if defined(AUTHORIZE_SYSCALLS)
/**
 * Possible values for the system call authorization.
 */
enum syscall_auth_response {
  /** syscall authorization granted, execute syscall */
  SYSCALL_AUTH_GRANTED,
  /** syscall execution denied, return fake value */
  SYSCALL_AUTH_FAKE,
  /** syscall rejected */
  SYSCALL_AUTH_DENIED
} __attribute__((packed));

#if defined(SYSCALL_POLICY_FILE)

/**
 * Possible argument types for each parameter to a system call
 */
enum syscall_argument_type {
  /** an integer value */
  SYSCALL_ARG_INT,
  /** pointer to unspecified data */
  SYSCALL_ARG_POINTER,
  /** a string, \0 terminated */
  SYSCALL_ARG_STRING,
  /** a path (used by file-related system calls) */
  SYSCALL_ARG_PATH,
  /** ignored */
  SYSCALL_ARG_IGNORE
} __attribute__((packed));

/**
 * Encoding for a single parameter of a system call
 */
struct syscall_argument {
  /** type, e.g., int, pointer, or string data */
  enum syscall_argument_type type;
  union {
    ulong_t int_value;
    void *pointer_value;
    char *string_value;
  } data;
};

/**
 * An single policy entry for a specific system call
 */
struct syscall_policy_entry {
  /** valid argument types for all potential syscall parameters */
  struct syscall_argument *args[6];
  /** what should we do if this policy rule matches? */
  enum syscall_auth_response action;
  ulong_t fake_value;
  /** next argument combination in list */
  struct syscall_policy_entry *next;
};
#endif  /* SYSCALL_POLICY_FILE */
#endif  /* AUTHORIZE_SYSCALLS */

#if defined(TRACK_INSTRUCTIONS)
#define TRACK_INSTRUCTIONS_PAGES_PER_LEVEL \
  ((256 * sizeof(ulong_t) + PAGESIZE - 1) / PAGESIZE)
#endif /* TRACK_INSTRUCTIONS */

/**
 * This struct defines thread local data that is needed inside the BT.
 * These fields are set during the startup of the BT and then used whenever
 * a new block of code is translated.
 */
struct thread_local_data {
  /** mapping table between code cache and program */
  void *mappingtable;
  /** this pointer points to a target in the code cache. optimizations and
      stack guards use this variable to hide locations from the translated
      program. trampolines use this value as a branch target */
  void *ind_target;
  /** this trampoline dereferences ind_target and transfers the control flow to
     the specified code */
  void *ret2app_trampoline;
  /** trampolines set up the secure stack and the unmanaged_code_trampoline
      handles the translation and lookup of the code and transfer the control
      back to the newly translated code */
  void *unmanaged_code_trampoline;
  /** this trampoline points to a fast, thread-local version of the indirect
      jump optimization that looks up a given PC in the mapping table and
      transfers control to the translated fragment. */
  void *opt_ijump_trampoline;
  /** same as opt_ijump_trampoline but for indirect calls */
  void *opt_icall_trampoline;
  /** this trampoline is used for plain return instructions */
  void *opt_ret_trampoline;
  /** this trampoline is used for return instructions that additionally pop a
      couple of bytes from the stack */
  void *opt_ret_remove_trampoline;
    
#if defined(ICF_PREDICT)
  /** trampoline that handles a misprediction of an indirect control flow
      transfer using an indirect jump instruction. */ 
  void *opt_ijump_predict_fixup;
  /** trampoline that handles a misprediction of an indirect control flow
      transfer using an indirect call instruction. */ 
  void *opt_icall_predict_fixup;
  /** Pointer to a free-list of icf predictions that can be used during the
      translation of new indirect control flow transfer locations. */
  struct icf_prediction *icf_predict;
#endif
#if defined(SHADOWSTACK)
  /** top of the stack, highest addr of the stack, next addr is free */
  struct shadowstack_entry *top_of_shadowstack;
  /** pointer to the base of the shadow stack (grows to higher addrs) */
  struct shadowstack_entry *shadowstack;
  /** special value whose address denotes end of shadow stack */
  ulong_t shadowstack_end;
  /** address of return instruction that has last been executed */
  ulong_t shadowstack_return_origin;
  /** start address of loader DSO */
  ulong_t shadowstack_loader_begin;
  /** end address of loader DSO */
  ulong_t shadowstack_loader_end;

  // ==== vsyscall related ====

  /** Counter to keep track of number of items seen in dl_iterate_phdr */
  ulong_t dl_iterate_phdr_counter;  

  /** Start of the vsyscall page */
  ulong_t vsyscall_page_begin;

  /** End of vsyscall page */ 
  ulong_t vsyscall_page_end;

#endif
#if defined(AUTHORIZE_SYSCALLS)
  /** the table with pointers to syscall authorization handlers */
  enum syscall_auth_response (**syscall_table)(struct thread_local_data*,
                                               ulong_t, ulong_t, ulong_t,
                                               ulong_t, ulong_t, ulong_t,
                                               ulong_t*, ulong_t, ulong_t*);
  /** location of the system call in the original program */
  void *syscall_location;
#if defined(SYSCALL_POLICY_FILE)
  /** specifies an array of handlers that take care of application signals. The
     pointers either point to an abort routine, to a trampoline or to a
     translated code region */
  struct syscall_policy_entry **policy_entries;
#endif  /* SYSCALL_POLICY_FILE */
#if defined(HANDLE_SIGNALS)
  /** specifies an array of handlers that take care of application signals. The
     pointers either point to an abort routine, to a trampoline or to a
     translated code region */
  //struct trampoline **signal_handlers;
  struct fbt_sigaction signals[MAX_NR_SIGNALS];

  /** Trampoline used as handler for signals that might be both user signals
      or internal signals (used by libdetox to communicate) */
  void *signal_trampoline;

#endif  /* HANDLE_SIGNALS */

#endif  /* AUTHORIZE_SYSCALLS */
  /** trampoline to handle sysenter instructions in a safe manner */
  void *sysenter_trampoline;
#if defined(AUTHORIZE_SYSCALLS)
  /** trampoline to handle int80 instructions in a safe manner. This trampoline
     is only needed if we authorize system calls */
  void *int80_trampoline;

  /** Trampoline used when starting a new thread so we can execute code in
    * the libdetox context before starting thread execution */
  void *bootstrap_thread_trampoline;
    
#endif  /* AUTHORIZE_SYSCALLS */
#if defined(VERIFY_CFTX)
  struct dso_chain *dso_objects;
  struct symbol_cache *symb_cache;
#if defined(DUMP_CFTX)
  /** File identifier used to dump control flow transfers on a
    * thread local level */
  long dump_cftx_file;  
#endif /* DUMP_CFTX */  
#endif  /* VERIFY_CFTX */
#if defined(DYNARACE)
  /** free list with dynarace_file elements, used for the race protection */
  struct dynarace_file *dynfile_free;
  /** list with cached file stats */
  struct dynarace_file *dynfile_cache;
#endif  /* DYNARACE */
  /** safe stack for the BT */
  ulong_t *stack;
  /** all allocated memory */
  struct mem_info *chunk;
  /** pointer to memory that can be used through the fbt_smalloc allocator */
  void *smalloc;
  /** amount of memory left available at smalloc above */
  long smalloc_size;  
  /** translation information for the current instruction that is currently being
      translated. */
  struct translate trans;
#if defined(SEL_DEBUG)
  struct sdbg_thread_local_data *sdbg;
#endif

#ifdef TRACK_BASIC_BLOCKS
  /**
   * A linked list containing all the basic blocks created by the current 
   * thread
   */
  struct basic_block_node *basic_blocks;  
#endif /* TRACK_BASIC_BLOCK */

#if defined(TRACK_INSTRUCTIONS)
  /** First page of a multilevel datastructure keeping track of all
   * translated -> original instruction mappings. Each page maps
   * 2 bytes of the input address (256 entries), the bottom page
   * contains the actual original instruction addresses, or 0 where
   * no mapping has been stored. Pages are created lazily. */
  ulong_t *instructions;
#endif /* TRACK_INSTRUCTIONS */

#if defined(TRACK_CFTX)
  /** Linked list storing locations of relative control flow transfers in
   * translated code */
  struct control_flow_transfer *control_flow_transfers;
  /** Location of handler trampoline executed after an interrupted control
   * flow transfers using <code>fbt_interrupt_cftx</code> */
  void *interruption_target;
  /** Information datastructure for interrupted control flow transfer
   * instruction */
  struct control_flow_transfer *interrupted_cft;

  /** Start of array storing locations of indirect jumps through ind_target
   * in trampolines */
  struct trampoline_cft* trampoline_cfts_begin;
  /** End of array beginning at <code>trampoline_cfts_begin</code> */
  struct trampoline_cft* trampoline_cfts_end;

  /** Trampoline used to execute a callback function before returning from
    * cft interruption */
  void *interrupt_cft_trampoline;
  void *interrupt_cft_from_ijump_trampoline;
  void *interrupt_cft_from_user_code_trampoline;

  /** Lock to make sure we can only interrupt cft one at the time */
  fbt_mutex_t interrupt_cft_mutex;

  /** Lock to make target thread of transaction restarting does not
    * start deleting code cache before calling thread has finished
    * overwriting control flow transfers */
  fbt_mutex_t interrupted_cft_mutex;


  /** Memory used by cft trampolines */
  struct mem_pool *cft_trampoline_mem_pool;  

#endif /* TRACK_CFTX */

#if defined(ONLINE_PATCHING)
  /** Local patching information */
  struct patching_information *patching_information;  
#endif

#ifdef SHARED_DATA
  /** Data that is shared between all threads */
  struct shared_data *shared_data;

  /** Thread identifier as returned by gettid syscall */
  ulong_t tid;
#endif /* SHARED_DATA */

#ifdef LMEM
  //unsigned int saved_eflags;
  unsigned int saved_arith_flags;
  unsigned int saved_eax;
#endif
};

/**
 * Different types of origins for trampolines.
 * Depending on the origin type we use different backpatch locations and
 * backpatch techniques. For more information about the different origins and
 * their translations see translate_execute in fbt_trampoline.c
 */
enum origin_type {
  /** origin is somewhere on the applications stack, we must clear
      it when the data is translated */
  ORIGIN_CLEAR,
  /** use relative address */
  ORIGIN_RELATIVE,
  /** use an absolute address */
  ORIGIN_ABSOLUTE,
#if defined(SHADOWSTACK)
  /** replace all entries in the shadow stack with the newly
      translated version. do an OSR - on stack replacement. */
  ORIGIN_SHADOWSTACK  
#endif
};
  
/**
 * A trampoline as a placeholder of an untranslated code block.
 * These trampolines contain two locations (origin and target) and some code to
 * branch from the translated code into the binary translator.
 * The code cache contains a JMP to the trampoline, this JMP will be fixed after
 * the translation of the code block and will then directly point to the
 * translated code in the code cache.
 *
 * [ mov %esp, (tld->stack) ] <- pointer to trampoline
 * [ mov stack+4/8, %esp    ]
 * [ call trampo            ]
 * [ origin                 ]
 * [ target                 ]
 * The call to trampo then leaves the RIP on the stack with whom we can
 * construct a pointer to origin and target.
 */
struct trampoline {
  /** placeholder for code */
  ulong_t code[3];
  /** either a pointer to the next free trampoline (if used in the free list) or
      the tail of the code to jump into the BT */
  struct trampoline *next;
  /** origin in the code cache (to fix/backpatch the jump location) */
  unsigned char *origin;
  /** target in the untranslated code */
  unsigned char *target;
  /** type of origin */
  enum origin_type origin_t;
};

#if defined(ICF_PREDICT)
/**
 * Structure for indirect control flow transfer predictions.
 * This structure is used in the optimized indirect control flow
 * prediction. This optimization caches the last two lookup locations in a local
 * cache. If there is a cache hit then we have a fast control flow transfer to
 * the cached lookup location. If there is a cache miss then we calculate a
 * lookup and update the cache. If the amount mispredictions is higher than
 * ICF_PREDICT_MAX_MISPREDICTIONS, then we rewrite the optimization into a fast
 * lookup.
 */
struct icf_prediction {
  union {
    void *src;  /**< Source of the prediction in the code cache */
    struct icf_prediction *next; /**< Pointer to the next free prediction in a
                                    the free list. */
  } pred;
  ulong_t *origin1;  /**< Ptr into the code cache (to 1st location). */
  ulong_t *dst1;  /**< Ptr into code cache (to 1st destination). */
  //void *origin2;  /**< Origin in the original code of the 2nd cache location. */
  //void *dst2;  /**< Destination in the code cache of the 2nd cache location. */
  ulong_t nrmispredict;  /**< Number of mispredictions */
};
#endif  /* ICF_PREDICT */

#if defined(SHADOWSTACK)
/** Information needed for the shadowstack feature */
struct shadowstack_entry {
  /** Return address for this call in binary translator space */
  ulong_t translated_return_address;
  
  /** Return address for this call in user space */
  ulong_t return_address;
  
  /* %esp at the time of the call */
  ulong_t stack_pointer;
  
  /* original call target of this call */
  ulong_t call_target;
};

/* Macros defining size of struct and offsets of members, so the DSL can use
more optimized instructions (as opposed to when using {offsetof(...)}). */
#define SHADOWSTACK_ENTRY_OFFSETOF_TRANSLATED_RETURN_ADDRESS 0
#define SHADOWSTACK_ENTRY_OFFSETOF_RETURN_ADDRESS 4
#define SHADOWSTACK_ENTRY_OFFSETOF_STACK_POINTER 8
#define SHADOWSTACK_ENTRY_OFFSETOF_CALL_TARGET 12
#define SHADOWSTACK_ENTRY_SIZE 16

#endif

#ifdef SHARED_DATA
struct thread_entry;

/** 
 * Structure that contains all the data that is shared among all threads. It
 * is only alloated once and then passed on when a new thread is generated. 
 */
struct shared_data {

  /** A linked list containing all threads part of the current thread pool */
  struct thread_entry *threads;
  
  /** Lock protecting the 'thread' member */
  fbt_mutex_t threads_mutex;   
  
  #if defined(ONLINE_PATCHING) 
    /** Global patching information */
    struct patching_information *patching_information;

    /** Thread-local data for the patching thread */
    struct thread_local_data *patching_tld;

    /** Protecting access to shared patching_information */
    fbt_mutex_t patching_information_lock;
  #endif

  /** The commit function when start_transaction was called */
  void (*commit_function)();  
};

/** Stores information about a list of threads. Each node should be allocated
  * in the thread local storage of the thread itself. */
struct thread_entry {
  /** Internal thread local data for this thread */
  struct thread_local_data *tld;
  /** Pointer to next thread in linked list or NULL if end of list */
  struct thread_entry *next;

  /** Can be used as temporary storage for thread-specific data */
  long user;  
};
#endif /* SHARED_DATA */

#ifdef TRACK_BASIC_BLOCKS
/** Information about a basic block translated by libdetox */
struct basic_block_node {
  /** Start of translated block */
  void *start;
  /** Length of translated block in bytes */
  long length;

  /** Start of basic block in original user code */
  void *original_start;
  /** Length of basic block in original user code in bytes */
  long original_length;

  /** Next basic block or NULL if end of list */
  struct basic_block_node *next;
  
  #ifdef BASIC_BLOCK_TRAMPOLINE
  void *stop_thread_trampoline;
  #endif  
};

struct stop_thread_context {
  void *next_instruction;
};
#endif /* TRACK_BASIC_BLOCKS */

#if defined(TRACK_CFTX)

/** Information regarding a control flow transfer used in a trampoline */
struct trampoline_cft {
  /** Address of the address used in the indirect jump */
  void *addr;
};

/** Information regarding a relative jump inside translated user code */
struct control_flow_transfer {
  /** Location of relative address in translated code */
  void *location;
  /** Number of bytes used for the address (TODO: should always be 4) */
  ulong_t size;
  /** Type of control flow transfer (TODO: could be removed) */
  ulong_t type;
  /** Address of original address that caused translation of this jump */
  void *original;
  /** Trampoline generated for this control flow transfer */
  void *trampoline;
  /** Next entry in linked list or NULL if end of list */
  struct control_flow_transfer *next;
};

enum CFTX_TYPE {
  CFTX_TYPE_JUMP,
  CFTX_TYPE_JUMP_IND,
  CFTX_TYPE_CALL,
  CFTX_TYPE_CALL_IND,
  CFTX_TYPE_JCC 
};
#endif /* TRACK_CFTX */

#endif  /* FBT_DATATYPES_H */
