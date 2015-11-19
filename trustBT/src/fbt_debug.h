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

/**
 * The file names for the output
 */
#define DEBUG_FILE_NAME					"debug.txt"
#define CODE_DUMP_FILE_NAME				"code_dump.txt"
#define JMP_TABLE_DUMP_FILE_NAME		"jmpTable_dump.txt"


#include "fbt_debug_impl.h"
#include "fbt_llio.h"

char* printnbytes(unsigned char *addr, unsigned int n);

/**
 * If there is any debug output enabled then ERROR is defined.
 * If not there will never be any debug output...
 */
#ifdef ERROR
	#define DEBUG_START							DEBUG_START_IMPL
	#define DEBUG_END							DEBUG_END_IMPL
#else
	#define DEBUG_START
	#define DEBUG_END
#endif



/**
 * The definition of the functions which should be used in the application
 * The implementation and the nasty stuff is in debug_impl.h
 */
#ifdef ERROR
	#define PRINT_ERROR(...)						PRINT_N_IMPL(ERROR_TYPE, 1, __VA_ARGS__)
	#define PRINT_ERROR_N(N, ...)					PRINT_N_IMPL(ERROR_TYPE, N, __VA_ARGS__)
	#define PRINT_ERROR_FUNCTION_START(...)			PRINT_FUNCTION_START_IMPL(ERROR_TYPE, __VA_ARGS__)
	#define PRINT_ERROR_FUNCTION_END(...)			PRINT_FUNCTION_END_IMPL(ERROR_TYPE, __VA_ARGS__)
#else
	#define PRINT_ERROR(...)
	#define PRINT_ERROR_N(N, ...)
	#define PRINT_ERROR_FUNCTION_START(...)
	#define PRINT_ERROR_FUNCTION_END(...)
#endif

#ifdef WARNING
	#define PRINT_WARNING(...)						PRINT_N_IMPL(WARNING_TYPE, 1, __VA_ARGS__)
	#define PRINT_WARNING_N(N, ...)					PRINT_N_IMPL(WARNING_TYPE, N, __VA_ARGS__)
	#define PRINT_WARNING_FUNCTION_START(...)		PRINT_FUNCTION_START_IMPL(WARNING_TYPE, __VA_ARGS__)
	#define PRINT_WARNING_FUNCTION_END(...)			PRINT_FUNCTION_END_IMPL(WARNING_TYPE, __VA_ARGS__)
#else
	#define PRINT_WARNING(...)
	#define PRINT_WARNING_N(N, ...)
	#define PRINT_WARNING_FUNCTION_START(...)
	#define PRINT_WARNING_FUNCTION_END(...)
#endif

#ifdef INFO
	#define PRINT_INFO(...)							PRINT_N_IMPL(INFO_TYPE, 1, __VA_ARGS__)
	#define PRINT_INFO_N(N, ...)					PRINT_N_IMPL(INFO_TYPE, N, __VA_ARGS__)
	#define PRINT_INFO_FUNCTION_START(...)			PRINT_FUNCTION_START_IMPL(INFO_TYPE, __VA_ARGS__)
	#define PRINT_INFO_FUNCTION_END(...)			PRINT_FUNCTION_END_IMPL(INFO_TYPE, __VA_ARGS__)
#else
	#define PRINT_INFO(...)
	#define PRINT_INFO_N(N, ...)
	#define PRINT_INFO_FUNCTION_START(...)
	#define PRINT_INFO_FUNCTION_END(...)
#endif

#ifdef DEBUG
	#define PRINT_DEBUG(...)						PRINT_N_IMPL(DEBUG_TYPE, 1, __VA_ARGS__)
	#define PRINT_DEBUG_N(N, ...)					PRINT_N_IMPL(DEBUG_TYPE, N, __VA_ARGS__)
	#define PRINT_DEBUG_FUNCTION_START(...)			PRINT_FUNCTION_START_IMPL(DEBUG_TYPE, __VA_ARGS__)
	#define PRINT_DEBUG_FUNCTION_END(...)			PRINT_FUNCTION_END_IMPL(DEBUG_TYPE, __VA_ARGS__)
#else
	#define PRINT_DEBUG(...)
	#define PRINT_DEBUG_N(N, ...)
	#define PRINT_DEBUG_FUNCTION_START(...)
	#define PRINT_DEBUG_FUNCTION_END(...)
#endif

#ifdef INFO_OUTPUT
    #define INFO_PRINTF(...)            printf(__VA_ARGS__)
    #define INFO_LLPRINT(str)           llprintf(str)
    #define INFO_LLPRINTF(...)   fllprintf(STDERR_FILENO, __VA_ARGS__)
#else
    #define INFO_PRINTF(...)
    #define INFO_LLPRINT(str)
    #define INFO_LLPRINTF(...)
#endif

#ifdef INFO_OUTPUT_VERBOSE
    #define INFO_LLPRINTF_VERBOSE(...)   fllprintf(STDERR_FILENO, __VA_ARGS__)
#else
    #define INFO_LLPRINTF_VERBOSE(...)
#endif
    

#ifdef DUMP_GENERATED_CODE
	#define DUMP_START											DUMP_START_IMPL
	#define DUMP_END											DUMP_END_IMPL
	#define DUMP_CODE(addr, len)						DUMP_CODE_IMPL(0, 0, addr, len)

	#define DUMP_CODE_BOTH(org_addr, org_len, transl_addr, transl_len)    	DUMP_CODE_IMPL(org_addr, org_len, transl_addr, transl_len)
	#define DUMP_JMP_TABLE_ENTRY(org_addr, transl_addr)			DUMP_JMP_TABLE_ENTRY_IMPL(org_addr, transl_addr)
#else
	#define DUMP_START
	#define DUMP_END
	#define DUMP_CODE(addr, len)
	#define DUMP_CODE_BOTH(org_addr, org_len, transl_addr, transl_len)
	#define DUMP_JMP_TABLE_ENTRY(org_addr, transl_addr)
#endif

#ifdef PBREAKGDB
	#define BREAK_ON_TRANSL(org_addr, transl_addr) BREAK_ON_TRANSL_IMPL(org_addr, transl_addr)
#else
	#define BREAK_ON_TRANSL(org_addr, addr)
#endif
