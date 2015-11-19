#include "stm_settings.h"
#include <stdio.h>
#include <string.h>
#include "libstm.h"
#include "stm_actions.h"
#include "stm_translate.h"
#include "stm_util.h"
#include "stm_tcache.h"

extern void (*stm_commit_address)();

/**
 * Copy the current instruction verbatim from the TU to the CCF
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_copy(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	/* copy instruction verbatim to translated version */
	memcpy((void*) transl_addr, (void*) addr, length);
	
	*(transl_length) = length;
	
	return neutral;	
}


/**
 * Handle call instruction to relative memory address.
 * Copies the call instruction, but changes the target address such that the translated 
 * version of the function is called.
 * If the function is not already translated, the function translate_function is invoked.
 * Calls to stm_commit_transaction are replaced by calls to end_transaction, and the translation 
 * routine will not be invoked after such a call.
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_call(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	/* our opcode should be 0xE8, a near relative call */
	
	/*
	 * read call argument (32bit immediate) and add EIP (EIP = addr + length) to argument 
	 * --> absolute target address 
	 */
	int32_t call_target = *((int32_t*) (addr + 1)) + addr + length;
	
	/* write: push original EIP (we have to do this either way) */
	*((unsigned char*) transl_addr) = 0x68;	/* opcode 0x68: push imm32 */
	transl_addr++;
	*((int32_t*) transl_addr) = addr + length;	/* write argument */
	transl_addr += 4;
	*transl_length = 10;
	
#ifdef DEBUG
	printf("stm_commit_address: %#x, call_target: %#x, raw argument: %#x\n", stm_commit_address, call_target, *((int32_t*) (addr + 1)));
#endif /* DEBUG */
	
	/* compare to address of stm_commit_transaction */
	if ((int32_t) (stm_commit_address) == call_target) {
		/* aye, this is a call to stm_commit_transaction */
		/* write: jmp to end_transaction (not stm_commit_transaction, that is actually a dummy / diagnostic function) */
		*((unsigned char*) transl_addr) = 0xE9;
		transl_addr++;
		*((int32_t*) transl_addr) = (int32_t) (&end_transaction) - transl_addr - 4;
		return close;
	} else {
		/* else check if target is already translated; if not, do so now */
		void *transl_target = tcache_find((void*) call_target);
		if (transl_target == NULL) {
			/* we still have to translate the call target */
			transl_target = translate_noexecute((void*) call_target);
		}
		
		/* write: jump instruction to translated target */
		*((unsigned char*) transl_addr) = 0xE9;
		transl_addr++;
		*((int32_t*) transl_addr) = (int32_t) transl_target - transl_addr - 4;
		transl_addr += 4;
		
		/* add an entry to the tcache index (required for return from translated function) */
		tcache_add_entry((void*) addr + length, (void*) transl_addr);
		
		return neutral;
	}
}


/**
 * TODO: implement
 * Handle call instruction to absolute memory address.
 * Copies the call instruction, but changes the target address such that the translated 
 * version of the function is called.
 * If the function is not already translated, the function translate_function is invoked.
 * Calls to stm_commit_transaction are replaced by calls to end_transaction, and the translation 
 * routine will not be invoked after such a call.
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_call_indirect(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	return neutral;
}


/**
 * TODO: implement
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_jcc(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	return close;
}


/**
 * TODO: implement
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_jmp(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	/*
	 * read call argument (either 8bit or 32bit offset) and add EIP (EIP = addr + length) to argument 
	 * --> absolute target address 
	 */
	int32_t jump_target;
	if (*((unsigned char*) addr) == 0xEB) {
		/* our argument is only an 8bit offset */
		jump_target = *((unsigned char*) (addr + 1)) + addr + length;
	} else {
		/* 32bit offset */
		jump_target = *((int32_t*) (addr + 1)) + addr + length;
	}
	
	/* check if the target is already translated; if it is not, do so now */
	void *transl_target = tcache_find((void*) jump_target);
	if (transl_target == NULL) {
		/* we still have to translate the jump target */
		transl_target = translate_noexecute((void*) jump_target);
	}
	
	*((unsigned char*) transl_addr) = 0xE9;
	*((int32_t*) (transl_addr + 1)) = (int32_t) transl_target;
	
	*transl_length = 5;
	return close;
}


/**
 * TODO: implement
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction (after ret, that is alway the case)
 */
enum finalize_tu action_ret(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	/* write: call to ccf_ret */
	*((unsigned char*) transl_addr) = 0xE8;		/* call instruction */
	transl_addr++;
	*((int32_t*) transl_addr) = (int32_t) &ccf_ret - transl_addr - 4;
	*transl_length = 5;
	
	/* in case we ever need to add more here, transl_addr is now 4 too small */
	
	return close;
}


/**
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_warn(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	printf("WARNING: unhandled opcode encountered in TU at %#x\n", addr);
	printf("Will try if it works to simply copy the instruction into the CCF, but something bad could happen now...\n");
	
	return action_copy(addr, transl_addr, length, transl_length);
}


/**
 * @param addr the address of the instruction in the TU
 * @param transl_addr the address of the instruction in the CCF
 * @param length the length of the instruction in the TU
 * @param transl_length the length of the instruction(s) in the CCF
 * @return enum that determines if the TU shall be finalized after this instruction
 */
enum finalize_tu action_none(int32_t addr, int32_t transl_addr, int length, int *transl_length)
{
	/* do nothing */
	*transl_length = 0;
	return neutral;
}

