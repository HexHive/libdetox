#include <stdio.h>

void main() {

/*
asm(
"xor %ebx, %ebx;"
"movl %ebx, %eax;"
"movb $0x8b,%bl;"
"inc %eax;"
"int $0x80;"
);
*/

/*
asm(
"jmp end;"
"begin:"
"popl %ecx;" // pop address of string
"xor %edx, %edx;" // message length
"movb $0x7, %dl;"
"xor %ebx, %ebx;"
"movl %ebx, %eax;"
"movb $0x4,%al;" // syscall 4 = write
"inc %ebx;" // file descriptor stdout
"int $0x80;"
"xor %ebx, %ebx;"
"movl %ebx, %eax;"
"inc %eax;"
"int $0x80;"
"end:"
"call begin;"
".string \"0wn3d!\n\";"
);
*/

/*
printf("setup\n");

asm(
"jmp end;"
"begin:"
//"call puts;"
//"nop;"
//"call 0x80482d4;"
"movl $0x80482d4, %eax;"
"call %eax;"
"xor %ebx, %ebx;"
"movl %ebx, %eax;"
"inc %eax;"
"int $0x80;"
"end:"
"call begin;"
".string \"0wn3d libc style!\";"
);
*/

/* system("/bin/bash");*/

}