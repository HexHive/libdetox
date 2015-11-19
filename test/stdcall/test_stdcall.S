	.file	"test_stdcall.c"
	.text
.globl main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ecx
	subl	$36, %esp
	movl	$5, 4(%esp)
	movl	$3, (%esp)
	call	add1
	subl	$8, %esp
	movl	%eax, -8(%ebp)
	movl	-4(%ebp), %ecx
	leave
	leal	-4(%ecx), %esp
	ret
	.size	main, .-main
.globl add1
	.type	add1, @function
add1:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	add2
	leave
	ret	$8
	.size	add1, .-add1
.globl add2
	.type	add2, @function
add2:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	addl	8(%ebp), %eax
	popl	%ebp
	ret
	.size	add2, .-add2
	.ident	"GCC: (GNU) 4.1.2 (Gentoo 4.1.2)"
	.section	.note.GNU-stack,"",@progbits
