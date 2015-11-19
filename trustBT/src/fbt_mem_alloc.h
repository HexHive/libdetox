#ifndef FBT_MEM_ALLOC_H
#define FBT_MEM_ALLOC_H

#include <sys/mman.h>
#include "fbt_private_datatypes.h"

/* allocate a small amount of memory; will be freed when thread finishes */
void *fbt_smalloc(struct thread_local_data *tld, int bytes);

/* allocate a small amount of memory; will only be freed when the application exits */
void *fbt_smalloc_pers(struct thread_local_data *tld, int bytes);

/* allocate a number of pages that will be freed when thread finishes */
void *fbt_lalloc(struct thread_local_data *tld, int pages);

/* when the thread finishes, it calls this function to free memory */
void fbt_free_all(struct thread_local_data *tld);

/** macro to allocate n pages of memory */
#define allocpages(n) mmap(NULL, n * sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

/** macro to free n pages of memory that was allocated with allocpages() */
#define freepages(addr, n) munmap(addr, n * sysconf(_SC_PAGESIZE))

#endif /* FBT_MEM_ALLOC_H */
