#ifndef STM_UTIL_H
#define STM_UTIL_H

#include <stdint.h>

extern void change_rip(void* rip_address);
void *read_rip();
extern void end_transaction();
extern int read_ebp();

#endif /* STM_UTIL_H */
