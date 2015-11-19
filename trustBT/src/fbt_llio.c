/**
 * Low-level I/O functions for use in system call authorization and 
 * instrumentation functions. Some functions that trigger system calls, like 
 * malloc (mmap syscall) or printf (write syscall) are not reentrant. 
 * This means that these functions cannot be used in system call authorization 
 * and instrumentation functions, at least not for the corresponding syscalls.
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "fbt_llio.h"

#define MIN(a,b) ((a)>(b)?(b):(a))

/**
 * small buffer size for the low-level printing functions.
 * The output string cannot be longer than BUFSIZE_S -1. Used for decimal and 
 * hexadecimal numbers.
 */
#define BUFSIZE_S 64

/**
 * large buffer size for the low-level printing functions.
 * The output string cannot be longer than BUFSIZE_L -1. Used for formatted 
 * strings.
 */
#define BUFSIZE_L 1024

/**
 * print a string str to file descriptor fd.
 * This function is a replacement for (f)printf with verbatim printing of the 
 * format string, i. e. no conversion specifications. 
 * In effect, this is just a wrapper around the write(2) system call which 
 * handles interrupted writes.
 * @param fd the target file descriptor
 * @param str the string to write
 * @return the number of characters written
 */
int fllprint(int fd, const char* str)
{
    int written = 0;
    int length = strlen(str);
    while (written < length) {
        int retval = write(fd, str, length - written);
        if (retval < 0) {
            // handle errors and interrupted writes
            if (errno != EINTR) {
                break;
            }
        } else {
            written += retval;
        }
    }
    return written;
}

/**
 * write a formatted string to the file descriptor fd.
 */
int fllprintf(int fd, const char* format, ...)
{
    char buf[BUFSIZE_L];
    va_list ap;
    va_start(ap, format);
    
    int bi = 0;     // index in the output string
    int fi = 0;     // index in the format string
    int end = 0;
    while ((bi < (BUFSIZE_L - 1)) && !end) {
        // parse format string and write contents to buffer
        switch (format[fi]) {
            case '%':
                fi++;
                char *s;
                switch(format[fi]) {
                    case 'd':
                        bi += snllprintd(&buf[bi], 
                                          BUFSIZE_L - bi, 
                                          va_arg(ap, int));
                        break;
                    case 'p':
                        bi += snllprintp(&buf[bi], 
                                          BUFSIZE_L - bi, 
                                          va_arg(ap, void*));
                        break;
                    case 's':
                        s = va_arg(ap, char*);
                        int slen = strlen(s);
                        strncpy(&buf[bi], s, MIN(slen, BUFSIZE_L - bi));
                        bi += slen;
                        break;
                    case '\0':
                        // end of format string, break out of while loop
                        buf[bi]=format[fi];
                        end = 1;
                        break;
                    default:
                        /*
                         * we have a % but no valid conversion specifier
                         * -> we copy the '%' and the following character
                         *    verbatim
                         */
                        buf[bi] = '%';
                        bi++;
                        buf[bi] = format[fi];
                        bi++;
                }
                break;
            case '\0':
                // end of format string, break out of while loop
                buf[bi] = '\0';
                end = 1;
                break;
            default:
                buf[bi] = format[fi];
                bi++;
        }
        fi++;
    }
    va_end(ap);
    
    // write buffer to the target device
    return fllprint(fd, buf);
}

/**
 * write decimal number d to a string.
 * This function is a replacement for snprintf with the formatting string "%d".
 * It writes the decimal number d to the dest string. A null byte is appended. 
 * @param dest the target string
 * @param size the maximum number of bytes to write, including the null byte
 * @param d the decimal number to write
 * @return the number of characters written, excluding the trailing null byte
 */
int snllprintd(char *dest, int size, int d)
{
    char buf[BUFSIZE_S];
    int i = BUFSIZE_S - 1;
    buf[BUFSIZE_S - 1] = '\0';
    int abs_d = abs(d);
    while ((abs_d > 0) && (i > 0)) {
        --i;
        buf[i] = (char) (0x30 + abs_d % 10);
        abs_d /= 10;
    }
    
    if (0 == d) {
        --i;
        buf[i] = '0';
    }
    if (d < 0) {
        --i;
        buf[i] = '-';
    }
    int length = MIN(size, BUFSIZE_S - i);
    strncpy(dest, &buf[i], length);
    return length - 1;
}




/**
 * print pointer p as hex number to target string dest.
 * This function is a replacement for snprintf with the formatting string "%p".
 * It writes the hexadecimal number p to the target string dest.
 * This function currently only handles little-endian byte order.
 * @param dest the target string
 * @param size the maximum number of bytes to write, including the null byte
 * @param p the pointer to write in hex format
 * @return the number of characters written, excluding the trailing null byte
 */
int snllprintp(char *dest, int size, void* p)
{
    char buf[BUFSIZE_S];
    int i = BUFSIZE_S - 1;
    buf[BUFSIZE_S - 1] = '\0';
    
    int length = 0;
    char *pointer = (char*) &p; // pointer to the first byte of th pointer p
    for (length = 0; length < sizeof(void*); length++) {
        // less significant nibble
        --i;
        char tmp = pointer[length] & 0x0f;
        if (tmp < 0xa) {
            buf[i] = 0x30 + tmp;
        } else {
            buf[i] = 0x57 + tmp;
        }
        
        // more significant nibble
        --i;
        tmp = (pointer[length] & 0xf0) >> 4;
        if (tmp < 0xa) {
            buf[i] = 0x30 + tmp;
        } else {
            buf[i] = 0x57 + tmp;
        }
    }
    
    // write prefix 0x
    --i;
    buf[i] = 'x';
    --i;
    buf[i] = '0';
    int len = MIN(size, BUFSIZE_S - i);
    strncpy(dest, &buf[i], len);
    return len - 1;
}



