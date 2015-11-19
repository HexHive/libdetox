#ifndef FBT_LLIO_H
#define FBT_LLIO_H

#include <stdarg.h>
#define STDOUT_FILENO 1

/* low-level I/O and memory allocation functions */

/* print a string to the file descriptor fd */
int fllwrite(int fd, const char* str);

/** macro to use fllprint with stdout as file descriptor */
#define llprint(str) fllprintf(STDOUT_FILENO, str)


/* write a formatted string to the file descriptor fd (might use a buffer) */
int fllprintf(int fd, const char *format, ...);
int fllprintfva(int fd, const char* format, va_list ap);
/* flush printf buffers */
void ffflush();

/** macro to use fllprintf with stdout as file descriptor */
#define llprintf(...) fllprintf(STDOUT_FILENO, __VA_ARGS__)

/** macro to allocate one page of memory */
//#define allocpage() mmap(NULL, PAGESIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

/** macro to free a page of memory that was allocated with allocpage() */
//#define freepage(addr) munmap(addr, PAGESIZE)



#endif /* FBT_LLIO_H */
