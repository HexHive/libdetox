#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "fbt_sandbox.h"
#include "fbt_policy_parser.h"
#include "fbt_sec_syscalls.h" /* for authorize_syscl_table[] */

#include "fbt_mem_alloc.h"
#include "fbt_mem_protection.h"
#include "fbt_llio.h"
#include "libfastbt.h"

static struct syscall_desc *syscalls[NR_SYSCALLS + 1]; /* the syscalls descriptions, stored in an array and indexed by the systemcall nr */
static struct policy_rule *rules[NR_SYSCALLS + 1]; /* the policy rules, indexed by the systemcall nr, each element is the first element of a linked list of policy rules */
static int sandbox_mode; /* 0: server mode, 1: interactive mode */
static int policy_mode; /* 1: blacklist, 2: whitelist */
static int auth_mode; /* default authorization in case no syscall specification is specified - 0: deny, 1: allow */
verify_syscall_argument_fptr_t verify_table[ARG_NR_OF_TYPES]; /* table of verification functions, for each argument type one */
int logfd; /* filedescriptor of logfile */
int msqid; /* message queue id */
key_t key; /* message queue mode */
static int initialized = 0; /* remembers if the sandbox is already initialized */
char* call_origin; /* buffer for call origin object strings */

/**
 * Initialize the sandbox.
 * @return 0 if initialization succeeded, otherwise -1
 */
int fbt_init_sandbox()
{
    int i;
#ifdef TRUST_SANDBOX_INTERACTIVE
    struct sandbox_init sinit;
#endif

    //llprintf("initialize sandbox\n");
    if(initialized)
    {
     //llprintf("sandbox already initialized\n");
     return 0;
    }

    /* not used? */
#ifdef TRUST_SANDBOX_INTERACTIVE
    sandbox_mode = 1;
#else
    sandbox_mode = 0;
#endif

    policy_mode = BLACKLIST_MODE; /* default policy mode is blacklisting */

    /* set all elements to NULL */
    for(i = 0; i < NR_SYSCALLS; i++)
    {
	syscalls[i] = NULL;
	rules[i] = NULL;
    }

    /* default authorization mode deny */
    auth_mode = AUTH_DEF_DENY;

    /* load system call specification file */
    if(fbt_load_syscall_specification() < 0)
    {
     llprintf("Loading syscall specification failed.\n");
     return -1;
    }

   /* special handlers in secuBT for syscalls: 5 open, 8 creat, 11 execve, 26 ptrace, 90 mmap, 125 mprotect, 285 sys_setaltroot, 295 openat */

   /* modify authorize_sycl_table according to the syscall specification
    * if a syscall is specified replace authorization handler with sandbox authorization handler
    */
    for(i = 0; i < NR_SYSCALLS; i++)
    {
#ifdef TRUST_DONT_HANDLE_ALL_SYSCALLS
	if(syscalls[i] != NULL) /* syscall is specified */
	{
#endif
		//llprintf("syscall spec found for %d, %s, %d\n", syscalls[i]->nr, syscalls[i]->name, syscalls[i]->nr_args);
#ifdef TRUST_HANDLE_SPECIAL_SYSCALLS
		if(authorize_syscl_table[i] != allow_syscall) syscalls[i]->old_authorize = authorize_syscl_table[i];
		else syscalls[i]->old_authorize = NULL;
#endif
		authorize_syscl_table[i] = fbt_sandbox_authorization;
#ifdef TRUST_DONT_HANDLE_ALL_SYSCALLS
	}
#endif
    }


#ifdef TRUST_SANDBOX_LOG
    logfd = 0;
#else

    /* load policy file and generate rules */
#ifdef TRUST_SANDBOX_INTERACTIVE
    if(fbt_load_policy(syscalls, rules, &policy_mode) < 0 && policy_exists())
#else
    if(fbt_load_policy(syscalls, rules, &policy_mode) < 0)
#endif
    {
     llprintf("Loading policy failed.\n");
     return -1;
    }

    /* init verify_table */
    verify_table[0] = verify_arg_int;
    verify_table[1] = verify_arg_ptr;
    verify_table[2] = verify_arg_string;
    verify_table[3] = verify_arg_undef;
    verify_table[4] = verify_arg_void;

#ifdef TRUST_SANDBOX_INTERACTIVE

    /* init IPC message queue */
    if(policy_exists()) sinit.mode = policy_mode; /* policy exists and was loaded successfully */
    else sinit.mode = UNDEFINED; /* policy does not exist */

    llprintf("Initializing IPC pid = %d ..... ", (int)getpid());

    /* send message */
    sinit = init_sandbox_ipc(sinit);

    /* set policy mode */
    policy_mode = sinit.mode;

    if(sinit.mode == BLACKLIST) llprintf("done.\nPolicy mode: blacklist (%d)\n", sinit.mode);
    else if(sinit.mode == WHITELIST) llprintf("done.\nPolicy mode: whitelist (%d)\n", sinit.mode);

#endif /* TRUST_SANDBOX_INTERACTIVE */

    /* just for debugging purposes */
    //print_rules();

#endif /* TRUST_SANDBOX_LOG */

#ifdef TRUST_SANDBOX_LOG_CALL_ORIGIN
 call_origin = fbt_smalloc_pers(get_tld(), MAX_SIZE_SYMB);
#endif

    initialized = 1;
    return 0;
}

/**
 * Load systemcalls specification file. -> REWRITE?
 * Specification file with following structure:
 * ...
 * NR1|NAME1(ARG1, ARG2,...):RETTYPE
 * NR2|NAME2(ARG1, ARG2,...):RETTYPE
 * ...
 * @return 0 if specification was loaded successfully, in case of failure -1
 */
int fbt_load_syscall_specification()
{
  int fd;
  char *filemem;
  char *addr;
  char *tmpstr;
  struct stat attr;
  struct syscall_desc *el;

  char buf[MAX_SIZE_SYMB];
  int argsparsed, argscount;

  /* open specifiaction file */
  if((fd = open(SYSCALLS_SPEC_FILE, O_RDONLY)) < 0)
  {
   llprintf("fbt_load_syscall_specification: open failed\n");
   return -1;
  }
  /* get stats */
  if(fstat(fd, &attr) == -1)
  {
   llprintf("fbt_load_syscall_specification: fstat failed\n");
   return -1;
  }

  /* map file to memory */
  filemem = (char*)mmap(0, attr.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if(filemem == ((caddr_t)-1))
  {
   llprintf("fbt_load_syscall_specification: mmap failed\n");
   return -1;
  }
  addr = filemem;

  /* close file */
  close(fd);

  /* read default mode definition */
  /* "default:" {"allow", "deny"} */
  while(*filemem != '\n') filemem++; /* move to first newline */
  if(filemem-addr > MAX_SIZE_SYMB)
  {
   llprintf("fbt_load_syscall_specification: symbol length exceeded\n");
   return -1;
  }
  strncpy(buf, addr, (filemem-addr));
  buf[filemem-addr] = 0x00; /* terminate line with nullbyte */
  if(strcmp(buf, AUTH_DENY) == 0) auth_mode = AUTH_DEF_DENY;
  else if(strcmp(buf, AUTH_ALLOW) == 0) auth_mode = AUTH_DEF_ALLOW;
  else
  {
   llprintf("fbt_load_syscall_specification: no default symbol found\n");
   return -1;
  }
  filemem++; /* go to 2nd line */

  /* loop through mapped memory till the end, one iteration per line */
  while((filemem-addr) < attr.st_size)
  {
    el = fbt_smalloc_pers(get_tld(), sizeof(struct syscall_desc)); /* allocate memory for a new syscall description element */
    argsparsed = 0;
    argscount = 0;

    /* NR|... */
    tmpstr = buf;
    while(*filemem != '|' && (filemem-addr) < attr.st_size && isdigit(*filemem) && (tmpstr-buf) < MAX_SIZE_SYMB) /* copy till character '|' encountered */
    {
     *tmpstr = *filemem; /* copy character */
     tmpstr++; filemem++; /* move one byte on */
    }
    filemem++;
    *tmpstr = 0x00; tmpstr++; /* terminate char buffer with 0x00 */
    el->nr = atoi(buf); /* convert string to number */

    /* ...|NAME(... */
    tmpstr = buf;
    while(*filemem != '(' && (filemem-addr) < attr.st_size && (tmpstr-buf) < MAX_SIZE_SYMB) /* copy till character '(' encountered */
    {
     *tmpstr = *filemem; /* copy character */
     tmpstr++; filemem++; /* move one byte on */
    }
    filemem++;
    *tmpstr = 0x00; tmpstr++; /* terminate char buffer with 0x00 */
    el->name = fbt_smalloc_pers(get_tld(), strlen(buf) + 1); /* allocate name string memory */
    el->name[strlen(buf)] = 0x00;
    strncpy(el->name, buf, strlen(buf));

    /* ARG,... */
    if(*filemem != ')') /* if a first argument follows */
    {
	el->arguments = fbt_smalloc_pers(get_tld(), sizeof(int)*MAX_NR_ARGS);  /* allocate arguments array memory */
	while(!argsparsed && argscount <= MAX_NR_ARGS)
	{
		tmpstr = buf;
		while(*filemem != ',' && *filemem != ')' && (filemem-addr) < attr.st_size && (tmpstr-buf) < MAX_SIZE_SYMB)
		{
		 *tmpstr = *filemem; /* copy character */
		 tmpstr++; filemem++; /* move one byte on */
		}
		if(*filemem == ')') argsparsed = 1;
		filemem++;
		*tmpstr = 0x00; tmpstr++; /* terminate char buffer with 0x00 */
		if(strcmp(buf, ARG_TYPE_INT_STR) == 0) el->arguments[argscount] = ARG_TYPE_INT;
		else if(strcmp(buf, ARG_TYPE_PTR_STR) == 0) el->arguments[argscount] = ARG_TYPE_PTR;
		else if(strcmp(buf, ARG_TYPE_STR_STR) == 0) el->arguments[argscount] = ARG_TYPE_STR;
		else if(strcmp(buf, ARG_TYPE_UNDEF_STR) == 0) el->arguments[argscount] = ARG_TYPE_UNDEF;
		else if(strcmp(buf, ARG_TYPE_VOID_STR) == 0) el->arguments[argscount] = ARG_TYPE_VOID;
		else
		{
		 llprintf("fbt_load_syscall_specification: argument type not valid (%d, %s)\n", argscount, buf);
		 return -1;
		}
		argscount++;
	}
    }
    else filemem++;

    el->nr_args = argscount;

    /* ):RETTYPE */
    if(*filemem == ':')
    {
	filemem++;
	tmpstr = buf;
	while(*filemem != 0x0a && (filemem-addr) < attr.st_size && (tmpstr-buf) < MAX_SIZE_SYMB)
	{
	 *tmpstr = *filemem; /* copy character */
	 tmpstr++; filemem++; /* move one byte on */
	}
	filemem++;
	*tmpstr = 0x00; tmpstr++; /* terminate char buffer with 0x00 */
	if(strcmp(buf, ARG_TYPE_INT_STR) == 0) el->ret_type = ARG_TYPE_INT;
	else if(strcmp(buf, ARG_TYPE_PTR_STR) == 0) el->ret_type = ARG_TYPE_PTR;
	else if(strcmp(buf, ARG_TYPE_STR_STR) == 0) el->ret_type = ARG_TYPE_STR;
	else if(strcmp(buf, ARG_TYPE_UNDEF_STR) == 0) el->ret_type = ARG_TYPE_UNDEF;
	else if(strcmp(buf, ARG_TYPE_VOID_STR) == 0) el->ret_type = ARG_TYPE_VOID;
	else
	{
	 llprintf("fbt_load_syscall_specification: return type not valid (%s)\n", buf);
	 return -1;
	}
    }

    /* if parsed syscall number is valid, add syscall description to syscalls array */
    if(el->nr >= 0 && el->nr <= NR_SYSCALLS) syscalls[el->nr] = el;

    /* if not already done, move pointer to next line */
    if(*filemem == 0x0a) filemem++;
  }

  /* unmap file */
  munmap(addr, attr.st_size);
  return 0;
}

/**
 * Writes the object name of the call origin to char* origin.
 * TO FIX: in case of !is_sysenter a segfault occurs because of a corrupt rip (?)
 */
void get_call_origin(int arg6, int is_sysenter, char* origin, int maxbytes)
{
 struct mem_info info;

 void** base_ptr;
 void* rip = 0;

 base_ptr = (void**) arg6;

 if(is_sysenter) rip = *(base_ptr + RIP_OFFSET_SYSENTER);
 else if((int)(base_ptr+RIP_OFFSET) > 0x000000FF) rip = *(base_ptr + RIP_OFFSET);
 //else rip = *(base_ptr + RIP_OFFSET);

 if(rip != 0 && fbt_memprotect_info(rip, &info)) strncpy(origin, info.obj_name, maxbytes);
 else strncpy(origin, WILDCARD, maxbytes);
}

/**
 * Returns the rip of the function that did the system call.
 * TO FIX: in case of !is_sysenter a segfault occurs because of a corrupt rip (?)
 */
void* get_rip(int arg6, int is_sysenter)
{
 //int call_stack_size = get_call_stack((void**) arg6, &call_stack, is_sysenter);
 void** base_ptr;
 void* rip = 0;

 base_ptr = (void**) arg6;

 if(is_sysenter)
 {
  rip = *(base_ptr + RIP_OFFSET_SYSENTER);
 }
 else if((int)(base_ptr+RIP_OFFSET) > 0x000000FF) rip = *(base_ptr + RIP_OFFSET);
 /*else
 {
  rip = *(base_ptr + RIP_OFFSET);
 }*/

 return rip;
}

/**
 * Checks if the origin of the call is allowed according to the policy.
 * TO FIX: in case of !is_sysenter a segfault occurs because of a corrupt rip (?)
 */
int is_call_origin_allowed(int arg6, int is_sysenter, struct policy_rule* rule)
{
 int ret;
 struct mem_info info;
 void* rip;

 ret = 0;

 if(rule->call_origin == NULL) return 1;

 /* get the rip */
 rip = get_rip(arg6, is_sysenter);

 /* resolve the location */
 if(rip != 0 && fbt_memprotect_info(rip, &info))
 {
  if(strcmp(info.obj_name, rule->call_origin) == 0) ret = 1;
 }
 else ret = 1; /* to fix, if no address could be determined, just allow */

 return ret;
}

/**
 * Sandbox authorization function.
 * Authorizes systemcalls according to the loaded policy.
 * @param syscall_nr the systemcall number stored in %eax
 * @param arg1 the first argument, stored in %ebx
 * @param arg2 the second argument, stored in %ecx
 * @param arg3 the third argument, stored in %edx
 * @param arg4 the fourth argument, stored in %esi
 * @param arg5 the fifth argument, stored in %edi
 * @param arg6 the sixth argument, stored in %ebp
 * @param retval pointer to the integer where a return value may be put
 * @return 1 if the systemcall is allowed according to the policy, 0 otherwise
 */
int fbt_sandbox_authorization(int syscall_nr, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int is_sysenter, int *retval)
{
 int j;
 int matched;
 struct policy_rule* rule;
 struct syscall_desc* syscall;
 int args[6];
#ifdef TRUST_SANDBOX_INTERACTIVE
 struct sandbox_request sreq;
 struct sandbox_reply srep;
#endif

 /* get syscall description */
 syscall = syscalls[syscall_nr];

/* just log the syscall */
#ifdef TRUST_SANDBOX_LOG

#ifdef TRUST_SANDBOX_LOG_CALL_ORIGIN
 get_call_origin(arg6, is_sysenter, call_origin, MAX_SIZE_SYMB);
#endif

 if(syscall != NULL)
 {
  if(log_syscall(syscall, arg1, arg2, arg3, arg4, arg5, arg6, is_sysenter, call_origin) != 0 )
  {
   llprintf("Could not log syscall.");
  }
 }

 return 1; /* allow */

#else

 /* store arguments in args array */
 args[0] = arg1;
 args[1] = arg2;
 args[2] = arg3;
 args[3] = arg4;
 args[4] = arg5;
 args[5] = arg6;

 /* get rule */
 rule = rules[syscall_nr];

 if(syscall_nr < 0 && syscall_nr > NR_SYSCALLS) return 1; /* should not happen */

 /* Go through all policy rules for this specific syscall and execute the correct actions depending on the policy mode. */

#ifdef TRUST_HANDLE_SPECIAL_SYSCALLS
  if(syscall->old_authorize != NULL && syscall->old_authorize(syscall_nr, arg1, arg2, arg3, arg4, arg5, arg6, is_sysenter, retval) == 0)
  {
	llprintf("The special authorization function disallowed the syscall.\n");
	llprintf("Syscall Nr: %d Args(%d, %d, %d, %d, %d, %d)\n", syscall_nr, arg1, arg2, arg3, arg4, arg5, arg6);
	llprintf("Program forced to exit.\n");
	_exit(1);
  }
#endif

 if(syscall != NULL)
 {

  if(policy_mode == BLACKLIST_MODE)
  {

#ifdef TRUST_SANDBOX_INTERACTIVE
   if(rule == NULL)
   {
    return handle_interactive_action(&sreq, &srep, syscall, args, is_sysenter, retval); /* we are in interactive mode, handle an interactive syscall */
   }
#else
   if(rule == NULL) return 1; /* syscall is authorized in blacklist mode */
#endif

   while(rule != NULL) /* if at least one rule exists for called syscall */
   {

    matched = 0; /* set matched 0 */

    for(j = 0; j < syscall->nr_args; j++) /* iterate through all arguments */
    {
     /* verify if the argument matches the policy rule */
     if(verify_table[syscall->arguments[j]-1](rule->arg_rules[j], args[j], syscall, rule) == 0) matched++;
     else break; /* a non matching argument found -> policy rule does not match -> break */
    }

#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
    if(matched == syscall->nr_args && is_call_origin_allowed(arg6, is_sysenter, rule))
#else
    if(matched == syscall->nr_args)
#endif
    {
     if(rule->action == ACTION_RETURN)
     {
       *retval = atoi(rule->ret_string);
       return 0; /* deny syscall and return retval */
     } else abort_policy_violation(syscall, rule, arg1, arg2, arg3, arg4, arg5, arg6, is_sysenter); /* rule matched -> policy violation */
    }

    rule = rule->next;
   }

#ifdef TRUST_SANDBOX_INTERACTIVE
   return handle_interactive_action(&sreq, &srep, syscall, args, is_sysenter, retval); /* we are in interactive mode, handle an interactive syscall */
#else
   return 1; /* no rule found, return 1 */
#endif

  }
  else if(policy_mode == WHITELIST_MODE)
  {

#ifdef TRUST_SANDBOX_INTERACTIVE
   if(rule == NULL)
   {
    return handle_interactive_action(&sreq, &srep, syscall, args, is_sysenter, retval); /* we are in interactive mode, handle an interactive syscall */
   }
#else
   if(rule == NULL) abort_no_rule_found(syscall, arg1, arg2, arg3, arg4, arg5, arg6, is_sysenter); /* syscall is not authorized in whitelist mode */
#endif

   while(rule != NULL) /* if at least one rule exists for called syscall */
   {

    matched = 0; /* set matched 0 */

    for(j = 0; j < syscall->nr_args; j++) /* iterate through all arguments */
    {
     /* verify if the argument matches the policy rule */
     if(verify_table[syscall->arguments[j]-1](rule->arg_rules[j], args[j], syscall, rule) == 0) matched++;
     else break; /* a non matching argument found -> policy rule does not match -> break */
    }

#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
    if(matched == syscall->nr_args && is_call_origin_allowed(arg6, is_sysenter, rule))
#else
    if(matched == syscall->nr_args)
#endif
    {
     if(rule->action == ACTION_RETURN)
     {
       *retval = atoi(rule->ret_string);
       return 0; /* deny syscall and return retval */
     } else return 1; /* rule matched -> syscall authorized */
    }

    rule = rule->next;
   }

#ifdef TRUST_SANDBOX_INTERACTIVE
   return handle_interactive_action(&sreq, &srep, syscall, args, is_sysenter, retval); /* we are in interactive mode, handle an interactive syscall */
#else
   abort_no_rule_found(syscall, arg1, arg2, arg3, arg4, arg5, arg6, is_sysenter); /* no rule found, abort */
#endif

  }
  else return 1;

 }
 else /* syscall definition is NULL */
 {
  if(auth_mode == AUTH_DEF_DENY) abort_no_syscalldef_found(syscall_nr, arg1, arg2, arg3, arg4, arg5, arg6); /* deny in case the syscall is not specified */
  else if(auth_mode == AUTH_DEF_ALLOW) return 1; /* allow in case the syscall is not specified */
 }

 /* should never happen */

 return 1; /* default: accept if syscall was not specified in syscalls specification */

#endif /* TRUST_SANDBOX_LOG */

}

/**
 * Handles an interactive syscall.
 * Notify user interface, wait for an answer and act according to the user's reply.
 */
int handle_interactive_action(struct sandbox_request* sreq, struct sandbox_reply* srep, struct syscall_desc* syscall, int* args, int is_sysenter, int* retval)
{
    int syscall_nr;

    syscall_nr = syscall->nr;

    /* fill sandbox_request struct */
    fill_sreq_with_args(sreq, syscall, args);
    /* send a request to the user interface */
    notify_UI(sreq, srep);

    /* handle the user interface reply */
    switch(srep->action)
    {
      /* the syscall got denied -> abort */
      case ACTION_DENY: abort_from_deny(syscall, args[0], args[1], args[2], args[3], args[4], args[5]); break;
      /* the syscall got allowed -> return 1 */
      case ACTION_ALLOW: return 1; break;
      /* a specific return value has to be returned */
      case ACTION_RETURN:
           {
             *retval = srep->retvalue;
             return 0;
           }
           break;
      /* a new rule has to be created */
      /* create the rule and re-authorize the syscall */
      case ACTION_CREATE:
           {
             insert_policy_rule(syscall_nr, srep);
             return fbt_sandbox_authorization(syscall_nr, args[0], args[1], args[2], args[3], args[4], args[5], is_sysenter, retval);
           }
           break;
      /* a new return rule has to be created */
      /* create the rule and re-authorize the syscall */
      case ACTION_CREATE_RET:
           {
             insert_policy_rule(syscall_nr, srep);
             return fbt_sandbox_authorization(syscall_nr, args[0], args[1], args[2], args[3], args[4], args[5], is_sysenter, retval);
           }
           break;
    }

    return 1; /* default case: allow - should never be reached */
}

/**
 * Creates and inserts a new policy rule into the rules table.
 * The rules fields get constructed out of the sandbox_reply information.
 * To improve: check fbt_smalloc return value!
 */
void insert_policy_rule(int syscallnr, struct sandbox_reply* srep)
{
 int i;
 struct policy_rule* tmp_rule;
 struct policy_rule* new_rule = (struct policy_rule*)fbt_smalloc_pers(get_tld(), sizeof(struct policy_rule));

 /* set syscall description */
 new_rule->syscall = syscalls[syscallnr];
 /* allocate arg_rules array */
 new_rule->arg_rules = fbt_smalloc_pers(get_tld(), sizeof(char*)*new_rule->syscall->nr_args);

 /* iterate through all arguments, allocate memory and copy the string from the sandbox_reply message */
 for(i = 0; i < new_rule->syscall->nr_args; i++)
 {
  new_rule->arg_rules[i] = fbt_smalloc_pers(get_tld(), strlen(srep->args[i])+1);
  strncpy(new_rule->arg_rules[i], srep->args[i], strlen(srep->args[i])+1);
 }

 /* set action according to policy mode */
 if(policy_mode == BLACKLIST_MODE) new_rule->action = ACTION_DENY;
 else if(policy_mode == WHITELIST_MODE) new_rule->action = ACTION_ALLOW;

 /* if a return rule gets created, set the fields accordingly */
 if(srep->action == ACTION_CREATE_RET)
 {
  new_rule->action = ACTION_RETURN;
  new_rule->ret_string = fbt_smalloc_pers(get_tld(), MAX_SIZE_SYMB);
  snllprintd(new_rule->ret_string, MAX_SIZE_SYMB, srep->retvalue);
 }

 /* rule created, append to rules table */
 tmp_rule = rules[new_rule->syscall->nr];
 if(tmp_rule == NULL)
 {
  rules[new_rule->syscall->nr] = new_rule;
 }
 else
 {
  while(tmp_rule->next != NULL) tmp_rule = tmp_rule->next;
  tmp_rule->next = new_rule;
 }

 new_rule->next = NULL; /* set next rule to NULL */
}

/**
 * Fills the sandbox_request struct with the specified values from the syscall description and the syscall arguments.
 */
void fill_sreq_with_args(struct sandbox_request* sreq, struct syscall_desc* syscall, int* args)
{
 int i;

 sreq->syscallnr = syscall->nr;

 for(i = 0; i < syscall->nr_args; i++) /* iterate through all arguments */
 {
  if(syscall->arguments[i] == ARG_TYPE_INT) snllprintd(sreq->args[i], MAX_STRING_LEN, args[i]);
  else if(syscall->arguments[i] == ARG_TYPE_STR) strncpy(sreq->args[i], (char*)args[i], MAX_STRING_LEN);
  else if(syscall->arguments[i] == ARG_TYPE_PTR) snllprintp(sreq->args[i], MAX_STRING_LEN, (void*)args[i]);
  else strncpy(sreq->args[i], WILDCARD, MAX_STRING_LEN);
 }

}

/**
 * Aborts in case the systemcall wasn't specified.
 */
void abort_no_syscalldef_found(int syscall_nr, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
 llprintf("A non specified syscall was called.\n");
 llprintf("Syscall Nr: %d Args(%d, %d, %d, %d, %d, %d)\n", syscall_nr, arg1, arg2, arg3, arg4, arg5, arg6);
 llprintf("Program forced to exit.\n");
 _exit(1);
}

/**
 * Aborts in case of a policy violation. -> blacklist
 */
void abort_policy_violation(struct syscall_desc* syscall, struct policy_rule* rule, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int is_sysenter)
{
 int i;
 int args[6];
#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
 char* origin = fbt_smalloc_pers(get_tld(), MAX_SIZE_SYMB);
#endif

 /* store arguments in args array */
 args[0] = arg1;
 args[1] = arg2;
 args[2] = arg3;
 args[3] = arg4;
 args[4] = arg5;
 args[5] = arg6;

 llprintf("Sandbox policy violated.\n");
 llprintf("Syscall: %s(", syscall->name);
 for(i = 0; i < syscall->nr_args; i++)
 {
  if(syscall->arguments[i] == ARG_TYPE_STR) llprintf("%s", (char*)args[i]);
  else llprintf("%d", args[i]);
  if(syscall->nr_args > 1 && i < syscall->nr_args-1) llprintf(", ");
 }
 llprintf(")\n");
#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
 get_call_origin(arg6, is_sysenter, origin, MAX_SIZE_SYMB);
 llprintf("Origin: %s\n", origin);
#endif
 llprintf("Violated policy rule.\n");
 print_rule(rule);
 llprintf("Program forced to exit.\n");
 _exit(1);
}

/**
 * Aborts in case no rule was found. -> whitelist
 */
void abort_no_rule_found(struct syscall_desc* syscall, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int is_sysenter)
{
 int i;
 int args[6];
#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
 char* origin = fbt_smalloc_pers(get_tld(), MAX_SIZE_SYMB);
#endif

 /* store arguments in args array */
 args[0] = arg1;
 args[1] = arg2;
 args[2] = arg3;
 args[3] = arg4;
 args[4] = arg5;
 args[5] = arg6;

 llprintf("Sandbox no policy rule found.\n");
 llprintf("Syscall: %s(", syscall->name);
 for(i = 0; i < syscall->nr_args; i++)
 {
  if(syscall->arguments[i] == ARG_TYPE_STR) llprintf("%s", (char*)args[i]);
  else llprintf("%d", args[i]);
  if(syscall->nr_args > 1 && i < syscall->nr_args-1) llprintf(", ");
 }
 llprintf(")\n");
#ifdef TRUST_SANDBOX_CALL_STACK_VERIFICATION
 get_call_origin(arg6, is_sysenter, origin, MAX_SIZE_SYMB);
 llprintf("Origin: %s\n", origin);
#endif
 llprintf("Program forced to exit.\n");
 _exit(1);
}

/**
 * Aborts in case of a deny. -> interactive mode
 */
void abort_from_deny(struct syscall_desc* syscall, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
 int i;
 int args[6];

 /* store arguments in args array */
 args[0] = arg1;
 args[1] = arg2;
 args[2] = arg3;
 args[3] = arg4;
 args[4] = arg5;
 args[5] = arg6;

 llprintf("Syscall denied.\n");
 llprintf("Syscall: %s(", syscall->name);
 for(i = 0; i < syscall->nr_args; i++)
 {
  if(syscall->arguments[i] == ARG_TYPE_STR) llprintf("%s", (char*)args[i]);
  else llprintf("%d", args[i]);
  if(syscall->nr_args > 1 && i < syscall->nr_args-1) llprintf(", ");
 }
 llprintf(")\n");
 llprintf("Program forced to exit.\n");
 _exit(1);
}

/**
 * Verifies an int argument according to the policy value for the specific argument.
 * @return 0 if the argument matches, -1 otherwise
 */
int verify_arg_int(char* str, int arg, struct syscall_desc* syscall, struct policy_rule* policy)
{
 int str_to_int;

 if(strcmp(str, WILDCARD) == 0) return 0;

 str_to_int = atoi(str);

 if(str_to_int == arg) return 0;

 return -1;
}

/**
 * Verifies a pointer argument according to the policy value for the specific argument.
 * @return 0 if the argument matches, -1 otherwise
 */
int verify_arg_ptr(char* str, int arg, struct syscall_desc* syscall, struct policy_rule* policy)
{
 if(strcmp(str, WILDCARD) == 0) return 0;
 if(strcmp(str, NULL_STR) == 0 && arg == 0) return 0;
 return -1;
}

/**
 * Verifies a string argument according to the policy value for the specific argument.
 * The string pattern matching algorithm is very primitive and not absolutely correct!
 * @return 0 if the argument matches, -1 otherwise
 */
int verify_arg_string(char* str, int arg, struct syscall_desc* syscall, struct policy_rule* policy)
{
 int i, j;
 char* strarg = (char*)arg;

 if(strcmp(str, WILDCARD) == 0) return 0;

 /* match policy rule string with syscall string -> compare char by char except for '*' that stands for a sequence of arbitrary chars */
 j = 0; /* index for strarg */
 for(i = 0; i < strlen(str); i++) /* iterate through policy string */
 {
  if(str[i] == WILDCARD_CHAR) /* if wildcard character encountered */
  {

   if(i == strlen(str)-1) return 0; /* last character is a '*' */
   else /* move on till character after '*' matches a character in strarg*/
   {
    while(j < strlen(strarg))
    {
     if(str[i+1] == strarg[j]) break;
     j++;
    }
   }

  }
  else /* if normal character encountered */
  {
   if(j == strlen(strarg)) return -1; /* if end of argument reached, return -1 */
   if(str[i] != strarg[j]) return -1; /* if characeters don't match, return -1 */
   j++;
  }
 }

 if(j != strlen(strarg)) return -1; /* if not all characters of argument checked, return -1 */

 return 0;
}

/**
 * Verifies an undef argument according to the policy value for the specific argument.
 * @return 0 if the argument matches, -1 otherwise
 */
int verify_arg_undef(char* str, int arg, struct syscall_desc* syscall, struct policy_rule* policy)
{
 if(strcmp(str, WILDCARD) == 0) return 0;
 return -1;
}

/**
 * Verifies a void argument according to the policy value for the specific argument.
 * This should never be called -> remove ?.
 * @return 0 if the argument matches, -1 otherwise
 */
int verify_arg_void(char* str, int arg, struct syscall_desc* syscall, struct policy_rule* policy)
{
 if(strcmp(str, WILDCARD) == 0) return 0;
 return -1;
}

/**
 * Prints the rules table to the console.
 * -> for debugging
 */
void print_rules()
{
 int i, j, l;
 struct policy_rule *rule;
 llprintf("*** SANDBOX RULES - BEGIN ***\n");
    for(i = 0; i < NR_SYSCALLS; i++)
    {
        if(rules[i] != NULL)
        {
         rule = rules[i];
         j = 0;
         while(rule != NULL)
         {
          llprintf("Rule syscall: %d / list element: %d", i, j);
          llprintf(" | %s", rule->syscall->name);
          for(l = 0; l < rule->syscall->nr_args; l++)
          {
           llprintf(" | arg %d = %s", l, rule->arg_rules[l]);
          }
          if(rule->action == ACTION_RETURN) llprintf(" | action = return(%s)\n", rule->ret_string);
          else if(rule->action == ACTION_DENY) llprintf(" | action = deny\n");
          else if(rule->action == ACTION_ALLOW) llprintf(" | action = allow\n");
          rule = rule->next;
          j++;
         }
        }
    }
 llprintf("*** SANDBOX RULES - END ***\n");
}

/**
 * Prints the specified rule.
 */
void print_rule(struct policy_rule *rule)
{
 int l;
         if(rule != NULL)
         {
          llprintf("Rule: %s", rule->syscall->name);
          for(l = 0; l < rule->syscall->nr_args; l++)
          {
           llprintf(" | arg %d = %s", l, rule->arg_rules[l]);
          }
          if(rule->action == ACTION_RETURN) llprintf(" | action = return(%s)\n", rule->ret_string);
          else if(rule->action == ACTION_DENY) llprintf(" | action = deny\n");
          else if(rule->action == ACTION_ALLOW) llprintf(" | action = allow\n");
         }
}

/**
 * Logs a syscall to the log file.
 * WARNING: Does not close it at the end -> TO FIX
 */
int log_syscall(struct syscall_desc* syscall, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int is_sysenter, char* origin)
{
 int count, i;
 char buffer[256];
 int args[6];

 /* store arguments in args array */
 args[0] = arg1;
 args[1] = arg2;
 args[2] = arg3;
 args[3] = arg4;
 args[4] = arg5;
 args[5] = arg6;

 count = 0;

 /* if not already opened -> open file */
 if(logfd == 0)
 {
  if((logfd=open(LOG_FILE, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR )) == -1)
  {
   llprintf("Could not open logfile.\n");
   return -1;
  }
 }

 /* "syscall(" */
 if(strlen(syscall->name)+1 > 256) return -1;
 strncpy(buffer, syscall->name, strlen(syscall->name));
 buffer[strlen(syscall->name)] = '('; 
 count = strlen(syscall->name)+1;

 /* write to file */
 if((write(logfd, &buffer, count)) == -1)
 {
  llprintf("Could not write to logfile.\n");
  close(logfd);
  return -1;
 }

 /* arg1, arg2, ... -> iterate through all arguments */
 for(i = 0; i < syscall->nr_args; i++)
 {
   /* check for argument type and write value to buffer */
   if(syscall->arguments[i] == ARG_TYPE_INT)
   {
#ifdef TRUST_SANDBOX_LOG_NOARGS
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#else
    snllprintd(buffer, 256, args[i]);
    count = strlen(buffer);
#endif
   }
   else if(syscall->arguments[i] == ARG_TYPE_PTR)
   {
#ifdef TRUST_SANDBOX_LOG_NOARGS
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#else
    if(args[i] == 0) strncpy(buffer, NULL_STR, strlen(NULL_STR)+1);
    else  strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#endif
   }
   else if(syscall->arguments[i] == ARG_TYPE_STR)
   {
#ifdef TRUST_SANDBOX_LOG_NOARGS
    strncpy(buffer+1, WILDCARD, strlen(WILDCARD)+1+2);
    buffer[0] = STRING_DELIMITER;
    buffer[2] = STRING_DELIMITER;
    buffer[3] = 0x00;
    count = strlen(buffer);
#else
    if(strlen((char*)args[i])+2 > 256) return -1;
    strncpy(buffer+1, (char*)args[i], strlen((char*)args[i]));
    count = strlen((char*)args[i])+2;
    buffer[0] = STRING_DELIMITER;
    buffer[strlen((char*)args[i])+1] = STRING_DELIMITER;
#endif
   }
   else if(syscall->arguments[i] == ARG_TYPE_UNDEF)
   {
#ifdef TRUST_SANDBOX_LOG_NOARGS
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#else
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#endif
   }
   else if(syscall->arguments[i] == ARG_TYPE_VOID)
   {
#ifdef TRUST_SANDBOX_LOG_NOARGS
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#else
    strncpy(buffer, WILDCARD, strlen(WILDCARD)+1);
    count = strlen(buffer);
#endif
   }

   /* write to file */
   if((write(logfd, &buffer, count)) == -1)
   {
    llprintf("Could not write to logfile.\n");
    close(logfd);
    return -1;
   }

   /* "," */
   if(syscall->nr_args > 1 && i < syscall->nr_args-1)
   {
    strncpy(buffer, ARGS_DELIMITER, strlen(ARGS_DELIMITER)+1);
    count = strlen(buffer);

    /* write to file */
    if((write(logfd, &buffer, count)) == -1)
    {
     llprintf("Could not write to logfile.\n");
     close(logfd);
     return -1;
    }
   }

 }

 /* "):allow\n" */
#ifdef TRUST_SANDBOX_LOG_CALL_ORIGIN
 strncpy(buffer, "):allow", 7);
 count = 7;
#else
 strncpy(buffer, "):allow\n", 8);
 count = 8;
#endif

 /* write to file */
 if((write(logfd, &buffer, count)) == -1)
 {
  llprintf("Could not write to logfile.\n");
  close(logfd);
  return -1;
 }

#ifdef TRUST_SANDBOX_LOG_CALL_ORIGIN
 if(origin != NULL)
 {
	/* ":origin" */
	strncpy(buffer, ":", 1);
	strncpy(buffer+1, origin, strlen(origin));
	strncpy(buffer+1+strlen(origin), "\n", 1);
	count = 2 + strlen(origin);
	
	/* write to file */
	if((write(logfd, &buffer, count)) == -1)
	{
	 llprintf("Could not write to logfile.\n");
	 close(logfd);
	 return -1;
	}
 }
#endif

 return 0;
}

/**
 * Closes the log file.
 */
void log_close()
{
 close(logfd);
}

/******************************************/
/* sandbox interactive mode IPC functions */
/******************************************/
/**
 * Initializes the sandbox interactive mode IPC.
 * Sends the specified sandbox_init to the user interface and returns the user interface's answer.
 */
struct sandbox_init init_sandbox_ipc(struct sandbox_init buf)
{
 int pid;

 pid = (int)getpid(); /* gets the pid for generating the message queue key */

 buf.mtype = MSG_TYPE_INIT;

 /* generate the key */
 if((key = ftok(IPC_KEY, pid)) == -1)
 {
  perror("ftok");
  exit(1);
 }

 /* get the message queue id */
 if((msqid = msgget(key, 0644 | IPC_CREAT)) == -1)
 {
  perror("msgget");
  exit(1);
 }

 /* sned the initialization message */
 if(msgsnd(msqid, &buf, sizeof(buf), 0) == -1)
 {
  perror("msgsnd");
  exit(1);
 }

 /* receive the initialization reply */
 if(msgrcv(msqid, &buf, sizeof(buf), MSG_TYPE_INIT_REP, 0) == -1)
 {
  perror("msgrcv");
  exit(1);
 }

 return buf;
}

/**
 * Notifies the user interface about a new syscall event and receives the user interface's reply.
 */
int notify_UI(struct sandbox_request* sreq, struct sandbox_reply* srep)
{
 sreq->mtype = MSG_TYPE_REQ;

 /* send notification */
 if(msgsnd(msqid, sreq, sizeof(struct sandbox_request), 0) == -1)
 {
  perror("msgsnd");
  exit(1);
 }

 /* receive reply */
 if(msgrcv(msqid, srep, sizeof(struct sandbox_reply), MSG_TYPE_REP, 0) == -1)
 {
  perror("msgrcv");
  exit(1);
 }

 return 0;
}

/**
 * Frees the ipc resources.
 */
int free_ipc()
{
 /* delete message queue */
 if (msgctl(msqid, IPC_RMID, NULL) == -1)
 {
  perror("msgctl");
  exit(1);
 }

 return 0;
}
