/**
 * @file fbt_sdbg.cpp
 * Datatypes used for communication with selDebug
 *
 * Copyright (c) 2011 ETH Zurich
 * @author Jonas Pfefferle <jonaspf@student.ethz.ch>
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * $Date: 2012-01-18 16:44:48 +0100 (Wed, 18 Jan 2012) $
 * $LastChangedDate: 2012-01-18 16:44:48 +0100 (Wed, 18 Jan 2012) $
 * $LastChangedBy: kravinae $
 * $Revision: 1189 $
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

#include "fbt_sdbg.h"

#include <unistd.h>
#include <asm-generic/fcntl.h>
#include <asm-generic/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <linux/futex.h>
#include <unistd.h>

#include "fbt_llio.h"
#include "fbt_libc.h"
#include "fbt_debug.h"
#include "fbt_syscalls_impl.h"
#include "fbt_mem_mgmt.h"
#include "fbt_asm_macros.h"
#include "fbt_code_cache.h"
#include "libfastbt.h"
#include "fbt_translate.h"
#include "fbt_x86_opcode.h"
#include "sdbg_insert_lea.h"

#ifdef DEBUG
//#define SDBG_CONSOLE_DEBUG_PRINT(...) printf("fbt_sdbg: "); printf(__VA_ARGS__); fflush(stdout);
#define SDBG_CONSOLE_DEBUG_PRINT(...)
#else
#define SDBG_CONSOLE_DEBUG_PRINT(...)
#endif

#ifdef SEL_DEBUG

struct command_t wcount_2(struct watchpoint_t *wp) {
  struct command_t command;
  command.operation = NOP;
  if (wp->times_hit == 2) {
    command.reaction = STOP;
  } else {
    command.reaction = CONTINUE;
  }
  return command;
}

struct command_t count_2(struct breakpoint_t *bp) {
  struct command_t command;
  command.operation = NOP;
  if (bp->times_hit == 2) {
    command.reaction = STOP;
  } else {
    command.reaction = CONTINUE;
  }
  return command;
}

struct command_t count_28(struct breakpoint_t *bp) {
  struct command_t command;
  command.operation = NOP;
  if (bp->times_hit == ((1<<28) + 1)) {
    command.reaction = STOP;
  } else {
    command.reaction = CONTINUE;
  }
  return command;
}

/**
 * Opens a shared memory region
 * @param region name of region
 * @param size size of region
 * @return pointer to start of region
 */
static __attribute__ ((noinline)) void *sdbg_shared_memory(char *region,
                                                           unsigned int size) {
  int fd;

  PRINT_DEBUG("sdbg_shared_memory(): opening %s\n", region);

  fbt_open(region, O_RDWR, S_IWUSR | S_IRUSR, fd,
           "BT failed to open file (sdbg_shared_memory: "
           "fbt_sdbg.c)\n");

  void *shared;
  fbt_mmap(NULL, (((size) + (PAGESIZE-1)) & (~(PAGESIZE-1))),
           PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0, shared,
           "BT failed to allocate memory (sdbg_shared_memory: "
           "fbt_sdbg.c)\n");

  int ret;
  fbt_close(fd, ret,
            "BT failed to close file (sdbg_shared_memory: "
            "fbt_sdbg.c)\n");

  return shared;
}

struct sdbg_thread_local_data *sdbg_init() {
  PRINT_DEBUG_FUNCTION_START("sdbg_init\n");
  char region[256] = "/dev/shm/sdbg-tld";
  int res;
  fbt_getpid(res, "BT failed to get pid (sdbg_init: fbt_sdbg.c)\n");
  fbt_itoa(res, region + fbt_strnlen(region, 256), 10);
  PRINT_DEBUG("open shared memory region (file) %s\n", region);

  struct sdbg_thread_local_data *sdbg = (struct sdbg_thread_local_data *)
      sdbg_shared_memory(region, sizeof(struct sdbg_thread_local_data));

  fbt_strncpy(region + fbt_strnlen(region, 256), "-flag", 5);
  PRINT_DEBUG("open shared memomry region (file) %s\n", region);
  sdbg->flag = (void *) sdbg_shared_memory(region, sizeof(int));

/*  printf("installing signal\n");
  signal(SIGSEGV, sigsegv_handler);
  printf("done installing signal\n"); */

  PRINT_DEBUG_FUNCTION_END(" ");
  return sdbg;
}

#define NO_FUNCTIONS 10
/**
 * Opens a new primitive
 * @param sdbg pointer to selDebug thread local data
 * @param number primitive's unique number
 * @param type primitive type
 * @return pointer to primitive structure
 */
static void *sdbg_new_primitive(struct sdbg_thread_local_data *sdbg,
                         unsigned int number,
                         enum sdbg_primitive type) {
  char region[256] = "/dev/shm/sdbg-";
  fbt_itoa(sdbg->id, region + fbt_strnlen(region, 256), 10);
  if (type == BREAKPOINT) {
    fbt_strncpy(region + fbt_strnlen(region, 256), "-bp", 4);
  } else if (type == WATCHPOINT) {
    fbt_strncpy(region + fbt_strnlen(region, 256), "-wp", 4);
  }
  fbt_itoa(number, region + fbt_strnlen(region, 256), 10);
  void *info = NULL;

  if (type == BREAKPOINT) {
    info = sdbg_shared_memory(region, sizeof(struct breakpoint_t));
    info = (void *) ((char *)info + sdbg->offset);
    static struct {
      char *name;
      struct command_t (*pointer)(struct breakpoint_t *);
    } functions[NO_FUNCTIONS] = {{"count_2", &count_2},
                                 {"count_28", &count_28}};

    struct breakpoint_t *bp = (struct breakpoint_t *)info;
    if (fbt_strnlen(bp->function, MAX_FUNC_NAME) != 0) {
      int i;
      for (i = 0; i < NO_FUNCTIONS; i++) {
        if (functions[i].name != NULL &&
            fbt_strncmp(functions[i].name, bp->function, MAX_FUNC_NAME) == 0) {
          bp->pfunc = functions[i].pointer;
          break;
        }
      }
      if (bp->pfunc == NULL) {
        llprintf("There is no function called %s! "
                 "Ignoring function argument for breakpoint %d\n",
                 bp->function,
                 bp->number);
      }
    }
  } else if (type == WATCHPOINT) {
    info = sdbg_shared_memory(region, sizeof(struct watchpoint_t));
    info = (void *) ((char *)info + sdbg->offset);
    static struct {
      char *name;
      struct command_t (*pointer)(struct watchpoint_t *);
    } functions[NO_FUNCTIONS] = {{"wcount_2", &wcount_2}};

    struct watchpoint_t *wp = (struct watchpoint_t *)info;
    if (fbt_strnlen(wp->function, MAX_FUNC_NAME) != 0) {
      int i;
      for (i = 0; i < NO_FUNCTIONS; i++) {
        if (functions[i].name != NULL &&
            fbt_strncmp(functions[i].name, wp->function, MAX_FUNC_NAME) == 0) {
          wp->pfunc = functions[i].pointer;
          break;
        }
      }
      if (wp->pfunc == NULL) {
        llprintf("There is no function called \"%s\"! "
                 "Ignoring function argument for watchpoint %d\n",
                 wp->function,
                 wp->number);
      }
    }
  }
  return info;
}

/**
 * Updates the registers in selDebug thread local data
 * Reads registers values from secure stack (tld->stack)
 * the order is determined by the order pusha pushes the
 * registers on the stack. However segments registers
 * are read directly.
 * @param tld pointer to thread local data
 * @param eip instruction pointer to update
 */
static void update_registers(struct thread_local_data *tld,
                             long eip,
                             long org_eip) {
  /*int q;
  for (q=0; q<10; q++){
      printf(">>>>>>>>> %x\n", tld->stack[-q]);
  }*/

  long *registers = tld->sdbg->registers;
  unsigned int i;
  for (i = 0; i < GP_REGISTERS; i++) {
    registers[i] = tld->stack[-(2 + i)];
  }
  registers[4] = tld->stack[-1]; // The untranslated code esp that the user actually cares about

  registers[R_EIP] = org_eip; // The translated eip shouldn't leak!
                              // the person debugging isn't interested
  registers[R_ORG_EIP] = org_eip; // need this because it's read internally

  /* 32-bit only! */
  __asm__ ("movw %%cs, 36(%%eax) \n" /* tld->sdbg->registers[9] */
           "movw %%ds, 40(%%eax) \n" /* tld->sdbg->registers[10] */
           "movw %%ss, 44(%%eax) \n" /* tld->sdbg->registers[11] */
           "movw %%es, 48(%%eax) \n" /* tld->sdbg->registers[12] */
           "movw %%fs, 52(%%eax) \n" /* tld->sdbg->registers[13] */
           "movw %%gs, 56(%%eax)"    /* tld->sdbg->registers[14] */
           :  : "a" (registers) : "memory");
}

static BOOL is_pointer_valid(void* ptr){
  BOOL ok = FALSE;
  int fd[2];
  if (pipe(fd) >= 0) {
      if (write(fd[1], ptr, 4) > 0){
          ok = TRUE;
      } else {
          ok = FALSE;
      }
      close(fd[0]);
      close(fd[1]);
  } else {
      fbt_suicide_str("fbt_sdbg.c: is_pointer_valid(): error creating pipe.");
  }
  return ok;
}

/**
 * Execute the operation associated with the command sepcified
 * @param tld pointer to thread local data
 * @param command command to execute operation from
 */
static void sdbg_execute_command_operation(struct thread_local_data *tld,
                                           struct command_t command) {
  SDBG_CONSOLE_DEBUG_PRINT("sdbg exec comm op!\n");
  static int evnr = 13;
  switch (command.operation) {
    case NOP:
      SDBG_CONSOLE_DEBUG_PRINT("sdbg nop!\n");
      break;
    case MEMORY:
    {
      SDBG_CONSOLE_DEBUG_PRINT("TLD->t = %p\n", &(tld->sdbg));
      char* where = (char*)command.info;
      if (is_pointer_valid(where)){
          struct event_t event = {STOPPED, RESPONSE, (long)*where, evnr++};
          SDBG_CONSOLE_DEBUG_PRINT("sdbg mem at %p is %x!\n", command.info, event.info);
          SDBG_CONSOLE_DEBUG_PRINT("enqing\n");
          sdbg_event_enqueue(&tld->sdbg->event_queue, event);
      } else {
          printf("Pointer is not valid...\n");
          struct event_t event = {STOPPED, SIGNAL, 5}; // ARBITRARY
          sdbg_event_enqueue(&tld->sdbg->event_queue, event);
      }
      break;
    }
    case CCACHE_FLUSH:
    {
      fbt_ccache_flush(tld);
      printf("\n\nSDBG CCACHE_FLUSH!\n\n jumping to %p\n", tld->sdbg->ccache_flush_trampoline);
      //SDBG_CONSOLE_DEBUG_PRINT("\n\nSDBG CCACHE_FLUSH!\n\n jumping to %p\n", tld->sdbg->ccache_flush_trampoline);
      tld->ind_target = (void *)command.info;
      SDBG_CONSOLE_DEBUG_PRINT("sdbg ind-target %p\n", tld->ind_target);
      ulong_t *stack = tld->stack - 10;
      __asm__ volatile ("movl %0, %%esp\n"
                        "jmp *%1"
                        :
                        : "m" (stack),
                          "m" (tld->sdbg->ccache_flush_trampoline)
                        : "memory");
      break;
    }
  }
  SDBG_CONSOLE_DEBUG_PRINT("sdbg exec comm op done!\n");
}

/**
 * Wait for a new command and execute
 * @param tld pointer to thread local data
 */
static void sdbg_wait_execute_command(struct thread_local_data *tld) {
  SDBG_CONSOLE_DEBUG_PRINT("wait exec cmd\n");
  while (1) {
    volatile struct sdbg_thread_local_data *sdbg = tld->sdbg;
    SDBG_CONSOLE_DEBUG_PRINT("Sdbg: waiting for command\n");
    while (!sdbg->new_command) {
      syscall(SYS_futex, &sdbg->new_command, FUTEX_WAIT, 0, NULL, NULL, 0);
    }
    SDBG_CONSOLE_DEBUG_PRINT("Sdbg: done waiting for command\n");
    sdbg->new_command = 0;

    if (sdbg->command.reaction == STEP) {
        SDBG_CONSOLE_DEBUG_PRINT("Sdbg: switching to stepping mode\n");
        sdbg->stepping_mode = TRUE;
        PRINT_DEBUG("stepping_mode = TRUE");
    }

    sdbg_execute_command_operation(tld, sdbg->command);
    // WILL NOT REACH THIS LINE IF CCACHE_FLUSH is the operation!!!

    if (sdbg->command.reaction == CONTINUE) {
      SDBG_CONSOLE_DEBUG_PRINT("Sdbg: continuing from %p\n", (void*)sdbg->command.info);
      break;
    } if (sdbg->command.reaction == STEP) {
      SDBG_CONSOLE_DEBUG_PRINT("Sdbg: (stepp) breaking\n");
      break;
    } else if (sdbg->command.reaction == TERMINATE) {
      SDBG_CONSOLE_DEBUG_PRINT("Sdbg: terminating\n");
      fbt_suicide(125);
    }
  }
}

/**
 * Prepare for a primitive event by locking the event queue and
 * updating the registers
 * @param tld pointer to thread local data
 * @param eip instruction pointer where the event happend
 */
static void sdbg_primitive_event_init(struct thread_local_data *tld,
                                      long eip, // Unneeded
                                      long org_eip) {
  update_registers(tld, eip, org_eip);
  sdbg_event_queue_lock(&tld->sdbg->event_queue);
}

/**
 * Finalize the primitive events just happend by unlocking the event queue
 * and waiting for a new command to execute
 * @param pointer to a thread local data
 */
static void sdbg_primitive_event_finalize(struct thread_local_data *tld) {
  SDBG_CONSOLE_DEBUG_PRINT("primitive event finalize\n");
  int empty = sdbg_event_queue_empty(&tld->sdbg->event_queue);

  sdbg_event_queue_unlock(&tld->sdbg->event_queue);

  if (!empty) {
    SDBG_CONSOLE_DEBUG_PRINT("calling sdbg_wait_execute_command()\n");
    sdbg_wait_execute_command(tld);
    SDBG_CONSOLE_DEBUG_PRINT("done sdbg_wait_execute_command(), returning to asm code...\n");
  }
}

/**
 * New watchpoint event
 * @param tld pointer to thread local data
 * @param wp watchpoint hit
 */
static void sdbg_watchpoint_event(struct thread_local_data *tld,
                                  struct watchpoint_t *wp) {
  wp->times_hit++;

  struct command_t command;
  /* default reaction */
  command.reaction = STOP;
  command.operation = NOP;

  if (wp->pfunc != NULL) {
    assert(0);
    command = wp->pfunc(wp);
    sdbg_execute_command_operation(tld, command);
  }
  if (command.reaction == STOP) {
    struct event_t event = {STOPPED, PRIMITIVE, wp->number};
    sdbg_event_enqueue(&tld->sdbg->event_queue, event);
  }
}

static void sdbg_watchpoint_events(
    struct thread_local_data *tld,
    long eip,
    long org_eip,
    struct sdbg_hashmap_value_type * first_found,
    int operand_type) {

  //SDBG_CONSOLE_DEBUG_PRINT("WATCHPOINT EVENT!\n");
  printf("WATCHPOINT EVENTS! (org eip = %p)\n", (void*)org_eip);

  struct sdbg_thread_local_data *sdbg = tld->sdbg;
  struct sdbg_hashmap *hashmap = &sdbg->watchpoints;

  sdbg_primitive_event_init(tld, eip, org_eip);

  struct sdbg_hashmap_value_type *iter = first_found;
  while (iter != NULL) {
    if (iter->mapped.number == -1){ // special
        printf("Memory has ben overwritten in %p\n", (void*)iter->key);
        break;
    }


    if (iter->mapped.data == NULL) {
      iter->mapped.data = sdbg_new_primitive(sdbg,
                                             iter->mapped.number,
                                             WATCHPOINT);
    }
    struct watchpoint_t *wp = (struct watchpoint_t *) iter->mapped.data;
    if (wp->type & operand_type) {
      printf("CALLING SINGLE WATCHPOINT EVENT!\n");
      sdbg_watchpoint_event(tld, wp);
    } else {
      printf("NOT calling single wp ev since types don't match\n");
    }
    iter = sdbg_hashmap_find_next(hashmap, iter);
  }

  printf("CALLING FINALIZE EVENT!\n");
  sdbg_primitive_event_finalize(tld);

  printf("RETURNING FROM WATCHPOINT EVENTS!\n");
}

/**
 * Check if the current instruction is a memory instruction and if
 * there are watchpoints available and insert runtime checks for
 * this watchpoints.
 * @param tld pointer to thread local data
 */
static void sdbg_handle_watchpoint(struct thread_local_data *tld) {
  PRINT_DEBUG("shw 1\n");

  struct translate *ts = &tld->trans;
  unsigned char *transl_instr = ts->transl_instr;

  if (hasMemOp(ts->cur_instr_info->auxFlags)) {
#if defined(DEBUG)
    llprintf("This instruction [%x %s] with auxiliary memory operand "
             "is not supported yet\n",
             *(ts->cur_instr + ts->num_prefixes),
             ts->cur_instr_info->mnemonic);
#endif
    return;
  }

  if (hasMemOp(ts->cur_instr_info->destFlags) ||
      hasMemOp(ts->cur_instr_info->srcFlags)) {
    if (*(ts->cur_instr + ts->num_prefixes) == LEAL) { /* lea */
      return;
    } else if ((ts->cur_instr_info->destFlags & EXECUTE) ||
               (ts->cur_instr_info->srcFlags & EXECUTE)) {
      /* indirect control flow instruction */
      return;
    }

    PRINT_DEBUG("shw 2\n");

    BEGIN_ASM(transl_instr)
      pushfl
      pushl %eax
    END_ASM

    if (ts->num_prefixes > 1) {
      /* we only support one prefix atm */
      fbt_suicide_str("Only one prefix supported (sdbg_handle_watchpoint: "
                      "fbt_sdbg.c)\n");
    }

    int ret = sdbg_insert_leal(ts, &transl_instr);
    if (ret){
        BEGIN_ASM(transl_instr)
            /* sanity check: this 'hlt' is never reached: since we
               do a 'return' without updating 'transl_instr'  */
           hlt
        END_ASM
        return;
    }

    PRINT_DEBUG("shw 3\n");

    int operand_type = 0;
    if (hasMemOp(ts->cur_instr_info->destFlags)) {
      operand_type |= ts->cur_instr_info->destFlags;
    }
    if (hasMemOp(ts->cur_instr_info->srcFlags)) {
      operand_type |= ts->cur_instr_info->srcFlags;
    }

    //SDBG_CONSOLE_DEBUG_PRINT("INS WP CODE\n");

    PRINT_DEBUG("shw 4 transl_instr = %p\n", transl_instr);

    long *eip;
    BEGIN_ASM(transl_instr)
      call_abs {tld->sdbg->fast_hashmap_search_trampoline}
      testl %eax, %eax
      je no_hit

      // push the original eax on the secure stack
      pushl %ebx // temporary
      movl 4(%esp), %ebx // get the original eax
      movl %ebx, {tld->stack-2} // put the original eax on secured stack
      popl %ebx

      // Switch to secured stack
      addl $8, %esp // There is the corresponding 'sub' below
      movl %esp, {tld->stack-1}
      movl ${tld->stack-1}, %esp

      // original eax already on secure stack
      movl %ecx, -8(%esp)
      movl %edx, -12(%esp)
      movl %ebx, -16(%esp)
      movl $9999, -20(%esp) // just an arbitrary value
      movl %ebp, -24(%esp)
      movl %esi, -28(%esp)
      movl %edi, -32(%esp)
      subl $32, %esp

      pushf  // to conform with ccache_flush_trampoline convention. needed???

      pushl ${operand_type}
      pushl %eax
      pushl ${ts->next_instr /*cur_instr*/}
      pushl ${eip = (long *)transl_instr}
      pushl ${tld}
      call_abs {&sdbg_watchpoint_events}
      //int $3 // Debug
      leal 20(%esp), %esp

      popf  //
      //popa

      addl $32, %esp
      movl -8(%esp), %ecx
      movl -12(%esp), %edx
      movl -16(%esp), %ebx
      //mov -20(%esp), --  // not needed
      movl -24(%esp), %ebp
      movl -28(%esp), %esi
      movl -32(%esp), %edi

      popl %esp
      subl $8, %esp

     no_hit:
      popl %eax
      popfl
    END_ASM

    PRINT_DEBUG("shw 4d\n", transl_instr);

    *eip = (long)transl_instr;
  }

  PRINT_DEBUG("shw 5\n");

  ts->transl_instr = transl_instr;
}

/**
 * New breakpoint event
 * @param tld pointer to thread local data
 * @param bp breakpoint hit
 */
static void sdbg_breakpoint_event(struct thread_local_data *tld,
                                  struct breakpoint_t *bp) {
  bp->times_hit++;

  struct command_t command;
  /* default reaction */
  command.reaction = STOP;
  command.operation = NOP;

  if (bp->pfunc != NULL) {
    command = bp->pfunc(bp);
    sdbg_execute_command_operation(tld, command);
  }
  if (command.reaction == STOP) {
    SDBG_CONSOLE_DEBUG_PRINT("Enqueuing event with number %d\n", bp->number);
    struct event_t event = {STOPPED, PRIMITIVE, bp->number};
    sdbg_event_enqueue(&tld->sdbg->event_queue, event);
  }
}

static void sdbg_first_instruction_event(struct thread_local_data *tld) {
  SDBG_CONSOLE_DEBUG_PRINT("FIRST INSTRUCTION: may attach to   %d  \n", getpid());
  struct event_t event = {STOPPED, FIRST_INSTRUCTION, 1, -1};
  sdbg_event_enqueue(&tld->sdbg->event_queue, event);
}


/**
 * Check for breakpoints at the current instruction and insert
 * code to deliver breakpoint event at runtime.
 * @param tld pointer to thread local data
 */
static void sdbg_handle_breakpoint(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("sdbg_handle_breakpoint(*tld=%p)", tld);

  struct translate *ts = &tld->trans;
  unsigned char *transl_instr = ts->transl_instr;

  struct breakpoint_t *bp = NULL;
  struct sdbg_thread_local_data *sdbg = tld->sdbg;
  struct sdbg_hashmap *hashmap = &sdbg->breakpoints;

  struct sdbg_hashmap_value_type *iter =
      sdbg_hashmap_find(hashmap, (void *)ts->cur_instr);
  int count = 0;

  //sdbg_hashmap_dump(hashmap);

  PRINT_DEBUG("Cur instr %p!\n", ts->cur_instr);
  if (iter != NULL) {
    while(iter != NULL) {
      if (iter->mapped.data == NULL) {
        // Here it will try to open shared memory!
        iter->mapped.data = sdbg_new_primitive(sdbg,
                                               iter->mapped.number,
                                               BREAKPOINT);
      }
      bp = (struct breakpoint_t *)iter->mapped.data;

      assert(bp != NULL);

      SDBG_CONSOLE_DEBUG_PRINT("Inserting bp at %x [%x] with number %d\n",
                                   ts->cur_instr,
                                   ts->transl_instr,
                                   bp->number);

      if (count == 0) {
        BEGIN_ASM(transl_instr)
          // Switch to secured stack
          movl %esp, {tld->stack-1}
          movl ${tld->stack-1}, %esp

          pusha
          pushf
          pushl ${ts->cur_instr}
          pushl {&bp->translated_address}
          pushl ${tld}
          call_abs {&sdbg_primitive_event_init}
          leal 12(%esp), %esp
        END_ASM
      }

      BEGIN_ASM(transl_instr)
        pushl ${bp}
        pushl ${tld}
        call_abs {&sdbg_breakpoint_event}
        leal 8(%esp), %esp
      END_ASM

      iter = sdbg_hashmap_find_next(hashmap, iter);
      count++;
    }

    BEGIN_ASM(transl_instr)
      pushl ${tld}
      call_abs {&sdbg_primitive_event_finalize}
      leal 4(%esp), %esp
      popf
      popa

      popl %esp
    END_ASM
  }

  ts->transl_instr = transl_instr;

  if (count != 0) {
    /* insert breakpoint here */
    PRINT_DEBUG("breakpoint inserted at %x [%x]",
                ts->cur_instr,
                ts->transl_instr);
    SDBG_CONSOLE_DEBUG_PRINT("breakpoint inserted at %x [%x]",
                             ts->cur_instr,
                             ts->transl_instr);

    iter = sdbg_hashmap_find(hashmap, (void *)ts->cur_instr);
    while(iter != NULL) {
      bp = (struct breakpoint_t *)iter->mapped.data;
      bp->translated_address = ts->transl_instr;
      iter = sdbg_hashmap_find_next(hashmap, iter);
    }
  }

  PRINT_DEBUG_FUNCTION_END("-> void");
}

/**
 * So that we break right away. Also useful for stepping.
 */
static void sdbg_handle_first_instruction(struct thread_local_data *tld) {
  PRINT_DEBUG_FUNCTION_START("sdbg_handle_first_instruction(*tld=%p)", tld);

  struct translate *ts = &tld->trans;
  unsigned char *transl_instr = ts->transl_instr;

  PRINT_DEBUG("Cur instr %p! Transl instr %p\n", ts->cur_instr, transl_instr);

  SDBG_CONSOLE_DEBUG_PRINT("FIRST INSTR INSERTED %p, %p\n", ts->cur_instr, transl_instr);

  BEGIN_ASM(transl_instr)
    // Switch to secured stack
    movl %esp, {tld->stack-1}   // push esp on the secure stack
    movl ${tld->stack-1}, %esp  // switch to the secure stack

    pusha
    pushf

    pushl ${ts->cur_instr}
    pushl $0x0
    pushl ${tld}
    call_abs {&sdbg_primitive_event_init}
    leal 12(%esp), %esp

    pushl ${tld}
    call_abs {&sdbg_first_instruction_event}
    leal 4(%esp), %esp

    pushl ${tld}
    call_abs {&sdbg_primitive_event_finalize}
    leal 4(%esp), %esp

    popf
    popa

    popl %esp     // go back to
  END_ASM

  ts->transl_instr = transl_instr;

  PRINT_DEBUG_FUNCTION_END("-> void");
}

void sdbg_handle_flush_and_breaks(struct thread_local_data *tld) {
  static BOOL first_instruction  = TRUE;

  PRINT_DEBUG("sdbg handling flushes and breaks.");

  struct sdbg_thread_local_data *sdbg = tld->sdbg;

  if (sdbg->command.operation == CCACHE_FLUSH) {
    PRINT_DEBUG("handling flush");
    sdbg->command.operation = NOP;
  } else {
    if (sdbg->stepping_mode){
        PRINT_DEBUG("handling single step");
        sdbg_handle_first_instruction(tld);
        sdbg->stepping_mode = FALSE;
    } else  if (first_instruction){
      PRINT_DEBUG("handling first instr");
      sdbg_handle_first_instruction(tld);
      first_instruction = FALSE;
    } else {
      if (!sdbg_hashmap_empty(&sdbg->breakpoints)) {
        PRINT_DEBUG("handling bp");
        sdbg_handle_breakpoint(tld);
      }
    }
  }
}

void sdbg_handle_watchpoints(struct thread_local_data *tld) {
  PRINT_DEBUG("sdbg handling watches");
  // Let's always instrument the code to avoid unnecessary hassle
  // with flushing caches and so on.
  //struct sdbg_thread_local_data *sdbg = tld->sdbg;
  //if (!sdbg_hashmap_empty(&sdbg->watchpoints)) {  // COMM FOR DEBUG
    PRINT_DEBUG("handling wp");
    sdbg_handle_watchpoint(tld);
  //}
}

#endif







