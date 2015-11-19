#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void nops()
{
 printf("nops\n");
 asm(
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
  "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
 );
}

void readeip()
{
 int *eip;
 nops();

 asm(
"mov %esp, %eax;"
"sub $0x4, %eax;"
"push (%eax);"
"sub $0x4, %esp;"
// gcc moves address of string to (%esp), sub 4 so pushed value doesn't get overwritten
 );
 printf("eip on stack: 0x%x\n");

 asm(
"call label;"
"label:"
"sub $0x4, %esp;"
// gcc moves address of string to (%esp), sub 4 so pushed value doesn't get overwritten
 );
 printf("call/pop eip: 0x%x\n");

}

int main()
{
 //printf("sleeping...\n");
 //sleep(2);

 readeip();

 //printf("sleeping...\n");
 //sleep(2);

 return 0;
}
