	.section .rodata
.mystring1:
	.string "hello world\n"
.mystring2:
	.string "this string is long and shall not be printed\n"

	.text
	.globl main
main:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	
	# print first string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring1, %ecx
	movl $12, %edx
	int $0x80
	
	# print second string, longer than 12 characters
	# shall be prevented by authorize_syscall
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring2, %ecx
	movl $45, %edx
	int $0x80
	
	# print first string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring1, %ecx
	movl $12, %edx
	int $0x80
	
	# print second string, longer than 12 characters
	# shall be prevented by authorize_syscall
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring2, %ecx
	movl $45, %edx
	int $0x80
	
	
	# print first string
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring1, %ecx
	movl $12, %edx
	int $0x80
	
	# print second string, longer than 12 characters
	# shall be prevented by authorize_syscall
	movl $4, %eax
	movl $1, %ebx
	movl $.mystring2, %ecx
	movl $45, %edx
	int $0x80
	
	
	popl %ebx
	movl $0, %eax
	leave
	ret
	
