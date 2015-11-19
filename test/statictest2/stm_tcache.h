#ifndef STM_TCACHE_H
#define STM_TCACHE_H

/** maximum size of a compiled code fragment in bytes */
#define MAX_CCF_SIZE 150

/** translation cache index entry */
struct tcache_entry {
	/** source address */
	void *src;
	/** destination address */
	void *dst;
};


void tcache_init();

void *tcache_find(void *tu_address);

void *tcache_put(void *tu_address);

void tcache_add_entry(void *tu_address, void *ccf_address);

void tcache_reclaim(void *ccf_address, int ccf_length);

#endif /* STM_TCACHE_H */
