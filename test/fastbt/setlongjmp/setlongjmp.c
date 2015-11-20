#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

void someFunc();
void jmpfunction(jmp_buf);

jmp_buf env_buffer;

int main()
{
   int val;

   /* save calling environment for longjmp */
   printf("calling setjmp...\n");
   val = setjmp(env_buffer);
   if(val != 0) 
   {
      printf("returned from a longjmp() with value = %d\n", val);
      exit(0);
   }

   printf("calling someFunc()...\n");
   someFunc(); 
   
   return(0);
}

void someFunc() {

 int j = 0;

 printf("someFunc() called\n");

 jmpfunction(env_buffer); 

 j++;
 printf("returning from someFunc() (should not happen)\n");

 return;
}

void jmpfunction(jmp_buf env_buf)
{
   int i = 0;
   printf("calling longjmp with value %d...\n", 1);
   longjmp(env_buf, 1);
   i++;
   return;
}

