
# This program demonstrates how it is possible to break out of the translated code 
# in secuBT or fastBT if the real instruction pointer (of the translated code 
# that is being executed) is accessible to the guest code.
# This is the case if we deny execution of an int 0x80 system call in secuBT, or possibly if
# we execute a system call with the sysenter mechanism (call *%gs:0x10). 
# In these cases, the (translated) return instruction pointer has to be on the stack
# and is then accessible to the guest code in -4(%esp) if we don't happen to be at a 
# page boundary on the stack.
# With some experimentation, it is possible to "backpatch" a jump target in the 
# translation cache from the guest code itself.



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
	int $0x80

	movl $func, %ecx		# 5 bytes, untranslated address of this instr is at -4(%esp)
	subl -4(%esp), %ecx		# 4 bytes, offset to untranslated func function into %ecx...
	subl $31, %ecx			# 3 bytes
	movl -4(%esp), %edx		# 4 bytes, address of jmp offset bytes into %edx...
	addl $27, %edx			# 3 bytes
	movl %ecx, (%edx)		# 2 bytes
	pushl $rip				# 5 bytes
	jmp func	# 1 byte opcode, 4 bytes offset when translated; backpatch this offset
rip:
	# print string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring3, %ecx
	movl $9, %edx
	int $0x80
	
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
	int $0x80
	
	movl $0, %eax
	leave
	ret
	
