/**
 * sandboxUI.c - A simple commandline fastBT sandbox user interface.
 * This program uses the sandboxlib.h functions to demonstrate how a user interface could be implemented.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "sandboxlib.h"

/**
 * Print usage.
 */
int usage(char* program)
{
 printf("Usage: %s <pid> [<specfile>]\n", program);
 printf("<pid>: process id of the sandboxed process\n");
 printf("<specfile>: syscall specification file (optional, default: %s)\n", SYSCALLS_SPEC_FILE);
 return 1;
}

/**
 * "Flushes" stdin. Just read characters till a newline character gets encountered.
 */
void flushinput()
{
 while(getchar() != '\n') {}
}

/**
 * Ask the user for a specific action.
 */
int ask_for_action(struct sandbox_request* sreq, struct sandbox_reply* srep)
{
 int i;
 int rettype;
 int retvalue;
 char selection;
 char retstring[MAX_STRING_LEN];

 retvalue = 0;
 selection = '\0';

 rettype = syscalls[sreq->syscallnr]->ret_type; /* assign return type */

 /* output possible actions */
 if(rettype == ARG_TYPE_INT) printf("(a)Allow? (d)Deny? (c)Create policy rule?\n(r)Return value? (s)Create return policy rule?\n:");
 else printf("(a)Allow? (d)Deny? (c)Create policy rule?\n:");

 /* get action input */
 if(rettype == ARG_TYPE_INT)
 {
  while(selection != 'a' && selection != 'd' && selection != 'r' && selection != 'c' && selection != 's')
  {
   if(scanf("%c", &selection) != 1) return -1;
  }
 }
 else
 {
  while(selection != 'a' && selection != 'd' && selection != 'c')
  {
   if(scanf("%c", &selection) != 1) return -1;
  }
 }

 /* switch action input */
 switch(selection)
 {
  /* allow */
  case 'a':
    srep->action = ACTION_ALLOW;
    break;
  /* deny */
  case 'd':
    srep->action = ACTION_DENY;
    break;
  /* return value */
  case 'r':
    {
     srep->action = ACTION_RETURN;
     printf("Return value <int>:"); /* read return value */
     if(scanf("%d", &retvalue) != 1) return -1;
     srep->retvalue = retvalue;
    }
    break;
  /* create new rule */
  case 'c':
    {
     srep->action = ACTION_CREATE;
     if(policy_mode == BLACKLIST) printf("A new blacklist rule will be created.\n");
     else if(policy_mode == WHITELIST) printf("A new whitelist rule will be created.\n");
     printf("Please specifiy the %d arguments.\n", syscalls[sreq->syscallnr]->nr_args);

     flushinput(); /* get rid of the newline from the last scanf */

     /* get all policy rule arguments from user */
     for(i = 0; i < syscalls[sreq->syscallnr]->nr_args; i++)
     {
	switch(syscalls[sreq->syscallnr]->arguments[i])
        {
		case ARG_TYPE_INT:
		   printf("Arg %d, int {*, <int>} default(%s):", i+1, sreq->args[i]);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], sreq->args[i], MAX_STRING_LEN);
		   break;
		case ARG_TYPE_PTR:
		   printf("Arg %d, ptr {*, null} default(*):", i+1);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], WILDCARD, MAX_STRING_LEN);
		   break;
		case ARG_TYPE_STR:
		   printf("Arg %d, string {*, <string with wildcard *>} default(%s):", i+1, sreq->args[i]);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], sreq->args[i], MAX_STRING_LEN);
		   break;
		//case ARG_TYPE_UNDEF:
		default:
		   printf("Argument %d has type undef.\n", i+1);
		   strncpy(srep->args[i], WILDCARD, MAX_STRING_LEN);
		   break;
        }
     }
    }
    break;
  /* create new return rule */
  case 's':
    {
     srep->action = ACTION_CREATE_RET;
     printf("Please specifiy the %d arguments.\n", syscalls[sreq->syscallnr]->nr_args);

     flushinput(); /* get rid of the newline from the last scanf */

     /* get all policy rule arguments from user */
     for(i = 0; i < syscalls[sreq->syscallnr]->nr_args; i++)
     {
	switch(syscalls[sreq->syscallnr]->arguments[i])
        {
		case ARG_TYPE_INT:
		   printf("Arg %d, int {*, <int>} default(%s):", i+1, sreq->args[i]);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], sreq->args[i], MAX_STRING_LEN);
		   break;
		case ARG_TYPE_PTR:
		   printf("Arg %d, ptr {*, null} default(*):", i+1);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], WILDCARD, MAX_STRING_LEN);
		   break;
		case ARG_TYPE_STR:
		   printf("Arg %d, string {*, <string with wildcard *>} default(%s):", i+1, sreq->args[i]);
		   if(getstring(srep->args[i], MAX_STRING_LEN) == 1) strncpy(srep->args[i], sreq->args[i], MAX_STRING_LEN);
		   break;
		//case ARG_TYPE_UNDEF:
		default:
		   printf("Argument %d has type undef.\n", i+1);
		   strncpy(srep->args[i], WILDCARD, MAX_STRING_LEN);
		   break;
        }
     }
        /* get return value for return rule */
	printf("Return value <int> default(0):");
	if(getstring(retstring, MAX_STRING_LEN) == 1) strncpy(retstring, "0", MAX_STRING_LEN);
	srep->retvalue = atoi(retstring);
    }
    break;
  default:
    break;
 }

 return 0;
}

/**
 * Read a string and store it to the specified char*. Reads and writes a maximum of 'maxsize' bytes.
 */
int getstring(char* str, int maxsize)
{
 int i;

 i = 0;
 /* loop a maximum of maxsize-1 times */
 while(i < maxsize-1)
 {
  str[i] = (char)getchar(); /* read char */
  i++;
  if(str[i-1] == '\n') break; /* exit loop if newline encountered */
 }

 str[i-1] = '\0'; /* terminate string with nullbyte */

 return i;
}

/**
 * Initialize IPC with fastBT and ask user for policy mode.
 */
int init(int argc, char *argv[], struct sandbox_init* sinit)
{
 int mode_selection;

 /* print welcome screen */
 printf("\n [sandboxUI] - a simple commandline user interface for fastBT's interactive sandbox mode\n\n");

 printf("Attaching to process %d ... ", atoi(argv[1]));

 /* init sandbox IPC */
 if(init_sandbox_ipc(atoi(argv[1]), sinit) < 0)
 {
  printf("failed.\nIs the sandboxed process already running?\nIs interactive sandbox mode enabled?\n");
  return -1;
 }

 printf("succeeded.\n");

 /* if mode is undefined -> no policy exists, ask user which kind of policy should be generated */
 if(sinit->mode == UNDEFINED)
 {
  /* prompt for mode and set mode */
  printf("A new policy will be generated.\n");
  mode_selection = 0;
  while(mode_selection != 1 && mode_selection != 2)
  {
   printf("Please select a policy mode to be used:\n");
   printf("1: blacklist\n");
   printf("2: whitelist\n");
   printf("Enter selection: ");
   if(scanf("%d", &mode_selection) != 1) return -1;
  }
  sinit->mode = mode_selection;
 }
 else
 {
  printf("A policy was already found.\n");
 }

 /* notify sandbox */
 if(init_sandbox_ipc_reply(sinit) < 0)
 {
  printf("Replying to sandbox IPC initialization failed.\n");
  return -1;
 }

 if(sinit->mode == BLACKLIST) printf("Mode: blacklist\n");
 else if(sinit->mode == WHITELIST) printf("Mode: whitelist\n");
 else
 {
  printf("Setting mode failed.\n");
  return -1;
 }

 /* assign mode */
 policy_mode = sinit->mode;

 return 0;
}

/**
 * Wait for syscall events, call ask_for_action() for each event and
 * notify the fastBT sandbox about the selected action.
 */
int wait_for_syscalls(struct sandbox_request* sreq, struct sandbox_reply* srep)
{
 int i;

 printf("Process is running, waiting for syscall...(ctrl-C to write policy and exit)\n");

 /* wait for syscall events loop */
 while(wait_for_syscall(sreq) >= 0)
 {

  /*****************************/
  /* print syscall information */
  /*****************************/
  /* print syscall type information */
  printf("\nProcess called ");
  printf("%s(", syscalls[sreq->syscallnr]->name);
  for(i = 0; i < syscalls[sreq->syscallnr]->nr_args; i++)
  {
	if(syscalls[sreq->syscallnr]->arguments[i] == ARG_TYPE_INT) printf("%s", ARG_TYPE_INT_STR);
	else if(syscalls[sreq->syscallnr]->arguments[i] == ARG_TYPE_PTR) printf("%s", ARG_TYPE_PTR_STR);
	else if(syscalls[sreq->syscallnr]->arguments[i] == ARG_TYPE_STR) printf("%s", ARG_TYPE_STR_STR);
	else if(syscalls[sreq->syscallnr]->arguments[i] == ARG_TYPE_UNDEF) printf("%s", ARG_TYPE_UNDEF_STR);
	else return -1; // not very nice
	if(i < syscalls[sreq->syscallnr]->nr_args - 1) printf(", ");
  }
  printf(")");
  if(syscalls[sreq->syscallnr]->ret_type == ARG_TYPE_INT) printf(":int");
  else if(syscalls[sreq->syscallnr]->ret_type == ARG_TYPE_UNDEF) printf(":undef");
  else if(syscalls[sreq->syscallnr]->ret_type == ARG_TYPE_VOID) printf(":void");

  /* print specific syscall information */
  printf(" >               %s(", syscalls[sreq->syscallnr]->name);
  for(i = 0; i < syscalls[sreq->syscallnr]->nr_args; i++)
  {
        printf("%s", sreq->args[i]);
        if(i < syscalls[sreq->syscallnr]->nr_args - 1) printf(", ");
  }
  printf(")\n");
  /*****************************/

  /* ask user for action */
  if(ask_for_action(sreq, srep) < 0)
  {
    /* an error occured during user prompt, exit loop */
    printf("Asking for action failed.\n");
    break;
  }

  /* notify sandbox about user selection */
  if(notify_sandbox(srep) < 0)
  {
    printf("Notifying sandbox failed.\n");
    break;
  }

  /* if new rule was created, insert the rule to the rules table */
  if(srep->action == ACTION_CREATE || srep->action == ACTION_CREATE_RET)
  {
   if(add_policy_rule(sreq->syscallnr, srep) < 0) printf("\nCreating new policy rule failed.\n");
  }

  printf("\nProcess is running, waiting for syscall...(ctrl-C to write policy and exit)\n");
 }

 /* loop exited because of an error, return 1 */
 return -1;
}

void handlesignal(int sig)
{
 //printf("Wake up call ... !!! - Catched signal: %d ... !!\n", sig);
 printf("\n\nCatched ctrl-C, writing policy to %s ... ", POLICY_FILE);

 if(write_policy(POLICY_FILE) < 0) printf("failed.\n");
 else printf("succeeded.\n");

 exit(0);
}

/**
 * sandboxUI main function.
 */
int main(int argc, char *argv[])
{
 char* specfile;
 struct sandbox_init sinit;
 struct sandbox_request sreq;
 struct sandbox_reply srep;

 /* check arguments*/
 if(argc < 2 || argc > 3) return usage(argv[0]);

 /* set specfile */
 if(argc == 3) specfile = argv[2];
 else specfile = NULL;

 /* load syscall specification */
 if(load_syscall_specification(specfile) < 0)
 {
  printf("Loading syscall specification failed.\n");
  return -1;
 }

 /* init and print welcome screen */
 if(init(argc, argv, &sinit) != 0) return -1;

 signal(SIGINT, handlesignal);

 /* enter wait for syscalls loop */
 return wait_for_syscalls(&sreq, &srep);
}
