/*
 * In the intel manual, the instruction PUSH imm8 and PUSH imm16/imm32 (opcodes 0x6A and 0x68, respectively) are missing the 
 * immediate operands in the opcode column. This is used to test if this is a mistake in the intel manual or if there 
 * is indeed no immediate operand in the machine code for these instructions. 
 */


int main(int argc, char **argv)
{
	/* push imm32 onto stack */
	__asm__("pushl $0x55555555");
	
	/* another test, MOV with moffs32: is moffs32 in the machine code, and if so, how? */
	__asm__("movl 0x55555555, %eax");
	__asm__("movl 0x5555, %eax");
	/* aye, moffs32 is in the code as a 32 bit number, arrrr */
	
	/* test ROL */
	__asm__("roll $3, %ecx");
	
	/* test LES */
	__asm__("les 0x44444444, %ecx");
	
	/* test unary group 3 (immediate arguments only for some instructions on same opcode?) */
	__asm__("testl $1023, 0x66666666");
	__asm__("negl 0x66666666");
	/* argh, fuck you intel! testl and negl have same opcode, but different arguments */
	
	/* test prefetch */
	__asm__("prefetch 0x66");
	
	/* test FXSAVE */
	__asm__("fxsave 0x11111111");
	
	/* test SGDT */
	__asm__("sgdt 0x22222222");
	
	/* test push r/m32 */
	__asm__("pushl 8(%ebp)");
	
	return 0;
}


