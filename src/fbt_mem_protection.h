#ifndef FBT_MEM_PROTECTION_H
#define FBT_MEM_PROTECTION_H

#ifndef _ASSEMBLER_

#include "fbt_rbtree.h"
#include "fbt_private_datatypes.h"

/* entry in the list of loaded libraries */
struct lib_list_entry {
  void *base_addr;
  int length;
  char *name;
};

/* info about a memory region */
struct mem_info {
  struct rb_node node;    // red-black tree node, contains addr_begin and addr_end
  const char* obj_name;   // name of the associated object (such as a library)
  const char* sec_name;   // name of the section in an object
  char flags;             // flags: read, write, execute, internal
  uint16_t lib_index;     // index in the library list
};

void fbt_memprotect_init();
void fbt_memprotect_add_valid(void *addr_begin, int len);

/* check if an address is in executable memory */
int fbt_memprotect_execquery(void *tu_addr);
int fbt_memprotect_info(void *tu_addr, struct mem_info *info);


/*
 * The following functions are used for protection of internal data structures
 * against tampering from within the guest code.
 * In this context, locking means removing the write permission from the
 * memory, and unlocking means setting the write permission.
 */

/* unlocks the most common data structures such as tcache, hashtable, tld */
void fbt_ids_unlock(struct thread_local_data *tld);

/* unlocks the data structure that contains the argument addr */
void fbt_ids_unlockat(struct thread_local_data *tld, void *addr);

/* locks all previously unlocked data structures, LIFO */
void fbt_ids_lockdown(struct thread_local_data *tld);

/* sets the executable flag for the data structure that contains addr */
void fbt_ids_setexec(struct thread_local_data *tld, void *addr);

#ifdef SECU_MPROTECT_IDS
#  define FBT_IDS_LOCKDOWN(tld) fbt_ids_lockdown(tld)
#  define FBT_IDS_UNLOCK(tld) fbt_ids_unlock(tld)
#else
#  define FBT_IDS_LOCKDOWN(tld)
#  define FBT_IDS_UNLOCK(tld)
#endif

#endif /* _ASSEMBLER_ */

/* bit masks for the flags field of elf_section_info */
#define INFO_RFLAG 0x4
#define INFO_WFLAG 0x2
#define INFO_XFLAG 0x1
#define INFO_BTFLAG 0x8    /* memory used for BT library */

#endif /* FBT_MEM_PROTECTION_H */
