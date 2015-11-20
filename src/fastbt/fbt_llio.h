/**
 * @file fbt_llio.h
 * Interface for low level IO (without any dependencies to libc).
 *
 * Copyright (c) 2011-2015 ETH Zurich
 * 
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 * @author Antonio Barresi <antonio.barresi@inf.ethz.ch>
 *
 * $Date: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedDate: 2011-12-30 14:24:05 +0100 (Fri, 30 Dec 2011) $
 * $LastChangedBy: payerm $
 * $Revision: 1134 $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#ifndef FBT_LLIO_H
#define FBT_LLIO_H

#if defined(DEBUG)
#include <stdarg.h>
#endif

#ifdef ENABLE_UI_MESSAGES
#include "../libs/uilib.h"
#endif

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/**
 * Print a string str to file descriptor fd.
 * This function is a replacement for (f)printf with verbatim printing of the
 * format string, i. e. no conversion specifications.
 * In effect, this is just a wrapper around the write(2) system call which
 * handles interrupted writes.
 * @param fd the target file descriptor
 * @param str the string to write
 * @return the number of characters written
 */
int fllwrite(int fd, const char* str);

/**
 * Write a formatted string to the file descriptor fd (might use a buffer).
 * @param fd File descriptor that is written to.
 * @param format format string that is interpreted (including all varargs).
 * @return Number of bytes written.
 */
int fllprintf(int fd, const char *format, ...);

#if defined(DEBUG)
/**
 * Write a formatted string to the file descriptor fd (might use a buffer). Used
 * only during debugging.
 * @param fd File descriptor that is written to.
 * @param format format string that is interpreted (including all varargs).
 * @param ap List of var args.
 * @return Number of bytes written.
 */
int fllprintfva(int fd, const char* format, va_list ap);
#endif

#ifdef ENABLE_UI_MESSAGES

/** macro to use fllprintf with stdout as file descriptor */
#define llprintf(...) fllprintf(uifd, __VA_ARGS__)
/** macro to use fllprintf with stderr as file descriptor */
#define llprinterr(...) fllprintf(uifd, __VA_ARGS__)

#else /* else ENABLE_UI_MESSAGES */

/** macro to use fllprintf with stdout as file descriptor */
#define llprintf(...) fllprintf(STDOUT_FILENO, __VA_ARGS__)
/** macro to use fllprintf with stderr as file descriptor */
#define llprinterr(...) fllprintf(STDERR_FILENO, __VA_ARGS__)

#endif /* ENABLE_UI_MESSAGES */

#endif  /* FBT_LLIO_H */
