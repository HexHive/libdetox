	.file	"switch.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"switch start"
.LC1:
	.string	"case 0"
.LC2:
	.string	"case 1"
.LC3:
	.string	"case 2"
.LC4:
	.string	"case 3"
.LC5:
	.string	"case 4"
.LC6:
	.string	"case 5"
.LC7:
	.string	"case 6"
.LC8:
	.string	"case 7"
.LC9:
	.string	"default case"
.LC10:
	.string	"switch end"
	.text
	.p2align 4,,15
.globl main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ecx
	subl	$20, %esp
	movl	$1, -8(%ebp)
	movl	$.LC0, (%esp)
	call	puts
	movl	-8(%ebp), %eax
	cmpl	$7, %eax
	jbe	.L15
	movl	$.LC9, (%esp)
	call	puts
	.p2align 4,,7
	.p2align 3
.L12:
	movl	$.LC10, (%esp)
	call	puts
	addl	$20, %esp
	xorl	%eax, %eax
	popl	%ecx
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.p2align 4,,7
	.p2align 3
.L15:
	jmp	*.L11(,%eax,4)
	.section	.rodata
	.align 4
	.align 4
.L11:
	.long	.L3
	.long	.L4
	.long	.L5
	.long	.L6
	.long	.L7
	.long	.L8
	.long	.L9
	.long	.L10
	.text
	.p2align 4,,7
	.p2align 3
.L10:
	movl	$.LC8, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L3:
	movl	$.LC1, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L4:
	movl	$.LC2, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L5:
	movl	$.LC3, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L6:
	movl	$.LC4, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L7:
	movl	$.LC5, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L8:
	movl	$.LC6, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L9:
	movl	$.LC7, (%esp)
	call	puts
	.p2align 4,,3
	.p2align 3
	jmp	.L12
	.size	main, .-main
	.ident	"GCC: (Debian 4.3.2-1.1) 4.3.2"
	.section	.note.GNU-stack,"",@progbits
