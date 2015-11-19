#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
 int pid, status;
 int i;


 pid = fork(); // fork process

 if (pid < 0) { /* error occurred */
	printf("fork() failed\n");
	return 1;
 }
 else if (pid==0) { /* child process */
  i = *((int*)0x00000000);
  return 0;
 }
 else { /* parent process */
	printf("pid of child: %d\n", pid);
	wait(&status);
	printf("child returned %d\n", status);
 }

 return 0;
}