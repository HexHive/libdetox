#
# This file defines all the in assembly implemented functions that are
# used mostly in the translate actions and when the bt is started or stopped
#
# Copyright (c) 2008 ETH Zurich
#   Mathias Payer <mathias.payer@inf.ethz.ch>
#   Marcel Wirth <mawirth@student.ethz.ch>
#   Stephan Classen <scl@soft-eng.ch>
#   Antonio Barresi <abarresi@student.ethz.ch>
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
#include "fbt_mem_protection.h"
#include "fbt_asm_offsets.h"

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

#ifdef SECU_MPROTECT_IDS
#  define MPROTECT_UNLOCK_SAFE(tld) \
	pushl	%eax; \
	pushl	%ecx; \
	pushl	%edx; \
	pushl	tld; \
	call	fbt_ids_unlock; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp; \
	popl	%edx; \
	popl	%ecx; \
	popl	%eax;
#  define MPROTECT_UNLOCK(tld) \
	pushl	tld; \
	call	fbt_ids_unlock; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp;
#  define MPROTECT_LOCKDOWN(tld) \
	pushl	tld; \
	call	fbt_ids_lockdown; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp;
#  define MPROTECT_LOCKDOWN_SAFE(tld) \
	pushl	%eax; \
	pushl	tld; \
	call	fbt_ids_lockdown; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp; \
	popl	%eax;
#else
#  define MPROTECT_UNLOCK(tld)
#  define MPROTECT_UNLOCK_SAFE(tld)
#  define MPROTECT_LOCKDOWN(tld)
#  define MPROTECT_LOCKDOWN_SAFE(tld)
#endif

# use optimized memory protection, no need for eager locks, do it lazily
#ifdef TRUST_MEMPROTECT
#undef MPROTECT_UNLOCK
#undef MPROTECT_UNLOCK_SAFE
#undef MPROTECT_LOCKDOWN
#undef MPROTECT_LOCKDOWN_SAFE
/*
#  define MPROTECT_LOCKDOWN(tld)
#  define MPROTECT_LOCKDOWN_SAFE(tld)
#  define MPROTECT_UNLOCK_PAGE(tld, page)
#  define MPROTECT_UNLOCKTLD_NOARG
*/
#  define MPROTECT_UNLOCK_SAFE(tld) \
	pushl	%eax; \
	pushl	%ecx; \
	pushl	%edx; \
	pushl	tld; \
	call	fbt_ids_unlocktld; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp; \
	popl	%edx; \
	popl	%ecx; \
	popl	%eax;
#  define MPROTECT_UNLOCK(tld) \
	pushl	tld; \
	call	fbt_ids_unlocktld; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp;
#  define MPROTECT_LOCKDOWN(tld) \
	pushl	tld; \
	call	fbt_ids_lock_list; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp;
#  define MPROTECT_LOCKDOWN_SAFE(tld) \
	pushl	%eax; \
	pushl	%ecx; \
	pushl	%edx; \
	pushl	tld; \
	call	fbt_ids_lock_list; \
	movl	$0, (%esp); \
	leal	4(%esp), %esp; \
	popl	%edx; \
	popl	%ecx; \
	popl	%eax;
#  define MPROTECT_UNLOCK_PAGE(tld, page) \
	pushl	%eax; \
	pushl	%ecx; \
	pushl	%edx; \
	pushl	page; \
	pushl	tld; \
	call	fbt_ids_unlock_page; \
	movl	$0, (%esp); \
	leal	8(%esp), %esp; \
	popl	%edx; \
	popl	%ecx; \
	popl	%eax;
#  define MPROTECT_UNLOCKTLD_NOARG \
	pushl	%eax; \
	pushl	%ecx; \
	pushl	%edx; \
	call	fbt_ids_unlocktld_noarg; \
	movl	$0, (%esp); \
	popl	%edx; \
	popl	%ecx; \
	popl	%eax;
#else
#  define MPROTECT_UNLOCK_PAGE(tld, page)
#  define MPROTECT_UNLOCKTLD_NOARG
#endif


#ifndef RTLD_DEFAULT
#  define RTLD_DEFAULT 0
#endif
#ifndef RTLD_NEXT
#  define RTLD_NEXT -1
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

#if defined(FBT_IND_JUMP_MULTIPLE)
	.global fix_ind_jmp_prediction
	.type	fix_ind_jmp_prediction, @function
	.global tcache_find_fast
	.type	tcache_find_fast, @function
#endif

#if defined(FBT_IND_PREDICTION)
	.global fix_ind_jmp_prediction
	.type	fix_ind_jmp_prediction, @function
#endif

#if defined(FBT_RET_PREDICT)
	.global fix_ret_predict
	.type	fix_ret_predict, @function
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

	.global	authorize_sysenter
	.type	authorize_sysenter, @function

	.global	authorize_int0x80
	.type	authorize_int0x80, @function

	.global	end_transaction
	.type	end_transaction, @function

	.global	end_transaction_clean
	.type	end_transaction_clean, @function

	.global	print_eip
	.type	print_eip, @function

	.global ind_jump_signal
	.type	ind_jump_signal, @function

	.global dlsym_handler
	.type	dlsym_handler, @function

	.global dlvsym_handler
	.type	dlvsym_handler, @function

	.global fbt_do_nothing
	.type	fbt_do_nothing, @function

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

	MPROTECT_UNLOCKTLD_NOARG
#if !defined(FBT_IND_JUMP_INLINE_FIND_FAST)
#	MPROTECT_UNLOCK(TLDPTR)
#else
#	MPROTECT_UNLOCK(24(%esp))
#endif

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
	leal	(,%eax, 8),%eax		# offset = (tu_address << 3) & HASH_PATTERN
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
	MPROTECT_UNLOCKTLD_NOARG
#if !defined(FBT_IND_JUMP_INLINE_FIND_FAST)
#	MPROTECT_UNLOCK_SAFE(TLDPTR)
#else
#	SAVE_EDX_FIND_FAST
#	pushl	%eax
#	pushl	TLDPTR
#	call fbt_ids_unlocktld
#	addl	$4, %esp
#	popl	%eax
#	RESTORE_EDX_FIND_FAST
#endif
	# write ind_jump location
	movl	TLDPTR, %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %eax
	movl	%eax, EIPPTR
	
	# overwrite tld pointer
	movl	$0, TLDPTR

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
#ifdef SECU_MPROTECT_IDS
#ifdef TRUST_MEMPROTECT
	call	fbt_ids_unlocktld
#else
	call	fbt_ids_unlock
#endif
#endif
	call	translate_noexecute
#ifdef SECU_MPROTECT_IDS
	pushl	%eax
	pushl	4(%esp)
#ifdef TRUST_MEMPROTECT
	call	fbt_ids_lock_list
#else
	call	fbt_ids_lockdown
#endif
	addl	$4, %esp
	popl	%eax
#endif	
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
	MPROTECT_UNLOCK(8(%ebp))
	pushl	16(%ebp)
	pushl	8(%ebp)
	call	translate_noexecute
	addl	$8, %esp
	MPROTECT_LOCKDOWN_SAFE(8(%ebp))
	RESTORE_EDX_FIND_FAST

.translated_ijr:
	MPROTECT_UNLOCKTLD_NOARG
#	MPROTECT_UNLOCK_SAFE(8(%ebp))

	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)

	# overwrite return instruction pointer to point to the jump-back trampoline
	# taking into account the number of bytes which should be removed from the stack
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %eax
	movl	%ebp, %ecx
	addl	12(%ebp), %ecx
	movl	%eax, 16(%ecx)

	# readjust stack
	addl	$8, %esp
	
	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
	
	MPROTECT_UNLOCK(8(%ebp))

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
	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)

	# readjust stack
	addl 	$16, %esp

	# rewrite jump target of the jump to the trampoline: translated_address - origin - 4
	# only the jump target (the last 4 bytes of the last instruction) need to be modified
	movl	16(%ebp), %ecx
	subl	$4, %eax
	subl	%ecx, %eax

	MPROTECT_UNLOCK_PAGE(8(%ebp), %ecx)
	movl	%eax, (%ecx)
	
	MPROTECT_LOCKDOWN(8(%ebp))

	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
	
	MPROTECT_UNLOCK(8(%ebp))

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
	
	# write ind_jump location
	movl	8(%ebp), %edx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%edx)

	# walk shadow stack and patch all entries that point to this trampoline
	pushl 	%ebx
	movl	%eax, %ebx			# ebx: translated address 
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
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	8(%ebp), %ecx
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)

	# readjust stack
	addl 	$16, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))

	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
# This function is used for the chaining/fallthrough optimization in fbt_translate.c
# Ensures that flags remain unchanged!
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
	
	MPROTECT_UNLOCK(8(%ebp))

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
	
	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)
	
	jmp	.done_ijc
	
.no_fallthru_ijc:
	# translate target if we cannot do fallthrough optimization
	call	translate_noexecute	
	
.translated_ijc:
	MPROTECT_UNLOCKTLD_NOARG
#	MPROTECT_UNLOCK_SAFE(8(%ebp))
	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)
	
	# chaining optimization
	# rewrite jump target of the jump to the trampoline: translated_address - origin - 4
	# only the jump target (the last 4 bytes of the last instruction) need to be modified
	movl	16(%ebp), %ecx
	subl	$4, %eax
	subl	%ecx, %eax

	MPROTECT_UNLOCK_PAGE(8(%ebp), %ecx)
	movl	%eax, (%ecx)

.done_ijc:
	# readjust stack
	addl 	$16, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))
	
	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
#  24 | src IP      | (for call)
#  20 | target IP   | (gets removed by ret $12)
#  16 | tld         | (gets removed by ret $12)
#  12 | cmp target  | (gets removed by ret $12 - gets patched)
#   8 | cmp target2 | (gets removed by ret $12 - gets patched)
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

	MPROTECT_UNLOCK(16(%ebp))
	
	# translate the target IP if necessary
	pushl	20(%ebp)
	pushl	16(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_ficp	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_ficp:
	# backpatch prediction:
	# test if we should remove this prediction
	movl	4(%ebp), %ecx
	MPROTECT_UNLOCK_PAGE(16(%ebp), %ecx)
	incl	(%ecx)
	cmpl	$0x20, (%ecx)
	jne .fix_ficp
	
#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_translated_call_ind_pred_removed)
#endif
	# ok, let's remove that stale prediction with an ind. jump
	movl	8(%ebp), %edx		# load addr of target2
	movl	12(%ebp), %ecx		# load addr of old target1
	subl	%ecx, %edx		# edx = addr of target2 - addr of target1
	subl	%edx, %ecx		# ecx+10 = start of cmpl instr (6 - je & offset, 4 imm of cmpl)
	movb	$0xff, 10(%ecx)		# pushl instr
	andb	$0xf7, 11(%ecx)		# modify opcode extension from 111 to 110 (cmpl -> pushl)

	movl	12(%ebp), %ecx		# load addr of old target1
	movb	$0xe9, (%ecx)		# jmp
	
	movl	12(%ebp), %edx
	addl	$1, %edx		# target
	
	movl	16(%ebp), %ecx		# load tld	
	movl	OFFSETOF_IND_CALL_TRAMPOLINE(%ecx), %ecx # dst
	# rel32 - dst - 4
	subl	%edx, %ecx		# target - dst
	subl	$4, %ecx
	MPROTECT_UNLOCK_PAGE(16(%ebp), %edx)
	movl	%ecx, (%edx)
	jmp	.leave_ficp
	
	
.fix_ficp:	
	# mov pred1 to pred2
	movl	12(%ebp), %ecx
	movl	(%ecx), %ecx		# load old target1
	movl	8(%ebp), %edx
	MPROTECT_UNLOCK_PAGE(16(%ebp), %edx)
	movl	%ecx, (%edx)		# save old target1 in target2

	movl	8(%ebp), %edx		# load addr of target2
	movl	12(%ebp), %ecx		# load addr of old target1
	subl	%ecx, %edx		# edx = addr of target2 - addr of target1
	movl	6(%ecx), %ecx		# load addr of old dest1 (offset depends on emitted code!)
	subl	%edx, %ecx		# ecx = new relative jump dest of dest1 ( target = target - (addr of target2 - addr of target1) )
	movl	8(%ebp), %edx		# load addr of target2
	movl	%ecx, 6(%edx)		# save (translated) dest1 in dest2 (offset depends on emitted code!)

	# save new target1
	movl	12(%ebp), %ecx
	movl	20(%ebp), %edx
	MPROTECT_UNLOCK_PAGE(16(%ebp), %ecx)
	movl	%edx, (%ecx)		# overwrite target1 with new target

	# save new dest1
	# relative jump:	 translated_address (eax) - origin (12(%ebx)-6) - 4
	movl	12(%ebp), %ecx
	# rel target:  end_of_jump_instr - dest
	# end of jump instr:	 10(target1) (6+4)
	# store at 6(target1) = 10(target1) - dest
 	movl	%eax, %edx		# first jmp (relative): translated_address - origin - 4 = ta
 	subl	%ecx, %edx		#                                                         -(origin-6)
 					#	-6 because 1imm32 (4b) + 2b of jmp
 	subl	$10, %edx 		#                                                         -10
 	movl	%edx, 6(%ecx)		# offset depends on emitted code!

.leave_ficp:
	# readjust stack
	addl	$8, %esp
	

	movl	%eax, 4(%ebp) # overwrite eip

	MPROTECT_LOCKDOWN(16(%ebp))
	# overwrite tld pointer
	movl	$0, 16(%ebp)

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	ret $16
	.size	fix_ind_call_prediction, .-fix_ind_call_prediction
#endif


#if 0
tester:
	movzx	4(%esp), %ebx
	#movl	*0x11223344(, %ebx, 4), %ebx
	#jmpl	*(%ebx)
	movl	$0x11223344, (0x33445566)
	jmpl	*0x11223344(, %ecx, 4)
	cmpl 0x11223344(,%ebx,4), %ecx
	movl %ebx, -4(%esp)
	movl %ecx, -8(%esp)
	movl %ecx, %ebx
	movl (%esp), %ebx
	movl -8(%esp), %ecx
	movl -4(%esp), %ebx
	movl %ebx, 4(%esp)
	cmpl  $0x11223344, 4(%esp)
	jz   hit
	popl  ebx
	jmpl  *0x66778899
hit:
	popl  %ebx
	leal  4(%esp), %esp

	.size tester, .-tester
#endif
	
#if defined(FBT_IND_JUMP_MULTIPLE) || defined(FBT_IND_PREDICTION)
# This function fixes an indirect jump prediction, it first looks up the target
# and then writes the resolved target to the cmp instruction and the jmp instruction in the ccf.
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
fix_ind_jmp_prediction:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ind_jump_pred_miss)
	adcl	$0x0, (fbt_nr_ind_jump_pred_miss+4)
#endif

	MPROTECT_UNLOCK(12(%ebp))
	
	# translate the target IP if necessary
	pushl	16(%ebp)
	pushl	12(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_fijp	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_fijp:
	# test if we should remove this prediction
	movl	4(%ebp), %ecx
	MPROTECT_UNLOCK_PAGE(12(%ebp), %ecx)
	incl	(%ecx)
	cmpl	$0x20, (%ecx)
	jne .fix_fijp

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_translated_jump_ind_pred_removed)
#endif
	movl	4(%ecx), %ecx		# load pushfl target
	movb	$0x90, (%ecx)		# nop
	movb	$0xff, 1(%ecx)		# cmpl -> pushl
	andb	$0xf7, 2(%ecx)		# modify opcode extension from 111 to 110 (cmpl -> pushl)
	
	movl	8(%ebp), %ecx		# load addr of old target1
	movb	$0xe9, (%ecx)		# jmp
	
	movl	8(%ebp), %edx
	addl	$1, %edx		# target
	
	movl	12(%ebp), %ecx		# load tld	
	movl	OFFSETOF_IND_JUMP_TRAMPOLINE(%ecx), %ecx # dst
	# rel32 - dst - 4
	subl	%edx, %ecx		# target - dst
	subl	$4, %ecx
	MPROTECT_UNLOCK_PAGE(12(%ebp), %edx)
	movl	%ecx, (%edx)
	jmp	.leave_fijp
	
.fix_fijp:	
	# backpatch prediction:
	# TODO:	 remove (stale old code?!)
	#movl	4(%ebp), %ecx		# get eip from mispredicted callsite
	#movl	%eax, %edx		# first jmp (relative): translated_address - origin - 4 = ta
	#subl	%ecx, %edx		#                                                         -(origin-1)
					#	-1 because 1b of jmp
	#subl	$5, %edx 		#                                                         -5
	#movl	%edx, 1(%ecx)		# offset depends on emitted code!
	
	movl	16(%ebp), %edx
	movl	8(%ebp), %ecx
	MPROTECT_UNLOCK_PAGE(12(%ebp), %ecx)
	movl	%edx, (%ecx)		# overwrite compare-target with new target value

	# backpatch first jmp in case prediction was correct, depends on emitted code!
	movl	%eax, %edx		# load target to %edx
	addl	$8, %ecx		# add 8 to address of compare-target -> points to location of immediate value of jmp instruction that has to be backpatched
	subl	%ecx, %edx		# calculate the relative jmp distance
	subl	$4, %edx		# subtract 4 because ecx + 8 points to the immediate value location but we need a value relative to the current ip
					# current ip is ecx + 8 + 4, the next instruction after jmp
	movl	%edx, (%ecx)		# overwrite relative jmp address
	
.leave_fijp:	
	# readjust stack
	addl	$8, %esp

	# save translated target to ind_jump_target (tld)
	#movl	12(%ebp), %ecx
	#movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)

	movl	%eax, 4(%ebp) # overwrite eip
	MPROTECT_LOCKDOWN(12(%ebp))

	# overwrite tld pointer
	movl	$0, 12(%ebp)

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	ret $12
	.size	fix_ind_jmp_prediction, .-fix_ind_jmp_prediction
#endif // ind_jump_multiple || ind_prediction

#if defined(FBT_RET_PREDICT)
# This function fixes a ret
# checks if the target is already translated and fixes
# the return instruction pointer
# backpatches the target as a prediction for next time
# Stack:
#     +-------------+
#  12 | target IP   | (gets removed by ret $8)
#   8 | tld         | (gets removed by ret $8)
#   4 | eip (call)  |
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         | <- esp
#     +-------------+
fix_ret_predict:
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

	MPROTECT_UNLOCK(16(%ebp))
	
	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_frp	# ,pt = branch prediction hint: take
	call	translate_noexecute

.translated_frp:
	# backpatch prediction:

	# test if we should remove this prediction
	movl	4(%ebp), %ecx
	MPROTECT_UNLOCK_PAGE(8(%ebp), %ecx)
	incl	(%ecx)
	cmpl	$0x20, (%ecx)
	jne .fix_frp
	# ok, let's remove that stale prediction with an ind. jump
	# we remove the whole prediction and rewrite the
	# first 5 bytes with a jmp ind_jump_trampoline
	
	movl	4(%ebp), %ecx
	leal	-50(%ecx), %ecx		# load addr of addl instr

 	movb	$0xe9, (%ecx)		# jmp
	
 	addl	$1, %ecx		# target
	
 	movl	8(%ebp), %edx		# load tld	
 	movl	OFFSETOF_IND_CALL_TRAMPOLINE(%edx), %edx # dst
 	# rel32 - dst - 4
 	subl	%ecx, %edx		# target - dst
 	subl	$4, %edx
 	movl	%edx, (%ecx)
 	jmp	.leave_frp
	
	
.fix_frp:	
	# mov pred1 to pred2
	movl	4(%ebp), %edx
	
	movl	-43(%edx), %ecx		# load old target1	
	movl	%ecx, -29(%edx)		# save old target1 in target2

	
	movl	-37(%edx), %ecx		# load addr of old dest1 (offset depends on emitted code!)
	subl	$14, %ecx		# ecx = new relative jump dest of dest1 ( target = target - (&target2-&target1=14) )
	movl	%ecx, -23(%edx)		# save (translated) dest1 in dest2 (offset depends on emitted code!)

	# save new target1
	movl	-18(%ebp), %ecx
	movl	%ecx, -43(%edx)		# overwrite target1 with new target

	# save new dest1
	# relative jump:	 translated_address (eax) - origin (12(%ebx)-6) - 4
	leal	-37(%edx), %ecx
	# rel target:  end_of_jump_instr - dest
	# end of jump instr:	 10(target1) (6+4)
	# store at 6(target1) = 10(target1) - dest
 	movl	%eax, %edx		# first jmp (relative): translated_address - origin - 4 = ta
 	subl	%ecx, %edx		#                                                         -(origin-6)
 					#	-6 because 1imm32 (4b) + 2b of jmp
 	subl	$4, %edx 		#                                                         -10
 	movl	%edx, (%ecx)		# offset depends on emitted code!

.leave_frp:
	# readjust stack
	addl	$8, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))

	# overwrite tld pointer
	movl	$0, 8(%ebp)
	
	movl	%eax, 4(%ebp) # overwrite eip

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave

	ret $8
	.size	fix_ret_predict, .-fix_ret_predict
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
	
	MPROTECT_UNLOCK(8(%ebp))

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

	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)
	
	# readjust stack
	addl	$8, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))

	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
	
	MPROTECT_UNLOCK(8(%ebp))

#if defined(FBT_STATISTIC)
	# long long increment
	addl	$0x1, (fbt_nr_ret_predict_miss_remove)
	adcl	$0x0, (fbt_nr_ret_predict_miss_remove+4)
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
	
	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	
	# overwrite return instruction pointer
	# taking into account the number of bytes which should be removed from the stack
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%ebp, %ecx
	addl	12(%ebp), %ecx
	movl	%edx, 16(%ecx)

	# readjust stack
	addl	$8, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))

	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
	adcl	$0x0, (fbt_nr_ret_stack_synchronize+4)
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

	# overwrite tld pointer
	movl	$0, 8(%ebp)

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
	leal	(,%eax, 8),%eax		# offset = (tu_address << 3) & HASH_PATTERN
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
	#use tcache_find (slow and c!)
 	#pushl %edx				# save %edx
 	#pushl 12(%esp)				# pass on parameters
 	#pushl 12(%esp)
 	#call tcache_find
 	#addl $8, %esp
 	#popl %edx
 	#ret
	
	addl	$SIZEOF_HASHTABLE_ENTRY, %eax	# iterate eax to the next row
	cmpl	$0x1, %ecx			# check for guard
	jne,pt	.load_compare_tff
#	movl	4(%esp), %ecx			# entry = tld->hashtable + offset
#	movl	OFFSETOF_TLD_HASHTABLE(%ecx), %ecx
#	addl	$HASHTABLE_SIZE, %ecx		# get maximum hash_value
#	cmpl	%ecx, %eax
#	jne,pt	.load_compare_tff
	subl	$HASHTABLE_SIZE, %ecx
#	subl	$HASHTABLE_SIZE, %ecx
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
#  20 | eip (call)  | <- gets patched (to translated code)
#  16 | old ecx     |
#  12 | old edx     |
#   8 | old ebp     |
#   4 | tld-pointer | (gets removed by ret $4)
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
	
	MPROTECT_UNLOCK(4(%ebp))

	# translate the target IP if necessary
	pushl	20(%ebp)
	pushl	4(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne .translated_se
	call	translate_noexecute

.translated_se:
	# write ind_jump_target for the jump-back trampoline
	movl	4(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite system call return address with address of ret jump-back tramp
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 20(%ebp)
	
	# readjust stack
	addl	$8, %esp
	
	MPROTECT_LOCKDOWN(4(%ebp))

	# overwrite tld pointer
	movl	$0, 4(%ebp)

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	leal 4(%esp), %esp	# pop tld-pointer from stack
	sysenter
	.size	prepare_sysenter, .-prepare_sysenter


# This is not a real function anymore. To use authorize_sysenter, you must NOT 
# call it, but jump to it. There must not be a return instruction pointer on 
# the stack, or authorize_sysenter will segfault.
#     +-------------+
#  20 | eip (call)  | <- gets patched (to translated code)
#  16 | old ecx     |
#  12 | old edx     |
#   8 | old ebp     |
#   4 | tld-pointer | (gets removed by ret $4)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         |
# -20 | fake retval |
# -20 | auth retval | <- esp
#     +-------------+
authorize_sysenter:
	pushl	%ebp
	movl	%esp, %ebp

	# push eflags and caller-save registers
	pushfl
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	
	MPROTECT_UNLOCK_SAFE(4(%ebp))

	# push arguments to system call onto the stack
	# as arguments for authorize_syscall
	pushl $-1	# default for fake retval if not authorized
	pushl %esp	# pointer to fake retval
	pushl $1	# tag as sysenter triggered system call
	pushl (%ebp)	# push original %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax


	# call authorize_syscall
	call *authorize_syscl_table( , %eax, 4)
	addl $32, %esp			# adjust stack
	movl %eax, (%esp)		# save return value

	# translate the target IP if necessary
	pushl	20(%ebp)
	pushl	4(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne .translated_ase
	call	translate_noexecute		# translate

.translated_ase:
	# write ind_jump_target for the jump-back trampoline
	movl	4(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite system call return address with address of ret jump-back tramp
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 20(%ebp)
	
	addl	$8, %esp
	popl	%eax		# pop authorization function return value
	testl	%eax, %eax
	jne		.granted_ase		
	
	# system call denied
	MPROTECT_LOCKDOWN(4(%ebp))
	popl %eax	# move fake syscall return value to %eax
	addl $12, %esp
	
	# overwrite tld pointer
	movl	$0, 4(%ebp)
	
	popfl
	leal 8(%esp), %esp	# increase %esp by 8 without affecting any flags
	popl %ebp	# was pushed onto stack by code in vdso
	popl %edx
	popl %ecx
	ret		# jumps to the jump-back trampoline


.granted_ase:
	# readjust stack
	addl	$4, %esp
	
	MPROTECT_LOCKDOWN(4(%ebp))
	
	# overwrite tld pointer
	movl	$0, 4(%ebp)

	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	leal 4(%esp), %esp	# pop tld-pointer from stack
	sysenter
	.size	authorize_sysenter, .-authorize_sysenter



#     +-------------+
#   8 | tld-pointer | (gets removed by ret $4)
#   4 | eip (call)  | 
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         |
#     +-------------+
authorize_int0x80:
	pushl %ebp
	movl %esp, %ebp
	
	pushfl
	pushl %eax
	pushl %ecx
	pushl %edx
	
	MPROTECT_UNLOCK_SAFE(8(%ebp))
	
	# push arguments to system call onto the stack
	# as arguments for authorize_syscall
	pushl $-1	# default for retval if not authorized
	pushl %esp	# pointer to retval
	pushl $0	# syscall not called via sysenter
	pushl (%ebp)	# push original %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	
	# write ind_jump_target for the jump-back trampoline; so that it points
	# to the int 0x80 instruction
	movl	8(%ebp), %ecx
	movl	4(%ebp), %edx
	movl	%edx, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite system call return address with address of ret jump-back tramp
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)
	
	#call authorize_syscall
	call *authorize_syscl_table( , %eax, 4)
	testl %eax, %eax
	jne .authorized_ai8
	
	# system call not authorized, increment tld->ind_jump_target by 2 
	# to jump over int 0x80 instruction
	movl 8(%ebp), %ecx
	addl $2, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# write fake syscall return value to %eax and return
	addl $36, %esp
	MPROTECT_LOCKDOWN(8(%ebp))
	movl	$0, 8(%ebp)		# overwrite tld pointer
	popl	%eax	# fake syscal return value into %eax
	popl	%edx
	popl	%ecx
	addl	$4, %esp	# discard saved %eax on the stack
	popfl
	leave
	ret $4
	
.authorized_ai8:
	addl	$40, %esp
	MPROTECT_LOCKDOWN(8(%ebp))
	movl	$0, 8(%ebp)		# overwrite tld pointer
	popl	%edx
	popl	%ecx
	popl	%eax
	popfl
	leave
	ret $4
	.size	authorize_int0x80, .-authorize_int0x80


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



	
# ends a transaction and jumps back to untranslated code
# (and prints a success message)
#     +-------------+
#   8 | tld-pointer |
#   4 | eip (call)  | 
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | eax         |
# -12 | ecx         |
# -16 | edx         |
#     +-------------+
end_transaction_clean:
	pushl	%ebp
	movl	%esp, %ebp

	pushfl		# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx

#	pushl	$SUCCSTR	# print success message
#	call	printf
#	addl	$4, %esp	# clear stack (printf argument)

	# clean allocated non-persistent memory
	pushl 	8(%ebp)
	call	fbt_free_all
	addl	$4, %esp

	popl	%edx	# pop caller-save and eflags registers
	popl	%ecx
	popl	%eax
	popfl

#if defined(FBT_STATISTIC)
	call	print_statistics
#endif
	
	leave
	ret
	.size	end_transaction_clean, .-end_transaction_clean



# Prints some debug message
# Does not change stack!
print_eip:
	pushl	%ebp
	movl	%esp, %ebp

	pushfl			# push eflags and caller-save registers
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	pushl	%ebx

 	pushl	%ebp
 	pushl	%edx
 	pushl	%ecx
 	pushl	%ebx
	pushl	%eax
# 	pushl	4(%ebp) # realip in translated code
 	pushl	16(%ebp) # rip to be translated
 	pushl	$EIP	# print jmp-src message
 	call	printf
 	addl 	$28, %esp	# readjust stack
	
#	pushl	16(%ebp) # realip in translated code
# 	pushl	%ebx # rip to be translated
#	call	check_opt
#	addl	$8, %esp

	
	popl	%ebx
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
	
	MPROTECT_UNLOCK(8(%ebp))
	
	# translate the target IP if necessary
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_ijs	# ,pt = branch prediction hint: take
	call	translate_noexecute
.translated_ijs:
	# write ind_jump location
	movl	8(%ebp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite return instruction pointer to point to the jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %edx
	movl	%edx, 4(%ebp)

	# re-adjust the stack
	addl	$8, %esp
	
	MPROTECT_LOCKDOWN(8(%ebp))
	
	movl	$0, 8(%ebp)		# overwrite tld pointer
	
	# pop caller-save and eflags registers

	popl	%edx
	popl	%ecx
	popl	%eax
	popfl

	leave
	ret	$8
	.size	ind_jump_signal, .-ind_jump_signal

/*
#     +-------------+
#  12 | symbol      |
#   8 | handle      |
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | ecx         |
# -12 | edx         | <- esp
#     +-------------+
# Replaces the dlsym(3) function in guest code. It calls dlsym with the 
# arguments of the call in the guest code, but then checks if the result is 
# within a section of the memory marked as internal data of the binary 
# translator, such as the memory occupied by libfastbt.so. If this is the case,
# it returns null, otherwise the result of the dlsym call.
*/
dlsym_handler:
	pushl	%ebp
	movl	%esp, %ebp
	# push eflags and caller-save registers
	pushfl
	pushl	%ecx
	pushl	%edx

	# lets get the tld
	pushl	tld_key
	call	pthread_getspecific
	addl	$4, %esp
	
	# write jump target for jump-back trampoline
	# but we need to make sure that it is translated
	
	# translate the return IP if necessary
	pushl	4(%ebp)
	pushl	%eax
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_dsh	# ,pt = branch prediction hint: take
	MPROTECT_UNLOCK((%esp))
	call	translate_noexecute
	MPROTECT_LOCKDOWN_SAFE(4(%esp))
.translated_dsh:
	# write ind_jump location
	movl	(%esp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite eip so we use jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %eax
	movl	%eax, 4(%ebp)
	
	# call dlsym 
	addl	$8, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	dlsym
	
	# call fbt_memprotect_info to find out if a sensitive symbol was looked up
	subl	$SIZEOF_MEM_INFO, %esp		# memory for mem_info struct
	pushl	%esp						# pointer to mem_info struct
	pushl	%eax
	call	fbt_memprotect_info
	testl	%eax, %eax					# check if info was found
	movl	(%esp), %eax	# write dlsym return value to eax
	je		.finish_dsh		# no info was found, we can return the dlsym result

	/* 
	 * Info about the looked up address was found, now check if it is an 
	 * internal data structure of the binary translator.
	 */
	leal	8(%esp), %ecx
	addl	$OFFSETOF_MEM_INFO_FLAGS, %ecx
	mov		(%ecx), %cl
	test	$INFO_BTFLAG, %cl
	je		.finish_dsh		# if BTFLAG is not set, we return the dlsym result
	
	/* BTFLAG is set, we have to return NULL and set errno */
	movl	$0, %eax
	# movl	$-1, errno
	

.finish_dsh:
	addl	$16, %esp
	addl	$SIZEOF_MEM_INFO, %esp
	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popfl
	leave
	ret
	.size	dlsym_handler, .-dlsym_handler


/*
#     +-------------+
#  16 | version     |
#  12 | symbol      |
#   8 | handle      |
#   4 | eip (call)  | (gets patched)
#   0 | old ebp     | <- ebp
#  -4 | flags       |
#  -8 | ecx         |
# -12 | edx         | <- esp
#     +-------------+
# same as dlsym_handler, but this one handles the nearly identical function
# dlvsym
*/
dlvsym_handler:
	pushl	%ebp
	movl	%esp, %ebp
	# push eflags and caller-save registers
	pushfl
	pushl	%ecx
	pushl	%edx

	# lets get the tld
	pushl	tld_key
	call	pthread_getspecific
	addl	$4, %esp
	
	# write jump target for jump-back trampoline
	# but we need to make sure that it is translated
	
	# translate the return IP if necessary
	pushl	4(%ebp)
	pushl	%eax
	call	TCACHE_FIND
	testl	%eax, %eax
	jne,pt .translated_dvh	# ,pt = branch prediction hint: take
	call	translate_noexecute
.translated_dvh:
	# write ind_jump location
	movl	(%esp), %ecx
	movl	%eax, OFFSETOF_TLD_IND_JUMP_TARGET(%ecx)
	
	# overwrite eip so we use jump-back trampoline
	movl	OFFSETOF_TLD_RET_JUMPBACK_TRAMP(%ecx), %eax
	movl	%eax, 4(%ebp)
	
	# call dlvsym 
	addl	$8, %esp
	pushl	16(%ebp)
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	dlvsym
	
	# call fbt_memprotect_info to find out if a sensitive symbol was looked up
	subl	$SIZEOF_MEM_INFO, %esp		# memory for mem_info struct
	pushl	%esp						# pointer to mem_info struct
	pushl	%eax
	call	fbt_memprotect_info
	testl	%eax, %eax					# check if info was found
	movl	(%esp), %eax	# write dlsym return value to eax
	je		.finish_dvh		# no info was found, we can return the dlsym result

	/* 
	 * Info about the looked up address was found, now check if it is an 
	 * internal data structure of the binary translator.
	 */
	leal	8(%esp), %ecx
	addl	$OFFSETOF_MEM_INFO_FLAGS, %ecx
	mov		(%ecx), %cl
	test	$INFO_BTFLAG, %cl
	je		.finish_dvh		# if BTFLAG is not set, we return the dlvsym result
	
	/* BTFLAG is set, we have to return NULL and set errno */
	movl	$0, %eax
	# movl	$-1, errno

.finish_dvh:
	addl	$20, %esp
	addl	$SIZEOF_MEM_INFO, %esp
	# pop caller-save and eflags registers
	popl	%edx
	popl	%ecx
	popfl
	leave
	ret
	.size	dlvsym_handler, .-dlvsym_handler


/* returns from a call to dl_iterate_phdr while doing nothing */
fbt_do_nothing:
	pushl	%ebp
	movl	%esp, %ebp
	
	# lets get the tld
	pushl	tld_key
	call	pthread_getspecific
	addl	$4, %esp
	
	leave
	pushl	%eax
	movl	$0, %eax
	call	ind_jump
	.size	fbt_do_nothing, .-fbt_do_nothing
	
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
#	call	print_eip
	sub	$0x4, %esp
#if defined(SLEEP_ON_INIT)
	movl	$0x2, (%esp)	# push parameter (2)
	call	sleep		# sleep(2);
#endif
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
#ifdef SLEEP_ON_FINI
	pushl	$10
	call	sleep
	addl	$4, %esp
#endif /* SLEEP_ON_FINI */
#ifdef TRUST_CLEANUP_ON_FINI
	call	fbt_cleanup
#endif /* TRUST_CLEANUP_ON_FINI */
	popa
	popf

#endif // hijack control
#endif // ld preload fun
