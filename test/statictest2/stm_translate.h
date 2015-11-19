#ifndef STM_TRANSLATE_H
#define STM_TRANSLATE_H

#include "stm_tcache.h"

void *translate_noexecute(void *tu_address);

extern void __attribute__((stdcall)) translate_execute(void *tu_address);

#endif /* STM_TRANSLATE_H */
