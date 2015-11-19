	.data
my_addr:
	.int 0x00000000
.my_string:
	.string "success if result is 8. result: %d\n"


	.global main

	.text
main:	pushl	%ebp
	movl	%esp, %ebp

	pushl	$0
	call	fbt_init

	pushl	$fbt_commit_transaction
	call	fbt_start_transaction
	xor	%eax, %eax

	movl	$foo, %ecx
	pushl	$.cont
	jmp	*%ecx

.cont:	call	fbt_commit_transaction
	pushl	%eax
	pushl	$.my_string
	call	printf

	movl	$0, %eax
	leave
	ret

foo:	pushl	%ebp
	movl	%esp, %ebp

	movl	$8, %eax

	leave
	ret
