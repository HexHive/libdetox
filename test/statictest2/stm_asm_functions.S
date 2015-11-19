
SUCCSTR:
	.string	"Successfully executed translated code until stm_commit_transaction().\n"

	.global translate_execute
	.global end_transaction
	.global change_rip
	.global read_ebp
	.global ccf_ret
translate_execute:
	pushl	%ebp
	movl	%esp, %ebp
	
	pushfl		# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx
		
	pushl	8(%ebp)	# translate_noexecute(tu_address)
	call	translate_noexecute
	
	pushl	%eax	# change_rip(ccf_address)
	call	change_rip

	addl  $8, %esp  # readjust stack (for both functions)
	
	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl
	
	leave
	ret	$4


end_transaction:
	pushl	%ebp
	movl	%esp, %ebp
	
	pushfl		# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	
	pushl	$SUCCSTR	# print success message
	call	printf
	addl  $4, %esp  # clear stack (printf argument)
	
	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl
	
	leave
	ret

change_rip:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl	(%ebp), %ecx
	movl	8(%ebp), %edx
	movl	%edx, 4(%ecx)
	
	leave
	ret
	
read_ebp:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl	(%ebp), %eax
	leave
	ret


ccf_ret:			# a ret instruction in the TU is replaced by a call to this function in the CCF
	pushl	%ebp
	movl	%esp, %ebp
	
	pushfl			# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	
	pushl	8(%ebp)
	call	tcache_find	# look up return address of function in TU in the translation cache index
	movl	%eax, 4(%ebp)	# overwrite our return instruction pointer with the result
	addl	$4, %esp	# release space on stack so the pops that follow work correctly
	
	
	popl	%edx		# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl
	
	leave
	ret	$4		# remove return instruction pointer of the translated function from the stack
