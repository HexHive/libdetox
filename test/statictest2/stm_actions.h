#ifndef STM_ACTIONS_H
#define STM_ACTIONS_H

#include <stdint.h>

/* TODO: decide placement of this enum definition; does this really belong here */
enum finalize_tu {
	open,		/* TU must not be closed after this instruction */
 	neutral,	/* TU can be closed after this instruction */
  	close		/* TU has to be closed after this instruction */
};

enum finalize_tu action_copy(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_call(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_call_indirect(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_jcc(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_jmp(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_ret(int32_t addr, int32_t transl_addr, int length, int *transl_length);


enum finalize_tu action_warn(int32_t addr, int32_t transl_addr, int length, int *transl_length);

enum finalize_tu action_none(int32_t addr, int32_t transl_addr, int length, int *transl_length);

extern void ccf_ret(void *original_rip);	/* implemented in stm_asm_functions.S */

#endif /* STM_ACTIONS_H */
