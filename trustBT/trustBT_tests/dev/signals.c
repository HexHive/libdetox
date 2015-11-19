#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void clean_exit_on_sig(int sig_num)
{
 printf("********************* program signal handler *********************\n");
 printf ("Signal %d received\n", sig_num);
}

int main()
{
 int a;

 signal(SIGSEGV, clean_exit_on_sig);
 signal(SIGSEGV, clean_exit_on_sig);
 a = *((int*)0x00000000);

 return 0;
}