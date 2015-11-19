/**
 * Implementation of the debug output
 *
 * Note: stdio is thread-safe
 *
 * WARNING: concerns DEBUG_FUNCTION_{START,END} and DEBUG_PRINT_N
 * - _everything_ written on the same line after one of the above
 *   macros will be ignored!
 * - output is written to file (i.e. debug.txt)
 * - function are thread safe
 */

/**
 * Ensure that all higher output levels are also defined
 */
#ifdef DEBUG
	#ifndef INFO
		#define INFO
	#endif
	#ifndef MIN_DEBUG_LEVEL
		#define MIN_DEBUG_LEVEL "DEBUG"
	#endif
#endif

#ifdef INFO
	#ifndef WARNING
		#define WARNING
	#endif
	#ifndef MIN_DEBUG_LEVEL
		#define MIN_DEBUG_LEVEL "INFO"
	#endif
#endif

#ifdef WARNING
	#ifndef ERROR
		#define ERROR
	#endif
	#ifndef MIN_DEBUG_LEVEL
		#define MIN_DEBUG_LEVEL "WARNING"
	#endif
#endif

#ifndef MIN_DEBUG_LEVEL
	#define MIN_DEBUG_LEVEL "ERROR"
#endif


/**
 * the includes
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "libdisasm/libdis.h"




/**
 * The global vars needed for debuging
 */
#ifdef ERROR
	pthread_mutex_t debug_mutex;
	pthread_key_t thread_debug;
	FILE* debugStream;
#endif
#ifdef DUMP_GENERATED_CODE
	pthread_mutex_t dump_mutex;
	FILE* dumpCodeStream;
	FILE* dumpJmpTableStream;
#endif

/**
 * Some Strings for a neat ouput
 */
#define DEBUG_TYPE			"[ DEBUG   ]"
#define INFO_TYPE			"[ INFO    ]"
#define WARNING_TYPE		"[ WARNING ]"
#define ERROR_TYPE			"[ ERROR   ]"


/**
 * The functions implemented in debug.c
 */
#ifdef DUMP_GENERATED_CODE
	void print_disasm_inst(FILE* f, char *addr, int len);
#endif

/**
 * The implementation.
 * This is independant of the debug level
 */
#define DEBUG_START_IMPL													\
	do {											\
		if (debugStream==NULL) \
		  debugStream = fopen(DEBUG_FILE_NAME,"w");					\
		fprintf(debugStream, "Start debugging on level: %s\n\n", MIN_DEBUG_LEVEL);	\
		pthread_mutex_init(&debug_mutex, NULL);										\
		if (pthread_key_create(&thread_debug, NULL) != 0) {							\
			perror("Error creating thread local\n");							\
			exit(1);									   	\
		}											\
		pthread_setspecific(thread_debug, (void *)0);								\
} while(0);

#define DEBUG_END_IMPL														\
	do {															\
		pthread_mutex_destroy(&debug_mutex);										\
		fprintf(debugStream, "\nStop debugging on level: %s\n", MIN_DEBUG_LEVEL);	\
		fclose(debugStream);												\
		debugStream = (void *) 0;											\
		pthread_key_delete(thread_debug);											\
	} while(0);

//		pthread_t tid = pthread_self();
//		fprintf(debugStream, "%s tid=%x: \t", debugType, (unsigned int)tid);
#define PRINT_FUNCTION_START_IMPL(debugType, ...)									\
	do {																\
		int i;														\
		int n = (int)pthread_getspecific(thread_debug);								\
		n++;															\
		pthread_mutex_lock(&debug_mutex);											\
		for (i = 0; i < n; i++) {											\
			fprintf(debugStream, "\t");										\
		}															\
		fprintf(debugStream, __VA_ARGS__);											\
		fprintf(debugStream, " {\n");												\
		fflush(debugStream);									\
		pthread_mutex_unlock(&debug_mutex);											\
		pthread_setspecific(thread_debug, (void *)n);								\
	} while (0);

//		pthread_t tid = pthread_self();
//		fprintf(debugStream, "%s tid=%x: \t",debugType, (unsigned int)tid);
#define PRINT_FUNCTION_END_IMPL(debugType, ...)										\
	do {															\
		int i;															\
		int n = (int)pthread_getspecific(thread_debug);								\
		pthread_mutex_lock(&debug_mutex);											\
		for (i = 0; i < n; i++) {											\
			fprintf(debugStream, "\t");										\
		}															\
		fprintf(debugStream, "} ");											\
		fprintf(debugStream, __VA_ARGS__);											\
		fprintf(debugStream, "\n");											\
		n--;													\
		if (n == 0) fprintf(debugStream, "\n");										\
		fflush(debugStream);												\
		pthread_mutex_unlock(&debug_mutex);											\
		pthread_setspecific(thread_debug, (void *)n);								\
	} while (0);

//		pthread_t tid = pthread_self();
//		fprintf(debugStream, "%s tid=%x: \t",debugType, (unsigned int)tid);
#define PRINT_N_IMPL(debugType, N, ...)												\
	do {																\
		int i;														\
		int n = (int)pthread_getspecific(thread_debug);								\
		pthread_mutex_lock(&debug_mutex);											\
		for (i = 0; i < N+n; i++) {											\
			fprintf(debugStream, "\t");										\
		}															\
		fprintf(debugStream, __VA_ARGS__);											\
		fprintf(debugStream, "\n");											\
		fflush(debugStream);												\
		pthread_mutex_unlock(&debug_mutex);											\
	} while(0);

#define PRINT_IMPL(debugType,...)		PRINT_N_IMPL(debugType, 1, ...)

#define DUMP_START_IMPL														\
	do {															\
		if (dumpCodeStream == NULL) dumpCodeStream = fopen(CODE_DUMP_FILE_NAME,"w");					\
		if (dumpJmpTableStream == NULL) dumpJmpTableStream = fopen(JMP_TABLE_DUMP_FILE_NAME,"w");			\
		disassemble_init(0, ATT_SYNTAX);											\
		pthread_mutex_init(&dump_mutex, NULL);										\
	} while(0);

#define DUMP_END_IMPL												\
	do {							\
		pthread_mutex_destroy(&dump_mutex);											\
		fclose(dumpCodeStream);											\
		fclose(dumpJmpTableStream);												 \
		disassemble_cleanup();										\
		dumpCodeStream = (void *) 0;												\
		dumpJmpTableStream = (void *) 0;											\
	} while(0);

#define DUMP_CODE_IMPL(org_addr, org_len, transl_addr, transl_len)									\
	do {																\
		pthread_mutex_lock(&dump_mutex);											\
		print_disasm_inst(dumpCodeStream, (char*)org_addr, org_len);						\
		fprintf(dumpCodeStream, "-->\n");									\
		print_disasm_inst(dumpCodeStream, (char*)transl_addr, transl_len);						\
		fprintf(dumpCodeStream, "\n");												\
		fflush(dumpCodeStream);												\
		pthread_mutex_unlock(&dump_mutex);											\
	} while (0);

#define DUMP_JMP_TABLE_ENTRY_IMPL(org_addr, transl_addr)							\
	do {																\
		pthread_mutex_lock(&dump_mutex);											\
		fprintf(dumpJmpTableStream, "%p --> %p\n", (void*)org_addr, (void*)transl_addr);	\
		fflush(dumpJmpTableStream);											\
		pthread_mutex_unlock(&dump_mutex);											\
	} while (0);



#define BREAK_ON_TRANSL_IMPL(org_addr, transl_addr) \
	do {	\
		if(break_on_transl_addr == (org_addr)) \
			break_on_transl(org_addr, transl_addr); \
	} while (0)

#ifdef PBREAKGDB

/*
 * Useful in gdb to break on translation of a certain address:
 * break break_on_transl
 * p pbreak(address)
 */
extern void *break_on_transl_addr;
extern void pbreak(void *break_addr);
extern void break_on_transl(void *org_addr, void *transl_addr);

#endif
