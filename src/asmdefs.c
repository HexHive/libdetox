
#include <pthread.h>
#include <stdio.h>
#include <stddef.h>
#include "fbt_private_datatypes.h"
#include "fbt_mem_protection.h"

int main(int argc, char **argv)
{
    printf("/*\n * fbt_asm_offsets.h\n * This file is automatically generated "
            "by asmdefs.\n * Manual edits will be overwritten.\n");
    printf(" * If you need to change something, please edit asmdefs.c\n"
            " */\n\n\n");
    printf("#ifndef FBT_ASM_OFFSETS_H\n#define FBT_ASM_OFFSETS_H\n\n");
            
    // offsets of struct thread_local_data
    printf("#define OFFSETOF_TLD_HASHTABLE %d\n",
            offsetof(struct thread_local_data, 
                      hashtable));
    printf("#define OFFSETOF_TLD_TS %d\n",
            offsetof(struct thread_local_data, 
                      ts));
                      
    printf("#define OFFSETOF_TLD_TS_TRANSL_INSTR %d\n",
            offsetof(struct thread_local_data, 
                      ts)
            + offsetof(struct translate_struct, 
                      transl_instr));
    
    printf("#define OFFSETOF_TLD_IND_JUMP_TARGET %d\n", 
            offsetof(struct thread_local_data, 
                      ind_jump_target));
/* TODO: REMOVE */
/*     printf("#define OFFSETOF_TLD_TRANSLATE_STACK %d\n",  */
/*             offsetof(struct thread_local_data,  */
/*                       translate_stack)); */
    printf("#define OFFSETOF_TLD_RET_JUMPBACK_TRAMP %d\n",
            offsetof(struct thread_local_data,
                      ret_jumpback_tramp));

    printf("#define OFFSETOF_IND_JUMP_TRAMPOLINE %d\n", 
            offsetof(struct thread_local_data, 
                      ind_jump_trampoline));
    printf("#define OFFSETOF_IND_CALL_TRAMPOLINE %d\n", 
            offsetof(struct thread_local_data, 
                      ind_call_trampoline));

#if defined(FBT_RET_STACK)
    printf("#define OFFSETOF_TLD_TRANSLATED_CALL_STACK_END %d\n",
            offsetof(struct thread_local_data, 
                      translated_call_stack_end));
    printf("#define OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS %d\n",
            offsetof(struct thread_local_data, 
                      translated_call_stack_tos));
    
    // offset of struct ret_stack_entry
    printf("#define OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR %d\n",
            offsetof(struct ret_stack_entry, 
                      transl_addr));
    printf("#define OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR %d\n",
            offsetof(struct ret_stack_entry, 
                      orig_addr));
#endif

    // offsets of struct tcache_entry
    printf("#define OFFSETOF_TCACHE_ENTRY_SRC %d\n",
            offsetof(struct tcache_entry, 
                      src));
    printf("#define OFFSETOF_TCACHE_ENTRY_DST %d\n",
            offsetof(struct tcache_entry, 
                      dst));
    printf("#define SIZEOF_HASHTABLE_ENTRY %d\n",
            sizeof(struct tcache_entry));

    // offsets of struct mem_info
    printf("#define OFFSETOF_MEM_INFO_FLAGS %d\n", 
            offsetof(struct mem_info, flags));
    printf("#define SIZEOF_MEM_INFO %d\n",
            sizeof(struct mem_info));

    printf("\n\n#endif /* FBT_ASM_OFFSETS_H */\n");
    return 0;
}