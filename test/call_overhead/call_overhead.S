# measure overhead of function call

.LC0:	.string "number of cycles without function call: %5d\n"
.LC1:	.string "number of cycles with    function call: %5d\n"

.globl main

main:	
	pushl %ebp
	movl %esp, %ebp
	subl $16, %esp		# enlarge stack for temporaries and call arguments
	
	# test without function call
	############################
	
	movl $3, -12(%ebp)
	movl $4, -16(%ebp)
	
	
	rdtsc			# measure cycle count and save it at -4(%ebp)
	movl %eax, -4(%ebp)
	
	
	movl $0, -8(%ebp)	# setup while loop (no func call)
	jmp wtst1
	
wstart1:			# while loop (no func call) start
	movl -12(%ebp), %edx
	movl -16(%ebp), %ecx
	addl %edx, %ecx
	incl -8(%ebp)
	
wtst1:	cmpl $1000, -8(%ebp)	# while loop (no func call) compare
	jl wstart1
	
	rdtsc			# measure cycle count a second time (result in %eax)
	
	subl -4(%ebp), %eax	# calculate number of cycles 
	movl %eax, 4(%esp)
	movl $.LC0, (%esp)
	call printf
	
	# test with function call
	#########################

	rdtsc
	movl %eax, -4(%ebp)
	
	
	movl $0, -8(%ebp)	# setup while loop (with func call)
	jmp wtst2
	
wstart2:			# while loop (with func call) start
	movl $3, -12(%ebp)
	movl $4, -16(%ebp)
	call add2
	incl -8(%ebp)
	
wtst2:	cmpl $1000, -8(%ebp)	# while loop (with func call) compare
	jl wstart2
	
	rdtsc
	
	subl -4(%ebp), %eax
	movl %eax, 4(%esp)
	movl $.LC1, (%esp)
	call printf
	
	# end of tests
	##############
	
	movl $0, %eax
	leave
	ret
	
	
	
add2:	pushl %ebp
	movl %esp, %ebp
	movl 12(%ebp), %ecx
	movl 8(%ebp), %eax
	addl %ecx, %eax
	leave
	ret
	
