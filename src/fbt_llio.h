#ifndef FBT_LLIO_H
#define FBT_LLIO_H

/* low-level I/O and memory allocation functions */

#include <unistd.h>     /* gives us the STDOUT_FILENO constant */
#include <sys/mman.h>   /* for mmap and munmap */
#include <stdarg.h>

/* print a string to the file descriptor fd */
int fllprint(int fd, const char* str);

/** macro to use fllprint with stdout as file descriptor */
#define llprint(str) fllprint(STDOUT_FILENO, str)


/* write a formatted string to the file descriptor fd */
int fllprintf(int fd, const char* format, ...);

/** macro to use fllprintf with stdout as file descriptor */
#define llprintf(...) fllprintf(STDOUT_FILENO, __VA_ARGS__)

/* write a decimal number to the string dest */
int snllprintd(char *dest, int size, int d);

/* write a pointer as hex number to the string dest */
int snllprintp(char *dest, int size, void *p);


/** macro to allocate one page of memory */
#define allocpage() mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

/** macro to free a page of memory that was allocated with allocpage() */
#define freepage(addr) munmap(addr, sysconf(_SC_PAGESIZE))



#endif /* FBT_LLIO_H */
