#include "stm_settings.h"
#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#include "stm_util.h"
#include "stm_translate.h"
#include "stm_actions.h"

#include "stm_tcache.h"

#define MAX_TU_INSN 12

/**
 * Push caller-save registers and EFLAGS register onto the stack.
 * We need to do this if a function where the call to that function is inserted, such that the caller 
 * does not save these registers.
 */
#define push_reg()		\
	do {			\
	__asm__("pushfl");	\
	__asm__("pushl %eax");	\
 	__asm__("pushl %ecx");	\
	__asm__("pushl %edx");	\
	} while (0)


/**
 * Pop caller-save registers and EFLAGS register from stack.
 * The converse thing to push_reg().
 */
#define pop_reg()		\
	do {			\
	__asm__("popl %edx");	\
	__asm__("popl %ecx");	\
	__asm__("popl %eax");	\
	__asm__("popfl");	\
	} while (0)


extern struct ia32_opcode *table_onebyte;

/**
 * TODO: prepare indirect call
 */
void prepare_indirect(void *addr)
{
	push_reg();
	/* *addr = translate_noexecute(addr); */
	pop_reg();
}

/**
 * translates a translation unit and jumps to the translated code.
 * If the translation unit (TU) has already been translated and is in the translation cache, jumps to the 
 * corresponding compiled code fragment (CCF). Otherwise, the TU is translated first and stored in the 
 * translation cache.
 * The stdcall attribute makes the function pop the arguments off the stack on return.
 * @param tu_address the address where the TU begins
 */
#ifdef DISABLE_ME
void __attribute__((stdcall)) translate_execute(void *tu_address)
{
	/* save caller-save registers on stack, as caller is not aware that he is calling :) */
	push_reg();
	
	/* translate the TU if not already in tcache */
	void *ccf_address = translate_noexecute(tu_address);
	
	/* overwrite return instruction pointer to address of CCF */
	change_rip(ccf_address);

	/* restore caller-save registers from stack and return */
	pop_reg();
	return;
}
#endif /* DISABLE_ME */


/**
 * translates a translation unit without jumping to the translated code.
 * If the translation unit (TU) has already been translated and is in the translation cache, the 
 * function returns immediately. Otherwise, the TU is translated first and stored in the 
 * translation cache.
 * @param tu_address the address where the TU begins
 * @return pointer to the CCF that corresponds to this TU
 */
void *translate_noexecute(void *tu_address)
{
	/* 
	 *check if this address has already been translated. If this is the case 
	 * jump to the translated code. Else, translate the code at this address first.
	 */
	
	/* look up address in translation cache index */
	void *ccf_address = tcache_find(tu_address);
	if (0 != ccf_address) {
#ifdef DEBUG
		printf("TU already translated: %#x -> %#x\n", tu_address, ccf_address);
#endif /* DEBUG */
		/* TU is already translated, jump to CCF */
		return ccf_address;
		
	}
		
	/* we need to translate TU, add to tcache index, jump to CCF */
	enum finalize_tu tu_state = neutral;
	ccf_address = tcache_put(tu_address);
	
	int insn_translated = 0;
	int32_t tu_insn_addr = (int32_t) tu_address;
	int32_t ccf_insn_addr = (int32_t) ccf_address;
	int tu_length = 0;
	int ccf_length = 0;
	enum finalize_tu (*opcode_handler)(int32_t, int32_t, int, int*) = 0;		/* that is a function pointer... */
	int insn_length = 0;
	int transl_insn_length = 0;
	while (((insn_translated < MAX_TU_INSN) && (neutral == tu_state)) || (open == tu_state)) {
#ifdef DEBUG		
		printf("tu_insn_addr: %#x\n", tu_insn_addr); /* debug */
		printf("ccf_insn_addr: %#x\n", ccf_insn_addr); /* debug */
#endif /* DEBUG */
		/* translate an instruction */
		insn_length = disasm_insn((unsigned char*) tu_insn_addr, &opcode_handler, &table_onebyte);
		tu_state = opcode_handler(tu_insn_addr, ccf_insn_addr, insn_length, &transl_insn_length);
		tu_length += insn_length;
		ccf_length += transl_insn_length;	
			
		
		tu_insn_addr += insn_length;
		ccf_insn_addr += transl_insn_length;
		insn_translated++;
#ifdef DEBUG
		printf("tu_state: %d\n", tu_state); /* debug */
		printf("tu_length: %d\n", tu_length); /* debug */
		printf("ccf_length: %d\n\n", ccf_length); /* debug */
#endif /* DEBUG */
	}
	
	/* if the TU was finished because the number of instructions hit the limit, we have to add glue code to the TU */
	if (neutral == tu_state) {
#ifdef DEBUG
		printf("finishing TU, invoking translation function on %#x\n", tu_insn_addr);
#endif /* DEBUG */
		/* write: push address of next instruction to translate onto stack */
		*((unsigned char*) ccf_insn_addr) = 0x68;
		ccf_insn_addr++;
		*((int32_t*) ccf_insn_addr) = tu_insn_addr;
		ccf_insn_addr += 4;
		
		/* write: call translate_execute */
		*((unsigned char*) ccf_insn_addr) = 0xE8;
		ccf_insn_addr++;
		*((int32_t*) ccf_insn_addr) = (int32_t) &translate_execute - ccf_insn_addr - 4;
		
		ccf_length += 10;
		
		/* ccf_insn_addr is now too small by 4 */
	}
	
	/* reclaim tcache memory that was reserved but not used */
	tcache_reclaim(ccf_address, ccf_length);
	
	return ccf_address;
}


