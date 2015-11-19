#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
 char *hint;
 int pid, status, retcode;
 int i, j;

 int *shared_mem=mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1,0);
 int *loop = shared_mem; // pointer to shared memory location of hint

 *loop = 1;

 //posix_memalign((void*)&hint, sysconf(_SC_PAGESIZE), sysconf(_SC_PAGESIZE));
 //hint = malloc(1);
 //posix_memalign((void*)&hint, 1, sysconf(_SC_PAGESIZE));
 //printf("*hint = %x\n", hint);

for(i = 0; i < 5; i++)
{
 *loop = 1;

 pid = fork(); // fork process
 
 if (pid < 0) { /* error occurred */
	printf("fork() failed\n");
	return 1;
 }
 else if (pid==0) { /* child process */
  //sleep(10);
  while(*loop);
  return 0;
 }
 else { /* parent process */
	printf("pid of child: %d\n", pid);
	sleep(10);
	*loop = 0;
	wait(&status);
 }
}

 return 0;
}
