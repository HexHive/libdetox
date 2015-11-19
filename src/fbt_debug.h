/**
 * This handles the debug output that can be customized in the Makefile
 *
 * Note: stdio is thread-safe
 *
 * IMPORTANT:
 * - The macro START_DEBUG and START_DUMP must be called before any
 *   call to PRINT_* or DUMP_* respectively, otherwise the program
 *   will abort.
 * - The macro STOP_DEBUG and STOP_DUMP clean up the data structure
 *   after calling one of these macros the corresponding debugging
 *   features must not be called anymore otherwise the program will
 *   abort.
 *
 * WARNING: concerns DEBUG_FUNCTION_{START,END} and DEBUG_PRINT_N
 * - Everything_ written on the same line after one of the above
 *   macros will be ignored!
 * - output is written to file (i.e. debug.txt)
 * - macros are thread safe
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
 *   Marcel Wirth <mawirth@student.ethz.ch>
 *   Stephan Classen <scl@soft-eng.ch>
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

#include "fbt_datatypes.h"
#include "fbt_llio.h"

#ifdef DEBUG

#include <assert.h>

/* use debug */
void debug_start();
void debug_end();
void debug_print_function_start(char *str, ...);
void debug_print_function_end(char *str, ...);
void debug_print_n(int N, char *str, ...);

#define DEBUG_START debug_start()
#define DEBUG_END   debug_end()
#define PRINT_DEBUG(...) debug_print_n(1, __VA_ARGS__)
#define PRINT_DEBUG_N(N, ...) debug_print_n(N, __VA_ARGS__)
#define PRINT_DEBUG_FUNCTION_START(...) debug_print_function_start(__VA_ARGS__)
#define PRINT_DEBUG_FUNCTION_END(...) debug_print_function_end(__VA_ARGS__)
#else
/* no debug */
#define assert(...)
#define DEBUG_START
#define DEBUG_END
#define PRINT_DEBUG(...)
#define PRINT_DEBUG_N(N, ...)
#define PRINT_DEBUG_FUNCTION_START(...)
#define PRINT_DEBUG_FUNCTION_END(...)
#endif

/* todo remove */
char* printnbytes(unsigned char *addr, unsigned int n);

#ifdef DUMP_GENERATED_CODE
/* dump generated code */
void debug_dump_start();
void debug_dump_end();
void debug_dump_code(struct translate_struct *ts, int instr_len, int transl_len);
void debug_dump_jmptable(char *orig_addr, char *transl_addr);
#define DUMP_START debug_dump_start()
#define DUMP_END debug_dump_end()
#define DUMP_CODE(ts, instr_len, transl_len) debug_dump_code(ts, instr_len, transl_len)
#define DUMP_JMP_TABLE_ENTRY(org_addr, transl_addr) debug_dump_jmptable(org_addr, transl_addr)
#else
/* do not dump generated code */
#define DUMP_START
#define DUMP_END
#define DUMP_CODE(ts, instr_len, transl_len)
#define DUMP_JMP_TABLE_ENTRY(org_addr, transl_addr)
#endif

#ifdef PBREAKGDB
#define BREAK_ON_TRANSL(org_addr, transl_addr)		\
do {							\
    if(break_on_transl_addr == (org_addr))		\
	break_on_transl(org_addr, transl_addr);		\
 } while (0)
/*
 * Useful in gdb to break on translation of a certain address:
 * break break_on_transl
 * p pbreak(address)
 */
extern void *break_on_transl_addr;
extern void pbreak(void *break_addr);
extern void break_on_transl(void *org_addr, void *transl_addr);
#else

#define BREAK_ON_TRANSL(org_addr, addr)
#endif
