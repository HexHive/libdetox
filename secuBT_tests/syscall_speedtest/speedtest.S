#define ITER $1000000000
#define PERC $10000000

# calls ITER times getpid to test system call performance

	.data
.iter:
	.int 10000
.perc:
	.int 100
	
.statusmsg:
	.string "%d percent complete\n"
	
.pidmsg:
	.string "pid of this process: %d\n"

	
	.text
	.globl main
main:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	
	movl $0, %esi
	
.sc_pid:
	# getpid system call
	movl $20, %eax
	call *%gs:0x10
	movl %eax, %edi
	
.progr:
	# progress indication calculation
	inc %esi
	movl PERC, %ecx
	movl $0, %edx
	movl %esi, %eax
	idiv %ecx
	testl %edx, %edx
	jne .itercmp
	
	# print progress information
	pushl %eax
	pushl $.statusmsg
	call printf
	addl $8, %esp
	
	
.itercmp:
	# check loop condition
	movl ITER, %edx
	cmpl %esi, %edx
	jge .sc_pid
	
	# print pid
	pushl %edi
	pushl $.pidmsg
	call printf
	addl $8, %esp
	
	popl %edi
	popl %esi
	popl %ebx
	movl $0, %eax
	leave
	ret
