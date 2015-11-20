#include <stdio.h>
#include <time.h>

/* This is a simple R_386_IRELATIVE test. It uses ctime which will cause the libc to use strnlen somewhre within libc. This requires one of the R_386_IRELATIVE relocations to be handled correctly. */
int main ()
{
  int i;
  char *ts,time_string[114];
  time_t now;

  now=time(NULL);
  ts=ctime(&now);
  for (i=0; ts[i]>=' '; i++) time_string[i]=ts[i];
  time_string[i]='\0';
  printf("%s\n",time_string);

return 0;

}

