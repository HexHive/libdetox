#
# This file defines all the in assembly implemented functions that are
# used mostly in the translate actions and when the bt is started or stopped
#
# Copyright (c) 2008 ETH Zurich
#   Mathias Payer <mathias.payer@inf.ethz.ch>
#   Marcel Wirth <mawirth@student.ethz.ch>
#   Stephan Classen <scl@soft-eng.ch>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#

#define _ASSEMBLER_
#include "fbt_private_datatypes.h"

#ifdef FBT_FIND_FAST
#  define TCACHE_FIND tcache_find_fast
#  define SAVE_EDX_FIND
#  define RESTORE_EDX_FIND
#  define SAVE_EDX_FIND_FAST pushl %edx 
#  define RESTORE_EDX_FIND_FAST popl %edx
#else
#  define TCACHE_FIND  tcache_find
#  define SAVE_EDX_FIND pushl %edx 
#  define RESTORE_EDX_FIND popl %edx
#  define SAVE_EDX_FIND_FAST
#  define RESTORE_EDX_FIND_FAST
#endif

.section .text	
SUCCSTR:
	.string	"Successfully executed translated code until fbt_commit_transaction().\n"

EIP:
  .string "eip: 0x%.8x eax: 0x%.8x ebx: 0x%.8x ecx: 0x%.8x edx: 0x%.8x ebp: 0x%.8x\n"
EIP2:
  .string "to: 0x%.8x from: 0x%.8x eax: 0x%.8x\n"

PREDICTMISS:
  .string "Ret miss: 0x%.8x -> 0x%.8x\n"
  
	.global	read_rip
	.type	read_rip, @function

	.global	change_rip
	.type	change_rip, @function

	.global	set_return_to_translated_asm
	.type	set_return_to_translated_asm, @function

	.global	set_return_to_translated_asm_nostackframe
	.type	set_return_to_translated_asm_nostackframe, @function
	
	.global	ind_jump
	.type	ind_jump, @function

	.global	ind_jump_remove
	.type	ind_jump_remove, @function

	.global	ind_jump_backpatch
	.type	ind_jump_backpatch, @function

#if defined(FBT_RET_STACK)
	.global	ind_jump_backpatch_abs
	.type	ind_jump_backpatch_abs, @function
#endif

	.global ind_jump_chaining
	.type	ind_jump_chaining, @function

#if defined(FBT_IND_CALL_PREDICTION)
	.global fix_ind_call_prediction
	.type	fix_ind_call_prediction, @function
#endif

#if defined(FBT_RET_PREDICTION)
	.global ret_predict
	.type	ret_predict, @function

	.global ret_predict_remove
	.type	ret_predict_remove, @function
#endif

#if defined(FBT_RET_STACK)
	.global ret_stack_synchronize
	.type	ret_stack_synchronize, @function
#endif
	
	.global	prepare_sysenter
	.type	prepare_sysenter, @function

	.global	end_transaction
	.type	end_transaction, @function

	.global	print_eip
	.type	print_eip, @function

	.global ind_jump_signal
	.type	ind_jump_signal, @function



read_rip:
	movl	4(%ebp), %eax  # load old eip into eax and return the value
	ret	
	.size	read_rip, .-read_rip


change_rip:
	pushl	%ebp
	movl	%esp, %ebp

	movl	(%ebp), %ecx
	movl	8(%ebp), %edx
	movl	%edx, 4(%ecx)

	leave
	ret
	.size	change_rip, .-change_rip

# This function changes the return instruction pointer of the calling
# function to point to the translated code.
# Stack:
#     +-------------+
#  ?? | old eip     | (gets patched)
#  ?? | old ebp     | <- ebp
#     | xxxxxxxxxxx |
#     | xxxxxxxxxxx |
#     | xxxxxxxxxxx |
#   0 | eip (call)  | <- esp
#     +-------------+
set_return_to_translated_asm:
	# get the thread local data
	call	get_tld

	# translate if necessary
	pushl	4(%ebp)
	pushl	%eax
	call	TCACHE_FIND
	testl	%eax, %eax
	jne .translated_srtt
	call	translate_noexecute

.translated_srtt:
	# overwrite return instruction pointer of caller function
	movl	%eax, 4(%ebp)

	# readjust stack
	addl	$8, %esp

	ret
	.size	set_return_to_translated_asm, .-set_return_to_translated_asm

# This function changes the return instruction pointer of the calling
# function to point to the translated code.
# It assumes it was called just after the call, there is no stack frame yet.
# Stack:
#     +-------------+
#   4 | old eip     |
#   0 | eip (call)  | <- esp
#     +-------------+
set_return_to_translated_asm_nostackframe:
	# get the thread local data
	call	get_tld

	# translate if necessary
	pushl	4(%esp)
	pushl	%eax
	call	TCACHE_FIND
	testl	%eax, %eax
	jne .translated_srttn
	call	translate_noexecute

.translated_srttn:
	# readjust stack
	addl	$8, %esp

	# overwrite return instruction pointer of caller function
	movl	%eax, 4(%esp)

	ret
	.size	set_return_to_translated_asm_nostackframe, .-set_return_to_translated_asm_nostackframe
	
# This function fixes an indirect jump
# checks if the target is already translated and fixes
# the return instruction pointer
# Stack:
#     +-------------+
#  12 | target IP   | (gets removed by ret $8)
#   8 | tld-pointer | (gets removed by ret $8)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
#if !defined(FBT_IND_JUMP_INLINE_FIND_FAST)
#  define TARGETIP 12(%ebp)
#  define TLDPTR 8(%ebp)
#  define EIPPTR 4(%ebp)
#else
#  define TARGETIP 20(%esp)
#  define TLDPTR 16(%esp)
#  define EIPPTR 12(%esp)
#  define TARGETIPEDX 24(%esp)
#  define TLDPTREDX 24(%esp)
#endif
ind_jump:
#if !defined(FBT_IND_JUMP_INLINE_FIND_FAST)
	pushl	%ebp
	movl	%esp, %ebp
#endif

#	call	print_eip2
	
	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	SAVE_EDX_FIND

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump)
	adcl	$0x0, (fbt_nr_ind_jump+0x4)
#endif

	# translate the target IP if necessary

	# inlined tcache lookup!
#if defined(FBT_IND_JUMP_INLINE_FIND_FAST)
#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_tcache_fast_lookups)
	adcl	$0x0, (fbt_nr_tcache_fast_lookups+0x4)
#endif
	movl	TARGETIP, %eax		# load tu_address
	leal	(,%eax, 4),%eax		# offset = (tu_address << 1) & HASH_PATTERN
	andl	$HASH_PATTERN, %eax
	movl	TLDPTR, %ecx		# entry = tld->hashtable + offset
	addl	OFFSETOF_TLD_HASHTABLE(%ecx), %eax
	
.load_compare_ij:
	movl	OFFSETOF_TCACHE_ENTRY_SRC(%eax), %ecx

	cmpl	TARGETIP, %ecx					# if (entry->src != tu_address) goto fallback_tff
	jne .fallback_ij
	movl	OFFSETOF_TCACHE_ENTRY_DST(%eax), %eax		# return entry->dst
#	jmp	.translated_ij
#else
	pushl	TARGETIP
	pushl	TLDPTR
	call	TCACHE_FIND
	addl	$8, %esp 	# re-adjust the stack
	testl	%eax, %eax
	jne,pt .translated_ij	# ,pt = branch prediction hint: take
	SAVE_EDX_FIND_FAST
	pushl	TARGETIP
	pushl	TLDPTR
	call	translate_noexecute
	addl	$8, %esp
	RESTORE_EDX_FIND_FAST
#endif
.translated_ij:
	# overwrite return instruction pointer
	movl	%eax, EIPPTR

	# pop caller-save and eflags registers
	RESTORE_EDX_FIND
	popl	%ecx
	popl	%eax
	popfl

#if !defined(FBT_IND_JUMP_INLINE_FIND_FAST)
	leave
#endif
	ret	$8
	
#if defined(FBT_IND_JUMP_INLINE_FIND_FAST)
# these are fallback and null catches and unlikely to be called
# -> moved them to the end of the function for better pipelining
.fallback_ij:
	jecxz .null_ij						# if (entry->src == 0) goto null_tff
#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_tcache_fast_miss)
	adcl	$0x0, (fbt_nr_tcache_fast_miss+0x4)
#endif
	addl	$SIZEOF_HASHTABLE_ENTRY, %eax	# iterate eax to the next row
	cmpl	$0x1, %ecx			# check for guard
	jne,pt	.load_compare_ij
	subl	$SIZEOF_HASHTABLE_ENTRY, %eax
	subl	$HASHTABLE_SIZE, %eax
	jmp	.load_compare_ij

.null_ij:	
#	SAVE_EDX_FIND_FAST
	pushl	%edx
	pushl	TARGETIPEDX
	pushl	TLDPTREDX
	call	translate_noexecute
	addl	$8, %esp
#	RESTORE_EDX_FIND_FAST
	popl	%edx
	jmp	.translated_ij
#endif
	.size	ind_jump, .-ind_jump



# This function fixes an indirect jump
# checks if the target is already translated and fixes
# the return instruction pointer
# Stack:
#     +-------------+
#  ?? | xxxxxxxxxxx | (will be removed by this function)
#  16 | target IP   | (will be removed by this function)
#  12 | num_bytes   | (will be removed by this function)
#   8 | tld-pointer | (will be removed by this function)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ind_jump_remove:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	SAVE_EDX_FIND

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump_remove)
	adcl	$0x0, (fbt_nr_ind_jump_remove+0x4)
#endif

	# translate the target IP if necessary
	pushl	16(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_ijr	# ,pt = branch prediction hint: take
	
	SAVE_EDX_FIND_FAST
	pushl	16(%ebp)
	pushl	8(%ebp)
	call	translate_noexecute
	addl	$8, %esp
	RESTORE_EDX_FIND_FAST

.translated_ijr:
	# overwrite return instruction pointer
	# taking into account the number of bytes which should be removed from the stack
	movl	%ebp, %ecx
	addl	12(%ebp), %ecx
	movl	%eax, 16(%ecx)

	# readjust stack
	addl	$8, %esp

	# pop caller-save and eflags registers
	RESTORE_EDX_FIND
	popl	%ecx
	popl	%eax
	popfl

	leave

	 # remove the requested number of bytes from the esp
	addl	8(%esp), %esp
	addl	$12, %esp
	ret
	.size	ind_jump_remove, .-ind_jump_remove



# Same as ind_jump, but has three parameters
# the third parameter gets patched and the trampoline
# (where we come from) is freed
# Stack:
#     +-------------+
#  16 | origin eip  | (this addr. gets patched) (gets removed by ret $12)
#  12 | target IP   | (gets removed by ret $12)
#   8 | tld-pointer | (gets removed by ret $12)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ind_jump_backpatch:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump_backpatch)
	adcl	$0x0, (fbt_nr_ind_jump_backpatch+0x4)
#endif

	# free the trampoline slot where we came from
	movl	4(%ebp), %eax
	subl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	trampoline_free

	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt	.translated_ijb	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_ijb:
	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)

	# readjust stack
	addl 	$16, %esp

	# rewrite jump target of the jump to the trampoline: translated_address - origin - 4
	# only the jump target (the last 4 bytes of the last instruction) need to be modified
	movl	16(%ebp), %ecx
	subl	$4, %eax
	subl	%ecx, %eax
	movl	%eax, (%ecx)

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$12
	.size	ind_jump_backpatch, .-ind_jump_backpatch

#if defined(FBT_RET_STACK)

# Same as ind_jump_backpatch, but the patching is absolute, not relative
# also walks the shadow stack and patches all entries that point to this trampoline
# Stack:
#     +-------------+
#  16 | origin eip  | (this addr. gets patched) (gets removed by ret $12)
#  12 | target IP   | (gets removed by ret $12)
#   8 | tld-pointer | (gets removed by ret $12)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ind_jump_backpatch_abs:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump_backpatch_abs)
	adcl	$0x0, (fbt_nr_ind_jump_backpatch_abs+0x4)
#endif

	# free the trampoline slot where we came from
	movl	4(%ebp), %eax
	subl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	trampoline_free

	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt	.translated_ijba	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_ijba:
	# rewrite jump target of the jump to the trampoline: translated_address
	# only the jump target (the last 4 bytes of the last instruction) need to be modified
	movl	16(%ebp), %ecx
	movl	%eax, (%ecx)

	# keep trampoline addr for walking the shadow stack
	movl	4(%ebp), %ecx
	subl	$20, %ecx

	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)

	# walk shadow stack and patch all entries that point to this trampoline
	pushl 	%ebx
	movl	%eax, %ebx			# ebx: translated address 
	movl	8(%ebp), %edx		# (tld)
	movl	OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS(%edx), %eax # eax: tld->translated_call_stack_tos 
	movl	OFFSETOF_TLD_TRANSLATED_CALL_STACK_END(%edx), %edx # edx: tld->translated_call_stack_end
	jmp .cond_ijbass
.next_ijbass:
	cmpl	%ecx, (%eax)		# find uses of this trampoline
	jne .skip_ijbass
	movl	%ebx, (%eax)		# replace trampoline with translated address
.skip_ijbass:
	addl	$8, %eax
.cond_ijbass:
	cmpl	%eax, %edx
	jne		.next_ijbass
	popl	%ebx

	# readjust stack
	addl 	$16, %esp

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$12
	.size	ind_jump_backpatch_abs, .-ind_jump_backpatch_abs

#endif

# Same as ind_jump_backpatch, but the jump to the trampoline is either 
# patched (next_instr already translated) or overwritten (otherwise).
# This function is used for the chaining optimization in fbt_translate.c
# Stack:
#     +-------------+
#  16 | origin eip  | (this addr. gets patched) (gets removed by ret $12)
#  12 | target IP   | (gets removed by ret $12)
#   8 | tld-pointer | (gets removed by ret $12)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ind_jump_chaining:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump_chaining)
	adcl	$0x0, (fbt_nr_ind_jump_chaining+0x4)
#endif

	# free the trampoline slot where we came from
	movl	4(%ebp), %eax
	subl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	trampoline_free

	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pn	.translated_ijc	# ,pn = branch prediction hint: do not take
	
	
	
	# check if we can overwrite jump to trampoline
	# this is only the case if no other TU was translated since this trampoline was constructed
	movl	8(%ebp), %ecx	# address of tld
	addl	$OFFSETOF_TLD_TS_TRANSL_INSTR, %ecx	# move to ts.transl_instr within tld
	
	
	# compare tld->ts->transl_instr to origin (origin should be 4 smaller if we can overwrite jmp)
	movl	16(%ebp), %edx	# move origin to edx
	addl	$4, %edx	# increase edx by 4
	cmpl	%edx, (%ecx)	# compare origin to transl_instr
	
	jne	.no_fallthru_ijc
	
	
	subl	$5, (%ecx)	# decrease tld->ts.transl_instr by 5, so jump to this trampoline gets overwritten with next CCF 
				# (fallthrough optimization)
	
	call	translate_noexecute
	
	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)
	
	jmp	.done_ijc
	
.no_fallthru_ijc:
	# translate target if we cannot do fallthrough optimization
	call	translate_noexecute	
	
.translated_ijc:
	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)
	
	# chaining optimization
	# rewrite jump target of the jump to the trampoline: translated_address - origin - 4
	# only the jump target (the last 4 bytes of the last instruction) need to be modified
	movl	16(%ebp), %ecx
	subl	$4, %eax
	subl	%ecx, %eax
	movl	%eax, (%ecx)

.done_ijc:
	# readjust stack
	addl 	$16, %esp
	
	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$12
	.size	ind_jump_chaining, .-ind_jump_chaining

#if defined(FBT_IND_CALL_PREDICTION)
# This function fixes a ret
# checks if the target is already translated and fixes
# the return instruction pointer
# backpatches the target as a prediction for next time
# Stack:
#     +-------------+
#  16 | target IP   | (gets removed by ret $12)
#  12 | tld         | (gets removed by ret $12)
#   8 | cmp target  | (gets removed by ret $12 - gets patched)
#   4 | eip (call)  |
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
fix_ind_call_prediction:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_calls_miss)
	adcl	$0x0, (fbt_nr_ind_calls_miss+4)
#endif

	# translate the target IP if necessary
	pushl	16(%ebp)
	pushl	12(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_ficp	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_ficp:
	# backpatch prediction:
	movl	4(%ebp), %ecx		# get eip from mispredicted callsite
	movl	%eax, %edx		# first jmp (relative): translated_address - origin - 4 = ta
	subl	%ecx, %edx		#                                                         -(origin-6)
					#	-6 because 1push (5b) + 1b of jmp
	subl	$10, %edx 		#                                                         -10
	movl	%edx, 6(%ecx)		# offset depends on emitted code!
	movl	16(%ebp), %edx
	movl	8(%ebp), %ecx
	movl	%edx, (%ecx)		# overwrite compare-target with new target value

	# readjust stack
	addl	$8, %esp

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	ret $12
	.size	fix_ind_call_prediction, .-fix_ind_call_prediction
#endif

#if defined(FBT_RET_PREDICTION)

# This function fixes a ret
# checks if the target is already translated and fixes
# the return instruction pointer
# backpatches the target as a prediction for next time
# Stack:
#     +-------------+
#  12 | target IP   | (will be removed by this function) 
#   8 | tld-pointer | (will be removed by this function)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ret_predict:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ret_predict_miss)
	adcl	$0x0, (fbt_nr_ret_predict_miss+4)
#endif

	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_rp	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_rp:

	# backpatch prediction:
	movl	4(%ebp), %ecx		# get (not overwritten yet) origin-4 (4 depends on emitted code!)
	movl	%eax, %edx			# first jmp (relative): translated_address - origin - 4 = ta
	subl	%ecx, %edx			#                                                         -(origin -4)
	subl	$8, %edx 			#                                                         -8
	movl	%edx, 4(%ecx)		# 4 depends on emitted code!
	movl	12(%ebp), %edx

	movl	%edx, -16(%ecx)		# -16 depends on emitted code!

	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)

	# readjust stack
	addl	$8, %esp

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	ret $8
	.size	ret_predict, .-ret_predict

# This function fixes a ret imm16
# checks if the target is already translated and fixes
# the return instruction pointer
# backpatches the target as a prediction for next time
# Stack:
#     +-------------+
#  ?? | xxxxxxxxxxx | (will be removed by this function)
#  16 | target IP   | (will be removed by this function) 
#  12 | num_bytes   | (will be removed by this function)
#   8 | tld-pointer | (will be removed by this function)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ret_predict_remove:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ret_predict_miss_remove)
	adcl	0x0, 0x4(fbt_nr_ret_predict_miss_remove)
#endif

	# translate the target IP if necessary
	pushl	16(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_rpr	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_rpr:
	# backpatch prediction:
	movl	4(%ebp), %ecx		# get (not overwritten yet) origin-7 (7 depends on emitted code!)
	movl	%eax, %edx			# first jmp (relative): translated_address - origin - 4 = ta
	subl	%ecx, %edx			#                                                         -(origin -7)
	subl	$11, %edx 			#                                                         -11
	movl	%edx, 7(%ecx)		# 7 depends on emitted code!
	movl	16(%ebp), %edx
	movl	%edx, -21(%ecx)		# -21 depends on emitted code!
	
	
	# overwrite return instruction pointer
	# taking into account the number of bytes which should be removed from the stack
	movl	%ebp, %ecx
	addl	12(%ebp), %ecx
	movl	%eax, 16(%ecx)

	# readjust stack
	addl	$8, %esp

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	 # remove the requested number of bytes from the esp
	addl	8(%esp), %esp
	addl	$12, %esp
	ret
	.size	ret_predict_remove, .-ret_predict_remove

#endif // ret_prediction


#if defined(FBT_RET_STACK)

# This function is called when the ret (shadow) stack gets out of sync with the original return address stack
# It tries to resynch by discarding elements or, if that fails, calling tcache_find.
# Stack:
#     +-------------+
#  16 | orig ret.a. |
#  12 | eax         | (saved, ignore)
#   8 | tld         | (gets removed by ret $4)
#	4 | eip (call)	|
#   0 | old ebp     | <- ebp
#  -4 | ecx         |
#  -8 | edx         | <- esp
#     +-------------+
ret_stack_synchronize:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ret_stack_synchronize)
	adcl	0x0, 0x4(fbt_nr_ret_stack_synchronize)
#endif

# try resynch by discarding elements
	movl	8(%ebp), %edx
	movl	OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS(%edx), %eax	# eax = tld->translated_call_stack_tos
	movl	OFFSETOF_TLD_TRANSLATED_CALL_STACK_END(%edx), %ecx	# ecx = tld->translated_call_stack_end
	jmp		.cond_rss
.next_rss:
	addl	$8, %eax		# pop stack

	movl	OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR(%eax), %edx		# if(tos->orig == orig_ret_a) goto found_rss
	cmpl	%edx, 16(%ebp)
	je .found_rss

.cond_rss:
	cmpl	%ecx, %eax		# while !(stack empty)
	jne	.next_rss

# stack empty -> init with result from tcache
	subl	$8, %eax
	movl	16(%ebp), %ecx		#tos->orig = orig
	movl	%ecx, OFFSETOF_RET_STACK_ENTRY_ORIG_ADDR(%eax)
	pushl	%eax	
	pushl	%ecx			#tos->transl = tcache_find
	pushl	8(%ebp)
	call	tcache_find
	testl	%eax, %eax
	jne,pt .translated_rss	# ,pt = branch prediction hint: take
	call	translate_noexecute
.translated_rss:
	addl	$8, %esp
	popl	%ecx
	movl	%eax, OFFSETOF_RET_STACK_ENTRY_TRANSL_ADDR(%ecx)
	movl	%ecx, %eax

.found_rss:
	movl	8(%ebp), %edx		#tld->translated_call_stack_tos = eax
	movl	%eax, OFFSETOF_TLD_TRANSLATED_CALL_STACK_TOS(%edx)

	popl	%edx
	popl	%ecx
	leave
	ret	$4
	.size	ret_stack_synchronize, .-ret_stack_synchronize

#endif // FBT_RET_STACK

# find the translation cache index entry for the given TU address.
# But only use 2 registers and do not create a stackframe and assume no hash collision.
# In case there is a hash collision, fall back to fcache_find.
# This function trashes the flags, %eax and %ecx!
# Stack:
#     +-------------+
#   8 | tu-address  | (will not be removed by this function)
#   4 | tld-pointer | (will not be removed by this function)
#   0 | eip (call)  |<- esp
#     +-------------+
tcache_find_fast:
#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_tcache_fast_lookups)
	adcl	$0x0, (fbt_nr_tcache_fast_lookups+0x4)
#endif
	movl	8(%esp), %eax		# load tu_address
	leal	(,%eax, 4),%eax		# offset = (tu_address << 1) & HASH_PATTERN
	andl	$HASH_PATTERN, %eax
	movl	4(%esp), %ecx		# entry = tld->hashtable + offset
	addl	OFFSETOF_TLD_HASHTABLE(%ecx), %eax
	
.load_compare_tff:
	movl	OFFSETOF_TCACHE_ENTRY_SRC(%eax), %ecx

	cmpl	8(%esp), %ecx					# if (entry->src != tu_address) goto fallback_tff
	jne .fallback_tff
	movl	OFFSETOF_TCACHE_ENTRY_DST(%eax), %eax		# return entry->dst
	ret

.fallback_tff:
	jecxz .null_tff						# if (entry->src == 0) goto null_tff
#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_tcache_fast_miss)
	adcl	$0x0, (fbt_nr_tcache_fast_miss+0x4)
#endif
# 	pushl %edx				# save %edx
# 	pushl 12(%esp)				# pass on parameters
# 	pushl 12(%esp)
# 	call tcache_find
# 	addl $8, %esp
# 	popl %edx
# 	ret
	addl	$SIZEOF_HASHTABLE_ENTRY, %eax	# iterate eax to the next row
	cmpl	$0x1, %ecx			# check for guard
	jne,pt	.load_compare_tff
#	movl	4(%esp), %ecx			# entry = tld->hashtable + offset
#	movl	OFFSETOF_TLD_HASHTABLE(%ecx), %ecx
#	addl	$HASHTABLE_SIZE, %ecx		# get maximum hash_value
#	cmpl	%ecx, %eax
#	jne,pt	.load_compare_tff
	subl	$HASHTABLE_SIZE, %ecx
	subl	$HASHTABLE_SIZE, %ecx
	jmp	.load_compare_tff

.null_tff:
	xorl	%eax, %eax			# return 0
	ret

# this function prepares for a sysenter instruction
# WARNING: this does only work on linux with linux-gate.so!!!
# WARNING: the syscall sees our translated code as return pointer
#   we have to do this, because sysenter uses the RIP from the
#   stack to hand control back to the user program!
#   we can not leave the userspace-ptr on the stack, otherwise we
#   loose control over the program and return to untranslated code!
# Every sysenter is called as follows:
#  push %ecx
#  push %edx
#  push %ebp
#  mov %esp, %ebp
#  sysenter
# we use this special prefix!
# http://manugarg.googlepages.com/systemcallinlinux2_6.html
# Stack:
#     +-------------+
#  24 | eip (call)  | <- gets patched (to translated code)
#  20 | old eax     |
#  16 | old ecx     |
#  12 | old edx     |
#   8 | tld-pointer | (gets removed by ret $4)
#   4 | eip (se)    | (this is the eip to sysenter)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
prepare_sysenter:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

	# translate the target IP if necessary
	pushl	24(%ebp)
	pushl	8(%ebp)
	call	tcache_find
	testl	%eax, %eax
	jne .translated_se
	call	translate_noexecute

.translated_se:
	# overwrite return instruction pointer
	movl	%eax, 24(%ebp)

	# readjust stack
	addl	$8, %esp

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$4
	.size	prepare_sysenter, .-prepare_sysenter



# ends a transaction and jumps back to untranslated code
# (and prints a success message)
end_transaction:
	pushl	%ebp
	movl	%esp, %ebp

	pushfl		# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#	pushl	$SUCCSTR	# print success message
#	call	printf
#	addl	$4, %esp	# clear stack (printf argument)

	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl

#if defined(FBT_STATISTIC)
	call	print_statistics
#endif
	
	leave
	ret
	.size	end_transaction, .-end_transaction



# Prints some debug message
# Does not change stack!
print_eip:
	pushl	%ebp
	movl	%esp, %ebp

	pushfl			# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx

	pushl	%ebp
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%eax
	pushl	12(%ebp)
	pushl	$EIP	# print jmp-src message
	call	printf
	addl 	$28, %esp	# readjust stack

	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret
	.size	print_eip, .-print_eip

# Prints some debug message
# Does not change stack!
print_eip2:
	pushl	%ebp
	movl	%esp, %ebp

	pushfl			# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx

	pushl	%eax
	movl	(%ebp), %eax
	pushl	4(%eax)
	pushl	12(%eax)
	pushl	$EIP2	# print jmp-src message
	call	printf
	addl 	$16, %esp	# readjust stack

	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret
	.size	print_eip2, .-print_eip2

# This function fixes an indirect jump
# checks if the target is already translated and fixes
# the return instruction pointer
# Stack:
#     +-------------+
#  12 | target IP   | (gets removed by ret $8)
#   8 | tld-pointer | (gets removed by ret $8)
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
ind_jump_signal:
	pushl	%ebp
	movl	%esp, %ebp

	
	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx


	# lets get the tid
	pushl	tld_key
	call	pthread_getspecific
	movl	%eax, 8(%ebp)	# overwrite tld
	addl	$4, %esp
	
	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_ijs	# ,pt = branch prediction hint: take
	call	translate_noexecute
.translated_ijs:
	# overwrite return instruction pointer
	movl	%eax, 4(%ebp)

	# re-adjust the stack
	addl	$8, %esp

	# pop caller-save and eflags registers

	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$8
	.size	ind_jump_signal, .-ind_jump_signal


#if defined(LDPRELOAD)
#if defined(HIJACKCONTROL)

# Remove the two comments below before
# .section .init
# and
# .section .fini
# to let LD_PRELOAD magic happen

.section .init
	pushf
	pusha
	call	print_eip
	sub	$0x4, %esp
	movl	$0x2, (%esp)	# push parameter (2)
	call	sleep		# sleep(2); 
	movl	$0x0, (%esp)	# push parameter (0)
	call	fbt_init	# fbt_init(NULL); 
	movl	$fbt_commit_transaction, (%esp)	# push function ptr
	call	fbt_start_transaction		# fbt_start_transaction(..)
	add	$0x4, %esp
	popa
	popf

.section .fini
	pushf
	pusha
	call fbt_commit_transaction
	popa
	popf

#endif // hijack control
#endif // ld preload fun
