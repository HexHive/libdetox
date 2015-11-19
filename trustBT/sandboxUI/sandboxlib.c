/**
 * sandboxlib.c - The sandboxlib functions.
 */

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

#include "sandboxlib.h"

struct syscall_desc* syscalls[NR_SYSCALLS + 1]; /* the syscalls descriptions, stored in an array and indexed by the systemcall nr */
struct policy_rule *rules[NR_SYSCALLS + 1]; /* the policy rules, indexed by the systemcall nr, each element is the first element of a linked list of policy rules */
int policy_mode; /* 1: blacklist, 2: whitelist */
int msqid; /* the IPC message queue ID */
key_t key; /* the IPC message queue key */

/**
 * Initialize sandbox IPC. Get first message from queue.
 */
int init_sandbox_ipc(int pid, struct sandbox_init* buf)
{
 int i;

 /* init policy rules table */
 for(i = 0; i < NR_SYSCALLS; i++)
 {
	rules[i] = NULL;
 }

 /* generate key */
 if((key = ftok(IPC_KEY, pid)) == -1) return -1;

 /* get message queue identifier */
 if((msqid = msgget(key, 0644)) == -1) return -1;

 /* receive first message from sandbox */
 if(msgrcv(msqid, buf, sizeof(struct sandbox_init), MSG_TYPE_INIT, 0) == -1) return -1;

 return 0;
}

/**
 * Reply to sandbox initialization.
 */
int init_sandbox_ipc_reply(struct sandbox_init* buf)
{
 /* set message type */
 buf->mtype = MSG_TYPE_INIT_REP;

 /* send message */
 if(msgsnd(msqid, buf, sizeof(struct sandbox_init), 0) == -1) return -1;

 return 0;
}

/**
 * Wait for a syscall event.
 */
int wait_for_syscall(struct sandbox_request* buf)
{
 /* receive a message of type request */
 if(msgrcv(msqid, buf, sizeof(struct sandbox_request), MSG_TYPE_REQ, 0) == -1) return -1;

 return 0;
}

/**
 * Reply to a syscall event request resp. notify sandbox about chosen action.
 */
int notify_sandbox(struct sandbox_reply* buf)
{
 /* set message type */
 buf->mtype = MSG_TYPE_REP;

 /* send message */
 if(msgsnd(msqid, buf, sizeof(struct sandbox_reply), 0) == -1) return -1;

 return 0;
}

/**
 * Add a new policy rule to the policy table.
 * This function creates a new policy rule from a sandbox_reply struct and the syscall number.
 */
int add_policy_rule(int syscallnr, struct sandbox_reply* srep)
{
 int i;
 struct policy_rule* tmp_rule;
 struct policy_rule* new_rule = (struct policy_rule*)malloc(sizeof(struct policy_rule)); /* allocate memory */

 if(new_rule == NULL) return -1;

 /* set syscall description */
 new_rule->syscall = syscalls[syscallnr];

 /* allocate arg_rules array */
 new_rule->arg_rules = malloc(sizeof(char*)*new_rule->syscall->nr_args);
 if(new_rule->arg_rules == NULL) return -1;

 /* allocate args memory */
 for(i = 0; i < new_rule->syscall->nr_args; i++)
 {
  new_rule->arg_rules[i] = malloc(strlen(srep->args[i])+1);
  if(new_rule->arg_rules[i] == NULL) return -1;
  strncpy(new_rule->arg_rules[i], srep->args[i], strlen(srep->args[i])+1);
 }

 /* set action */
 if(policy_mode == BLACKLIST)
 {
  new_rule->action = ACTION_DENY;
 }
 else if(policy_mode == WHITELIST)
 {
  new_rule->action = ACTION_ALLOW;
 }

 /* set return rule specific values */
 if(srep->action == ACTION_CREATE_RET)
 {
  new_rule->action = ACTION_RETURN;
  new_rule->ret_string = malloc(MAX_SIZE_SYMB);
  if(new_rule->ret_string == NULL) return -1;
  sprintf(new_rule->ret_string, "%d", srep->retvalue);
  new_rule->retvalue = srep->retvalue;
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

 new_rule->next = NULL;

 return 0;
}

/**
 * Write the policy table to a policy file.
 */
int write_policy(char* file)
{
 int count, i, j, fd, pexists;
 char buffer[MAX_SIZE_SYMB];
 struct syscall_desc* syscall;
 struct policy_rule* policy;
 struct stat st;

 count = 0;

 /* check if policy file exists */
 if(stat(file, &st) == 0) pexists = 1;
 else pexists = 0;

 /* open policy file */
 if((fd=open(file, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR )) == -1)
 {
  printf("Could not open policy file.\n");
  return -1;
 }

 /* write policy mode if policy file doesn't already exist */
 if(!pexists)
 {
  if(policy_mode == BLACKLIST) strncpy(buffer, "mode:blacklist\n", 15);
  else if(policy_mode == WHITELIST) strncpy(buffer, "mode:whitelist\n", 15);

  /* write to file */
  if((write(fd, &buffer, 15)) == -1)
  {
	printf("Could not write to policy file.\n");
	close(fd);
	return -1;
  }

 }

 /* for each rule */
 for(j = 0; j < NR_SYSCALLS; j++)
 {
   policy = rules[j];

   syscall = syscalls[j];

   while(policy != NULL) /**********************************************************/
   {

	/* "syscall(" */
	if(strlen(syscall->name)+1 > 256) return -1;
	strncpy(buffer, syscall->name, strlen(syscall->name));
	buffer[strlen(syscall->name)] = '('; 
	count = strlen(syscall->name)+1;
	
	/* write to file */
	if((write(fd, &buffer, count)) == -1)
	{
		printf("Could not write to policy file.\n");
		close(fd);
		return -1;
	}
	
	
	for(i = 0; i < syscall->nr_args; i++)
	{
	
		/* copy argument to buffer */
		if(syscall->arguments[i] == ARG_TYPE_STR)
		{
			if(strlen(policy->arg_rules[i])+2 > 256) return -1;
			strncpy(buffer+1, policy->arg_rules[i], strlen(policy->arg_rules[i]));
			count = strlen(policy->arg_rules[i])+2;
			buffer[0] = STRING_DELIMITER;
			buffer[strlen(policy->arg_rules[i])+1] = STRING_DELIMITER;
		}
		else
		{
			strncpy(buffer, policy->arg_rules[i], strlen(policy->arg_rules[i]));
			count = strlen(policy->arg_rules[i]);
		}
		
		/* write to file */
		if((write(fd, &buffer, count)) == -1)
		{
			printf("Could not write to policy file.\n");
			close(fd);
			return -1;
		}
		
		/* "," */
		if(syscall->nr_args > 1 && i < syscall->nr_args-1)
		{
			strncpy(buffer, ARGS_DELIMITER, strlen(ARGS_DELIMITER)+1);
			count = strlen(buffer);
			
			/* write to file */
			if((write(fd, &buffer, count)) == -1)
			{
				printf("Could not write to policy file.\n");
				close(fd);
				return -1;
			}
		}
	
	}

	/* write action deny*/
	if(policy->action == ACTION_DENY)
	{
		strncpy(buffer, "):deny\n", 7);
		count = 7;
	}
	/* write action allow */
	else if(policy->action == ACTION_ALLOW)
	{
		strncpy(buffer, "):allow\n", 8);
		count = 8;
	}
	/* write action return */
	else if(policy->action == ACTION_RETURN)
	{
		count = sprintf(buffer, "):return(%d)\n", policy->retvalue);
		if(count < 0)
		{
			close(fd);
			return -1;
		}
	}
	
	/* write to file */
	if((write(fd, &buffer, count)) == -1)
	{
		printf("Could not write to policy file.\n");
		close(fd);
		return -1;
	}

	/* go to next policy rule */
	policy = policy->next;

   }  /******************************************************************************/

 }

 /* close file */
 close(fd);

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
int load_syscall_specification(char* specfile)
{
  int fd, i;
  char *filemem;
  char *addr;
  char *tmpstr;
  struct stat attr;
  struct syscall_desc *el;

  char buf[MAX_SIZE_SYMB];
  int argsparsed, argscount;

  for(i = 0; i < NR_SYSCALLS; i++)
  {
	syscalls[i] = NULL;
  }

  /* open specifiaction file */
  if(specfile != NULL)
  {
   if((fd = open(specfile, O_RDONLY)) < 0) return -1;
  }
  else
  {
   if((fd = open(SYSCALLS_SPEC_FILE, O_RDONLY)) < 0) return -1;
  }

  /* get stats */
  if(fstat(fd, &attr) == -1) return -1;

  /* map file to memory */
  filemem = (char*)mmap(0, attr.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if(filemem == ((caddr_t)-1)) return -1;
  addr = filemem;

  /* close file */
  close(fd);

  /* loop through mapped memory till the end, one iteration per line */
  while((filemem-addr) < attr.st_size)
  {
    el = malloc(sizeof(struct syscall_desc)); /* allocate memory for a new syscall description element */
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
    el->name = malloc(strlen(buf) + 1); /* allocate name string memory */
    el->name[strlen(buf)] = 0x00;
    strncpy(el->name, buf, strlen(buf));

    /* ARG,... */
    if(*filemem != ')') /* if a first argument follows */
    {
	el->arguments = malloc(sizeof(int)*MAX_NR_ARGS);  /* allocate arguments array memory */
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
		else return -1; // not very nice
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
	else return -1; // not very nice
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
