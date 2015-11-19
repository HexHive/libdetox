	.file	"ret_predict_test.c"
	.text
.globl bar
	.type	bar, @function
bar:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	ret
	.size	bar, .-bar
.globl foo
	.type	foo, @function
foo:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	bar
	leave
	ret
	.size	foo, .-foo

.globl bar2
	.type	bar2, @function
bar2:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	pushl	(%esp)		# duplicate top-of-stack
	ret	$4		# remove duplicate with "ret imm16"
	.size	bar2, .-bar2
.globl foo2
	.type	foo2, @function
foo2:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	bar2
	leave
	ret
	.size	foo2, .-foo2

	.section	.rodata
.LC0:
	.string	"1: %d\n"
.LC1:
	.string	"2: %d\n"
.LC2:
	.string	"3: %d\n"
.LC3:
	.string	"4: %d\n"
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
	subl	$52, %esp
	movl	$0, (%esp)
	call	fbt_init
	movl	$fbt_commit_transaction, (%esp)
	call	fbt_start_transaction

	movl	$1, (%esp)
	call	foo			# init prediction
	movl	%eax, -8(%ebp)
	movl	$2, (%esp)
	call	foo			# correct predicted
	movl	%eax, -12(%ebp)
	movl	$3, (%esp)
	call	bar			# wrong predicted
	movl	%eax, -16(%ebp)
	movl	$4, (%esp)
	call	bar			# correct again
	movl	%eax, -20(%ebp)

	# no with ret imm16

	movl	$1, (%esp)
	call	foo2			# init prediction
	movl	%eax, -24(%ebp)
	movl	$2, (%esp)
	call	foo2			# correct predicted
	movl	%eax, -28(%ebp)
	movl	$3, (%esp)
	call	bar2			# wrong predicted
	movl	%eax, -32(%ebp)
	movl	$4, (%esp)
	call	bar2			# correct again
	movl	%eax, -36(%ebp)

	call	fbt_commit_transaction
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	movl	-16(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC2, (%esp)
	call	printf
	movl	-20(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	-24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	movl	-28(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	movl	-32(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC2, (%esp)
	call	printf
	movl	-36(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	$0, %eax
	addl	$52, %esp
	popl	%ecx
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.1.3 20070929 (prerelease) (Ubuntu 4.1.2-16ubuntu2)"
	.section	.note.GNU-stack,"",@progbits
