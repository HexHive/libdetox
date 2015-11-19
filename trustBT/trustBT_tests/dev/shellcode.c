#include <stdio.h>
#include <unistd.h>

/* gcc -o shellcode -ggdb -static shellcode.c */

void main() {

/*
   char *name[2];
   name[0] = "/bin/sh";
   name[1] = NULL;
   execve(name[0], name, NULL);
*/

/*
   exit(1);
*/

/*
 char msg[] = "0wn3d!\n";
 write(1, msg, 7);
*/

 printf("0wn3d libc style!\n");

}
