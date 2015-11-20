#include "gtest/gtest.h"

extern "C"{
#include "libfastbt.h"
#include "fbt_debug.h"
#include "fbt_translate.h"
#include "fbt_lmem_instruction.h"
}

/*static void disasn(const struct translate* start, int ninstr)
{
	result[0] = *start;
	for (int i=0; i<ninstr; i++) {
		fbt_disasm_instr(&result[(*result_size)-1]);
		int length = result[i-1].next_instr - result[i-1].cur_instr;
		result[i] = result[i];
	}
}*/

static struct lmem_instruction getinst(unsigned char* addr){
	struct translate tr;
	tr.next_instr = addr;
	fbt_disasm_instr(&tr);
	struct lmem_instruction instr;
	compute_lmem_instruction(&tr, &instr);
	return instr;
}

TEST(LMem, VerySimpleTranslationTest){
  unsigned char buf[1024];
  unsigned char* trans = &buf[0];
  BEGIN_ASM(trans)
    mov %edx,0x80496c0(,%eax,4)
  END_ASM
  ASSERT_EQ(buf[0], 0x89);
  ASSERT_EQ(buf[1], 0x14);
}

TEST(LMem, MovRegMem){
	DEBUG_START();
	
  unsigned char buf[1024];
  unsigned char* trans = &buf[0];
  BEGIN_ASM(trans)
    movl %edx,0x80496c0(,%eax,4)
  END_ASM
  
  struct lmem_instruction instr = getinst(&buf[0]);
  ASSERT_TRUE(instr.supported);
  ASSERT_EQ(NONE_BITS, instr.gp_registers_written);
  ASSERT_EQ(EDX_BITS | EAX_BITS, instr.gp_registers_read);
  
  DEBUG_END();  
}

TEST(LMem, AddImmEax){
	DEBUG_START();
	
  unsigned char buf[1024];
  unsigned char* trans = &buf[0];
  BEGIN_ASM(trans)
    addl $34, %eax
  END_ASM
  
  struct lmem_instruction instr = getinst(&buf[0]);
  ASSERT_TRUE(instr.supported);
  ASSERT_EQ(EAX_BITS, instr.gp_registers_written);
  ASSERT_EQ(EAX_BITS, instr.gp_registers_read);
  
  DEBUG_END();  
}

TEST(LMem, PushEbx){
	DEBUG_START();
	
  unsigned char buf[1024];
  unsigned char* trans = &buf[0];
  BEGIN_ASM(trans)
    pushl %ebx
  END_ASM
  
  struct lmem_instruction instr = getinst(&buf[0]);
  ASSERT_TRUE(instr.supported);
  ASSERT_EQ(ESP_BITS, instr.gp_registers_written);
  ASSERT_EQ(EBX_BITS | ESP_BITS, instr.gp_registers_read);
  
  DEBUG_END();  
}

