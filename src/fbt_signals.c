/**
 * This handles all signals and adds some special catch-all signals
 *
 * Copyright (c) 2008 ETH Zurich
 *   Mathias Payer <mathias.payer@inf.ethz.ch>
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
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "libfastbt.h"
#include "fbt_debug.h"
#include "fbt_private_datatypes.h"
#include "fbt_trampoline.h"
#include "fbt_signals.h"
#include "fbt_asm_functions.h"
#include "fbt_tcache.h"
#include "fbt_translate.h"
#include "fbt_mem_protection.h"
#include "fbt_mem_alloc.h"

#if defined(RTLD_NEXT)
#define REAL_LIBC RTLD_NEXT
#else
#define REAL_LIBC ((void *) -1L)
#endif

#ifdef SECU_MPROTECT_IDS
#define FIX_BACK2TRANSLATED   /* get thread local data */ \
  thread_local_data_t *tld = get_tld();\
  fbt_ids_unlock(tld); \
  /* find out return instruction pointer (=beginning of TU)*/\
  void *tu_begin = read_rip();\
  /* check if this code has already been translated */\
  void *ccf_begin = tcache_find(tld, tu_begin);\
  /* if not then do it now */\
  if (ccf_begin == NULL) {\
    /* translate the TU if not already in tcache */\
    ccf_begin = translate_noexecute(tld, tu_begin);\
  }\
  /* overwrite return instruction pointer with address of first instruction in CCF */\
  change_rip(ccf_begin);\
  /* avoid tail-call optimization (-foptimize-sibling-calls) for change_rip() */\
  __asm__ volatile("");
#else
#define FIX_BACK2TRANSLATED   /* get thread local data */ \
  thread_local_data_t *tld = get_tld();\
  /* find out return instruction pointer (=beginning of TU)*/\
  void *tu_begin = read_rip();\
  /* check if this code has already been translated */\
  void *ccf_begin = tcache_find(tld, tu_begin);\
  /* if not then do it now */\
  if (ccf_begin == NULL) {\
    /* translate the TU if not already in tcache */\
    ccf_begin = translate_noexecute(tld, tu_begin);\
  }\
  /* overwrite return instruction pointer with address of first instruction in CCF */\
  change_rip(ccf_begin);\
  /* avoid tail-call optimization (-foptimize-sibling-calls) for change_rip() */\
  __asm__ volatile("");
#endif /* SECU_MPROTECT_IDS */


#if defined(LDPRELOAD)
/* special shared library hacks to replace some nasty functions */
static void seghandler (unsigned int sn, siginfo_t  si, struct ucontext *sc);

void (*(*o_signal) ( int signum, void (*handler)(int)))(int)=0;
int (*o_sigaction) ( int signum, const struct sigaction *act, struct sigaction *oldact)=0;
int (*o_pthread_create)(pthread_t *__restrict __newthread,
				 __const pthread_attr_t *__restrict __attr,
				 void *(*__start_routine) (void *),
				 void *__restrict __arg)=0;
void (*o_nptl_deallocate_tsd)(void)=0;

void initSignals()
{
  struct sigaction m;
  memset(&m, 0, sizeof(struct sigaction));
  m.sa_flags = SA_SIGINFO;
  m.sa_sigaction = (void *)seghandler;
  /* let us call the real sigaction function */
  o_sigaction = (int(*)(int signum, const struct sigaction *act, struct sigaction *oldact)) dlsym(REAL_LIBC, "sigaction");
  o_sigaction (SIGILL,&m,(struct sigaction *)NULL);
  o_sigaction (SIGSEGV,&m,(struct sigaction *)NULL);
  o_sigaction (SIGBUS,&m,(struct sigaction *)NULL);

  o_signal = (void (*(*) ( int signum, void (*handler)(int)))(int)) dlsym(REAL_LIBC, "signal");

  o_pthread_create =  (int (*)(pthread_t *__restrict __newthread,
					       __const pthread_attr_t *__restrict __attr,
					       void *(*__start_routine) (void *),
					       void *__restrict __arg)) dlsym(REAL_LIBC, "pthread_create");

  o_nptl_deallocate_tsd = (void (*)(void))dlsym(REAL_LIBC, "__nptl_deallocate_tsd");
  PRINT_DEBUG("Installed special SIGSEGV handler");
}

static void seghandler(unsigned int sn , siginfo_t  si , struct ucontext *sc)
{
  int *gregs = sc->uc_mcontext.gregs;

  printf("ERROR: Tried to access unmapped memory. secuBT will exit now\n");

  printf("I'll try to show some debug output (might catch another segv):\n");
  printf(" Signal number = %d (%d), status = %d\n", si.si_signo, sn, si.si_status);
  switch(si.si_code) {
    case 1: printf(" SI code = %d (Address not mapped to object)\n", si.si_code);
      break;
    case 2: printf(" SI code = %d (Invalid permissions for mapped object)\n",si.si_code);
      break;
    default: printf(" SI code = %d (Unknown SI Code)\n",si.si_code);
      break;
  }
  printf(" Intruction pointer = 0x%08x \n", gregs[REG_EIP]);
  printf(" Fault addr = 0x%p \n",si.si_addr);
  printf(" trap = 0x%08x \n", gregs[REG_TRAPNO]);
  printf(" err  = 0x%08x \n", gregs[REG_ERR]);
  printf("*** Registers ***\n");
  printf("    EAX: 0x%08x\n", gregs[REG_EAX]);
  printf("    EBX: 0x%08x\n", gregs[REG_EBX]);
  printf("    ECX: 0x%08x\n", gregs[REG_ECX]);
  printf("    EDX: 0x%08x\n", gregs[REG_EDX]);
  printf("    EDI: 0x%08x\n", gregs[REG_EDI]);
  printf("    ESI: 0x%08x\n", gregs[REG_ESI]);
  printf("    EBP: 0x%08x\n", gregs[REG_EBP]);
  printf("    ESP: 0x%08x\n", gregs[REG_ESP]);
  printf("     SS: 0x%08x\n", gregs[REG_SS]);
  printf(" EFLAGS: 0x%08x\n", gregs[REG_EFL]);
#if defined(SLEEP_ON_FAIL)
  printf("Something bad happened, you might want to attach a debugger now!\n");
  sleep(5);
#endif
  _exit(139);
}

typedef void (*sighandler_t)(int);
sighandler_t fbt_signal(int signum, sighandler_t handler)
{
    FIX_BACK2TRANSLATED

    PRINT_DEBUG("signal(%d)\n", signum);
    //printf("signal(%d)\n", signum);
    if (signum==SIGSEGV || signum==SIGBUS || signum==SIGILL) {
        PRINT_DEBUG("Blocked SIGSEGV replacement\n");
        FBT_IDS_LOCKDOWN(tld);
        return 0;
    } else {
        /* Escape to static trampoline for signal handling */
        sighandler_t result;
        if (handler!=NULL) {
            PRINT_DEBUG("Handler: %p\n", handler);
            result = (*o_signal)(signum, trampoline_put_signal(handler));
        } else {
            result = (*o_signal)(signum, handler);
        }
        FBT_IDS_LOCKDOWN(tld);
        return result;
    }
}

int fbt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    FIX_BACK2TRANSLATED

    PRINT_DEBUG("sigaction(%d)\n", signum);
    //printf("sigaction(%d)\n", signum);

    int result = 0;

    if (signum==SIGSEGV) {
        PRINT_DEBUG("Blocked SIGSEGV replacement\n");
        struct sigaction m;
        memset(&m, 0, sizeof(struct sigaction));
        m.sa_flags = SA_SIGINFO;
        m.sa_sigaction = (void *)seghandler;
        result = (*o_sigaction)(SIGSEGV,&m,(struct sigaction *)NULL);
    } else {
        /* overwrite call to sigaction with our own trampoline for signal handling */
        if (act!=NULL) {
            if (act->sa_handler!=NULL) {
                void *myhandler = act->sa_handler;
                PRINT_DEBUG("Handler: %p\n", act->sa_handler);
                myhandler = trampoline_put_signal(act->sa_handler);
            } else if (act->sa_sigaction!=NULL) {
                void *myhandler = act->sa_sigaction;
                PRINT_DEBUG("Handler: %p\n", act->sa_sigaction);
                myhandler = trampoline_put_signal(act->sa_sigaction);
            }
        }
        result = (*o_sigaction)(signum, act, oldact);
    }
    FBT_IDS_LOCKDOWN(tld);
    return result;
}

int fbt_pthread_create(pthread_t *__restrict __newthread,
			   __const pthread_attr_t *__restrict __attr,
			   void *(*__start_routine) (void *),
		    void *__restrict __arg)
{
  FIX_BACK2TRANSLATED
  INFO_PRINTF("pthread_create inside transaction - using fixup! (in fbt_signals.c)\n");
  FBT_IDS_LOCKDOWN(tld);

  /* allocate trampoline on a page; set as executable and readonly later on */
  char *pthread_trampoline = (char*) allocpages(1);
  fbt_memprotect_add_valid((void*) (pthread_trampoline + 24), 36);

  /* push	$0x0	        # push parameter (0)
     call	fbt_init	# fbt_init(NULL);
     movl	$fbt_commit_transaction, (%esp)	# push function ptr
     call	fbt_start_transaction		# fbt_start_transaction(..)
     add	$0x4, %esp
     call       __start_routine
     call       fbt_commit_transaction
  */
  char *tramptr = pthread_trampoline;

  *tramptr++ = 0x9c;  // pushfl

  *tramptr++ = 0x60;  // pusha

  *tramptr++ = 0x68;  // push 0x00
  *((int32_t*) tramptr) = 0x0;
  tramptr+=4;

  *tramptr++ = 0xE8;		/* 0xE8 = call instruction */
  *((int32_t*) tramptr) = (int32_t) &fbt_init - (int32_t) tramptr - 4;
  tramptr+=4;

  *tramptr++ = 0xc7;  // mov r/m32, imm32
  *tramptr++ = 0x04;
  *tramptr++ = 0x24;  // (%esp)
  *((int32_t*) tramptr) = (int32_t) &fbt_commit_transaction;
  tramptr+=4;

  *tramptr++ = 0xE8;		/* 0xE8 = call instruction */
  *((int32_t*) tramptr) = (int32_t) &fbt_start_transaction - (int32_t) tramptr - 4;
  tramptr+=4;

  *tramptr++ = 0x83;  // add r/m8, imm8
  *tramptr++ = 0xc4;
  *tramptr++ = 0x04;

  *tramptr++ = 0x61;  // popa

  *tramptr++ = 0x9d;  // popfl

  *tramptr++ = 0xE9;		/* 0xE9 = JMP instruction */
  *((int32_t*) tramptr) = (int32_t) __start_routine - (int32_t) tramptr - 4;
  tramptr+=4;

  /* WE NEVER GET BACK TO HERE, BUT WE CANNOT USE A CALL ABOVE */
  /* TODO: Figure out a way to reclaim these bytes! (we can never free this trampoline) */
  *tramptr++ = 0xE8;		/* 0xE8 = call instruction */
  *((int32_t*) tramptr) = (int32_t) &fbt_commit_transaction - (int32_t) tramptr - 4;
  tramptr+=4;

  *tramptr++ = 0xc3;  // ret

  INFO_PRINTF("Created trampoline: %p (orig start: %p)\n", pthread_trampoline, __start_routine);

  // set memory protection of trampoline
  mprotect(pthread_trampoline, sysconf(_SC_PAGESIZE), PROT_READ | PROT_EXEC);

  int ret = o_pthread_create(__newthread, __attr, (void *(*) (void *))pthread_trampoline, __arg);

  return ret;
}

/**
 * overwrite dlclose so that a libraries can never be unmapped.
 * If a library gets unmapped and a new library gets mapped to the same place then
 * we might execute stale code that is still in the code cache.
 * An other option to forbidding the dlclose is to flush the code cache and the hashtable.
 * This could be future work
 */
int dlclose(void *handle)
{
  INFO_PRINTF("fbt_signals.c: dlclose intercepted!\n");
  return 0;
}

/**
 * The following functions might be unsafe inside transactions!
 */
/*
#include <sys/types.h>
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence)
{
  printf("XXX lseek (%d)called\n", fd);
  static off_t (*o_lseek)(int fd, off_t offset, int whence)=0;
  o_lseek = (off_t (*)(int fd, off_t offset, int whence)) dlsym(REAL_LIBC, "lseek");
  return o_lseek(fd, offset, whence);
}

int dup(int oldfd)
{
  printf("XXX dup (%d) called\n", oldfd);
  static int (*o_dup)(int oldfd)=0;
  o_dup = (int (*)(int oldfd)) dlsym(REAL_LIBC, "dup");
  return o_dup(oldfd);
}

int dup2(int oldfd, int newfd)
{
  printf("XXX dup2 (%d, %d) called\n", oldfd, newfd);
  static int (*o_dup2)(int oldfd, int newfd)=0;
  o_dup2 = (int (*)(int oldfd, int newfd)) dlsym(REAL_LIBC, "dup2");
  return o_dup2(oldfd, newfd);
}

void pthread_exit(void *retval)
{
  printf("exit %p\n", retval);
}
int pthread_cancel(pthread_t thread) {
  printf("cancel request\n");
}

void __nptl_deallocate_tsd (void) {
  printf("Deallocating my local storage - let's better bail out!\n");
  fbt_commit_transaction();
  o_nptl_deallocate_tsd();
}
*/

#endif
