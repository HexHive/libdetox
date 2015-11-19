#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fbt_policy_parser.h"
#include "fbt_llio.h"
#include "fbt_mem_alloc.h"
#include "fbt_mem_protection.h"
#include "libfastbt.h"

static struct policy_rule *new_rule; /* pointer for new rule */
static struct parser_node *parser_root; /* the root parser node */
static struct policy_rule** rules_table; /* the rules table */
static int* policy_mode_ptr; /* pointer to policy mode */
static int linenr; /* the current line number parsed */
static int set_call_origin; /* indicates if the call origin has to be stored or not */
/**
 * Load policy file and generate policy rules datastructure.
 * @return 0 if policy was loaded successfully, in case of failure -1
 */
int fbt_load_policy(struct syscall_desc** syscalls, struct policy_rule** rules, int* pmode)
{
  int i, j, len, varlen;
  int fd;
  int parse_error;
  int node_found;
  int childmatch;
  char *filemem;
  char *addr;
  char *varfilemem;
  char *varstr;
  struct stat attr;
  static struct parser_node *current_node;
  static struct parser_node *down_node;
  static struct parser_node *tmp;

  policy_mode_ptr = pmode; /* set policy mode pointer */

  rules_table = rules; /* set rules table */

  /* clear policy and free memory */
  for(i = 0; i < NR_SYSCALLS; i++)
  {
/* no free exists for fbt_smalloc_pers */
/*
        if(rules[i] != NULL)
        {
		for(j = 0; j < rules[i]->syscall->nr_args; j++)
		{
			free(rules[i]->arg_rules[j]);
			free(rules[i]->arg_rules);
		}
		if(rules[i]->ret_string != NULL) free(rules[i]->ret_string);
		free(rules[i]);
        }
*/
	rules[i] = NULL;
  }

  /* generate parsetree */
  if(fbt_create_parsetree(syscalls) < 0) return -1;
  //llprintf("parsetree created successfully!\n");

  /* open specifiaction file */
  if((fd = open(POLICY_FILE, O_RDONLY)) < 0) return -1;

  /* get stats */
  if(fstat(fd, &attr) == -1) return -1;

  /* map file to memory */
  filemem = (char*)mmap(0, attr.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if(filemem == ((caddr_t)-1)) return -1;
  addr = filemem;

  /* close file */
  close(fd);

  /* init variables */
  parse_error = 0;
  linenr = 1;
  current_node = parser_root;
  set_call_origin = 0;

  /* loop through mapped memory till the end, one iteration per line */
  while(current_node != NULL && (filemem-addr) < attr.st_size && !parse_error)
  {

    /***************************************** parse node SYMBOL ***************************************************/
    if(current_node->type == PARSE_NODE_TYPE_SYMBOL)
    {
     len = strlen(current_node->symbol);
     if(strmatch(filemem, current_node->symbol))
     {
        /* in case the nodes down the tree are just symbols, verify that at least one matches the location after this symbol */
	tmp = current_node->down;
	childmatch = 0;
	while(tmp != NULL && childmatch == 0)
	{
	  if((tmp->type == PARSE_NODE_TYPE_SYMBOL && strmatch(filemem+len, tmp->symbol)) || tmp->type != PARSE_NODE_TYPE_SYMBOL) childmatch++;
	  tmp = tmp->next;
	}

	if(childmatch)
	{
		if(current_node->parse_event != NULL)
		{
			if(current_node->parse_event(current_node->arg1, current_node->arg2, current_node->arg3) < 0) //return -1;
			{
				parse_error = 1;
				break;
			}
		}
		current_node = current_node->down;
		filemem += len;
	}
	else current_node = current_node->next;
     }
     else current_node = current_node->next;

    } /***************************************** parse node STR ***************************************************/
    else if(current_node->type == PARSE_NODE_TYPE_STR)
    {
     varlen = 0;
     node_found = 0;
     while((filemem+varlen-addr) < attr.st_size && !node_found && !parse_error)
     {
	varfilemem = filemem + varlen;
	down_node = current_node->down;
	while(down_node != NULL && !node_found)
	{
		len = strlen(down_node->symbol);
		if(strmatch(varfilemem, down_node->symbol))
		{
		 if(current_node->parse_event != NULL)
		 {
			varstr = fbt_smalloc_pers(get_tld(), varlen + 1);
			strncpy(varstr, filemem, varlen);
			varstr[varlen] = 0x00;
			if(current_node->parse_event(current_node->arg1, varstr, current_node->arg3) < 0) //return -1;
			{
				parse_error = 1;
				break;
			}
		 }
		 current_node = down_node->down;
		 node_found = 1;
		 filemem = filemem + varlen + len;
		}
		else down_node = down_node->next;
	}
	varlen++;
     }
     /* if node was not found and end of file is reached a parse error occured */
     if(!node_found && (filemem+varlen-addr) >= attr.st_size) parse_error = 1;

    } /*************************************** parse node COMMENT ************************************************/
    else if(current_node->type == PARSE_NODE_TYPE_COMMENT)
    {
     len = strlen(current_node->symbol);
     if(strmatch(filemem, current_node->symbol))
     {
	current_node = current_node->down;
	filemem += len;
        while(*filemem != NEWLINE_CHAR && (filemem-addr) < attr.st_size) filemem++; /* skip comment line */
	filemem++;
     }
     else current_node = current_node->next;

    } /***************************************** invalid type ***************************************************/
    else parse_error = 1;

  } /* while loop end */

  /* check if an error occured or a NULL node was reached */
  if(current_node == NULL || parse_error == 1)
  {
     llprintf("Syntax error in policy file @ line %d.\n", linenr);
     if(filemem-addr > 10 && filemem-addr < (attr.st_size-10))
     {
      llprintf("Around:\n");
      filemem = filemem-10;
      for(i = 0; i < 20; i++) printf("%c", filemem[i]);
      llprintf("\n");
     }

     /* unmap file */
     munmap(addr, attr.st_size);

     return -1;
  }

  /* unmap file */
  munmap(addr, attr.st_size);

  return 0;
}

/**
 * Matches a memory location with a string byte by byte.
 * @return 1 if the byte sequence is equal, otherwise 0
 */
int strmatch(char* mem, char* str)
{
 int i, len;
 int match = 1;

 len = strlen(str);
 for(i = 0; i < len; i++)
 {
  if(mem[i] != str[i])
  {
   match = 0;
   break;
  }
 }

 return match;
}

/**
 * Validates an int string. Just strings containing digits or at most one '-' character at the beginning are allowed.
 * @return 1 if the string is valid, otherwise 0
 */
int is_valid_int(char* str, struct syscall_desc* syscall)
{
 int i;

 if(strcmp(str, WILDCARD) == 0) return 1; /* allow wildcard */

 i = 0;

 if(strlen(str) > 1) /* check if first character is a digit or a '-' */
 {
  if(!isdigit(str[0]) && str[0] != '-') return 0;
  else i++;
 }

 for(; i < strlen(str); i++) /* check if all remaining characters are digits */
 {
  if(!isdigit(str[i])) return 0;
 }

 return 1;
}

/**
 * Validates an ptr string. Just this two strings are allowed: "*" or "NULL".
 * @return 1 if the string is valid, otherwise 0
 */
int is_valid_ptr(char* str, struct syscall_desc* syscall)
{
 if(strcmp(str, WILDCARD) == 0) return 1; /* allow wildcard */
 if(strcmp(str, NULL_STR) == 0) return 1; /* check for null string */

 return 0;
}

/**
 * Validates a string. In most cases a filename (?).
 * @return 1 if the string is valid, otherwise 0
 */
int is_valid_string(char* str, struct syscall_desc* syscall)
{
 int i, j;
 char* invalid = INVALID_CHARS;

 if(strlen(str) == 0) return 0; /* no empty strings allowed */
 if(strcmp(str, WILDCARD) == 0) return 1; /* allow wildcard */

 for(i = 0; i < strlen(str); i++) /* check if characters are valid */
 {
  //if(iscntrl(str[i]) || isgraph(str[i]) || isspace(str[i])) return 0;
  for(j = 0; j < strlen(invalid); j++) if(str[i] == invalid[j])
  {
   llprintf("String %s contains invalid character: %c\n", str, str[i]);
   return 0;
  }
 }

 return 1;
}

/**
 * Validates an undef string. Just allow "*".
 * @return 1 if the string is valid, otherwise 0
 */
int is_valid_undef(char* str, struct syscall_desc* syscall)
{
 if(strcmp(str, WILDCARD) == 0) return 1; /* allow wildcard */
 return 0;
}

/**
 * Validates a void string. Nothing allowed.
 * @return 1 if the string is valid, otherwise 0
 */
int is_valid_void(char* str, struct syscall_desc* syscall)
{
 return 0;
}

/**
 * Gets called whenever a new rule gets parsed.
 * @param strarg the name of the syscall
 * @param arg a pointer to the syscall descriptor
 */
int event_new_rule(int nrarg, char* strarg, void* arg)
{
 new_rule = fbt_smalloc_pers(get_tld(), sizeof(struct policy_rule));

 new_rule->syscall = (struct syscall_desc*)arg;

 if(new_rule->syscall->nr_args > 0) new_rule->arg_rules = fbt_smalloc_pers(get_tld(), sizeof(char*)*new_rule->syscall->nr_args);
 else new_rule->arg_rules = NULL;

 new_rule->call_origin = NULL;

 return 0;
}

/**
 * Gets called whenever a new parameter gets parsed.
 * @param nrarg the index of the parameter
 * @param arg a pointer to the syscall descriptor
 * @param strarg the parsed parameter as string
 */
int event_add_parameter(int nrarg, char* strarg, void* arg)
{
 struct syscall_desc* syscall = (struct syscall_desc*)arg;

 if(nrarg < 0 || nrarg >= syscall->nr_args) return -1;

 /* check if parameter is valid resp. if it contains invalid characters */
 if(syscall->arguments[nrarg] == ARG_TYPE_INT)
 {
  if(!is_valid_int(strarg, syscall))
  {
   llprintf("Integer argument is invalid.\n");
   return -1;
  }
 }
 else if(syscall->arguments[nrarg] == ARG_TYPE_PTR)
 {
  if(!is_valid_ptr(strarg, syscall))
  {
   llprintf("Pointer argument is invalid.\n");
   return -1;
  }
 }
 else if(syscall->arguments[nrarg] == ARG_TYPE_STR)
 {
  if(!is_valid_string(strarg, syscall))
  {
   llprintf("String argument is invalid.\n");
   return -1;
  }
 }
 else if(syscall->arguments[nrarg] == ARG_TYPE_UNDEF)
 {
  if(!is_valid_undef(strarg, syscall))
  {
   llprintf("Undef argument is invalid.\n");
   return -1;
  }
 }
 else if(syscall->arguments[nrarg] == ARG_TYPE_VOID)
 {
  if(!is_valid_void(strarg, syscall))
  {
   llprintf("Void argument is invalid.\n");
   return -1;
  }
 }
 else return -1;

 new_rule->arg_rules[nrarg] = strarg;

 return 0;
}

/**
 * Gets called whenever an action gets parsed.
 */
int event_action(int nrarg, char* strarg, void* arg)
{
 static struct policy_rule *tmp_rule;

 /* mode blacklist -> ignore action "allow" */
 if(*policy_mode_ptr == BLACKLIST_MODE && nrarg == ACTION_ALLOW)
 {
/* no free exists for fbt_smalloc_pers */
/*
  free(new_rule->arg_rules);
  free(new_rule);
*/
  set_call_origin = 0;
  return 0;
 }
 /* mode whitelist -> ignore action "deny" */
 if(*policy_mode_ptr == WHITELIST_MODE && nrarg == ACTION_DENY)
 {
/* no free exists for fbt_smalloc_pers */
/*
  free(new_rule->arg_rules);
  free(new_rule);
*/
  set_call_origin = 0;
  return 0;
 }

 new_rule->action = nrarg;
 if(nrarg == ACTION_RETURN)
 {
  new_rule->ret_string = strarg;
  new_rule->retvalue = atoi(strarg);
 }
 else new_rule->ret_string = NULL;

 /* rule created, append to rules table */
 tmp_rule = rules_table[new_rule->syscall->nr];
 if(tmp_rule == NULL)
 {
  rules_table[new_rule->syscall->nr] = new_rule;
 }
 else
 {
  while(tmp_rule->next != NULL) tmp_rule = tmp_rule->next;
  tmp_rule->next = new_rule;
 }

 /* set next rule to NULL */
 new_rule->next = NULL;

 /* indicate to set origin if parsed */
 set_call_origin = 1;

 return 0;
}

/**
 * Gets called whenever a mode gets parsed.
 * @param nrarg the policy mode number, 1 for blacklist, 2 for whitelist
 */
int event_mode(int nrarg, char* strarg, void* arg)
{
 *policy_mode_ptr = nrarg;
 return 0;
}

/**
 * Gets called whenever a newline gets parsed.
 */
int event_newline(int nrarg, char* strarg, void* arg)
{
 linenr++;
 return 0;
}

/**
 * Gets called whenever an object name for the callstack validation gets parsed.
 */
int event_callstack(int nrarg, char* strarg, void* arg)
{
 /* if rule was created, set the call origin */
 if(set_call_origin)
 {
   /* if call origin is * set to NULL */
   if(strcmp(strarg, WILDCARD) == 0)
   {
    new_rule->call_origin = NULL;
   }
   else
   {
    new_rule->call_origin = strarg;
   }
 }
 else
 {
   new_rule->call_origin = NULL;
 }

 return 0;
}

/**
 * Creates a new node for the parsetree.
 */
struct parser_node* fbt_create_node(int type, char* symbol, parse_event_fptr_t parse_event, int arg1, char* arg2, void* arg3, struct parser_node *next, struct parser_node *down)
{
 struct parser_node *tmp_node;

 /* allocate memory */
 tmp_node = fbt_smalloc_pers(get_tld(), sizeof(struct parser_node));
 if(tmp_node == NULL) return NULL;

 /* create node */
 tmp_node->type = type;

 if(symbol != NULL)
 {
  tmp_node->symbol = fbt_smalloc_pers(get_tld(), strlen(symbol) + 1);
  strncpy(tmp_node->symbol, symbol, strlen(symbol));
  tmp_node->symbol[strlen(symbol)] = 0x00; /* terminate string with nullbyte */
 }
 else tmp_node->symbol = NULL;

 tmp_node->parse_event = parse_event; /* no parse event */
 tmp_node->next = next;
 tmp_node->down = down;
 tmp_node->arg1 = arg1;
 tmp_node->arg2 = arg2;
 tmp_node->arg3 = arg3;

 return tmp_node;
}

/**
 * Creates a new node for the parsetree and appends it to the parent.
 */
struct parser_node* fbt_append_child(struct parser_node *parent, int type, char* symbol, parse_event_fptr_t parse_event, int arg1, char* arg2, void* arg3, struct parser_node *next, struct parser_node *down)
{
 struct parser_node *tmp_node;

 tmp_node = fbt_create_node(type, symbol, parse_event, arg1, arg2, arg3, next, down);

 /* append child */
 parent->down = tmp_node;

 return tmp_node;
}

/**
 * Creates a new node for the parsetree and appends it to the successor.
 */
struct parser_node* fbt_append_successor(struct parser_node *predecessor, int type, char* symbol, parse_event_fptr_t parse_event, int arg1, char* arg2, void* arg3, struct parser_node *next, struct parser_node *down)
{
 struct parser_node *tmp_node;

 tmp_node = fbt_create_node(type, symbol, parse_event, arg1, arg2, arg3, next, down);

 /* append successor */
 predecessor->next = tmp_node;

 return tmp_node;
}

/**
 * Frees the parsetree.
 * IMPLEMENT ?
 */
int fbt_free_parsetree()
{
 return 0;
}

/**
 * Creates the parsetree.
 */
int fbt_create_parsetree(struct syscall_desc** syscalls)
{
 struct parser_node *tmp_node;
 struct parser_node *tmp_parent;
 struct parser_node *deny_node;
 struct parser_node *allow_node;
 struct parser_node *return_node;
 int i, j;

 /* root -> "mode" - " " - "/n" - "syscallsymbol1" - "syscallsymbol2" - ... - "syscallsymbolN" */

 /*********************************** "mode" *********************************************/
 tmp_node = fbt_create_node(PARSE_NODE_TYPE_SYMBOL, MODE_SYMBOL, NULL, 0, NULL, NULL, NULL, NULL);

 /* assign to root */
 parser_root = tmp_node;

 /* NEWLINE */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, NEWLINE, event_newline, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* reconnect NEWLINE node with root */
 tmp_node->down = parser_root;

 /* ' ' */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* reconnect WHITESPACE node with root */
 tmp_node->down = parser_root;

 /* '#' */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_COMMENT, COMMENT, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* reconnect COMMENT node with root */
 tmp_node->down = parser_root;

 /* everything following "mode" */

 /* ' ' */
 tmp_node = fbt_append_child(parser_root, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;
 tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

 /* DELIMITER */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, DELIMITER, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* ' ' */
 tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;
 tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

  /* MODE1 -> event policy mode */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, MODE1, event_mode, BLACKLIST_MODE, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* backup mode1 node */
 tmp_parent = tmp_node;

  /* MODE2 -> event policy mode */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, MODE2, event_mode, WHITELIST_MODE, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

  /* ' ' */
 tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;
 tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

 /* assign child to mode1 node */
 tmp_parent->down = tmp_node;

  /* NEWLINE */
 tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, NEWLINE, event_newline, 0, NULL, NULL, NULL, NULL);
 if(tmp_node == NULL) return -1;

 /* reconnect NEWLINE node with root */
 tmp_node->down = parser_root;



 /*********************************** "syscallsymbol"s ******************************************/
 for(i = 0; i < NR_SYSCALLS; i++)
 {
  tmp_node = parser_root;
  while(tmp_node->next != NULL) tmp_node = tmp_node->next;

  if(syscalls[i] != NULL)
  {
    /* syscall name */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, syscalls[i]->name, event_new_rule, 0, NULL, (void*)syscalls[i], NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* parenthesis "(" */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, LEFT_PARENTHESIS, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    for(j = 0; j < syscalls[i]->nr_args; j++)
    {

	/* ' ' */
	tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
	if(tmp_node == NULL) return -1;
	tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */
	
	if(syscalls[i]->arguments[j] == ARG_TYPE_STR)
	{
		/* " */
		tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, STR_DEL, NULL, 0, NULL, NULL, NULL, NULL);
		if(tmp_node == NULL) return -1;
	
		/* * wildcard, a sequence of characters */
		tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_STR, NULL, event_add_parameter, j, NULL, (void*)syscalls[i], NULL, NULL);
		if(tmp_node == NULL) return -1;
		
		/* " */
		tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, STR_DEL, NULL, 0, NULL, NULL, NULL, NULL);
		if(tmp_node == NULL) return -1;
	}
	else
	{
		/* * wildcard, a sequence of characters */
		tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_STR, NULL, event_add_parameter, j, NULL, (void*)syscalls[i], NULL, NULL);
		if(tmp_node == NULL) return -1;
		
		/* ' ' */
		tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
		if(tmp_node == NULL) return -1;
		tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */
	}
	
	if(syscalls[i]->nr_args > 1 && j < (syscalls[i]->nr_args-1))
	{
	
		if(syscalls[i]->arguments[j] == ARG_TYPE_STR)
		{
			/* ' ' */
			tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
			if(tmp_node == NULL) return -1;
			tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */
		}

		/* ARGS_DELIMITER */
		tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, ARGS_DELIMITER, NULL, 0, NULL, NULL, NULL, NULL);
		if(tmp_node == NULL) return -1;

	}

    }

    /* parenthesis ")" */
    if(syscalls[i]->nr_args == 0 || (syscalls[i]->arguments[j-1]) == ARG_TYPE_STR) tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, RIGHT_PARENTHESIS, NULL, 0, NULL, NULL, NULL, NULL);
    else  tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, RIGHT_PARENTHESIS, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;



    /* ' ' */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

    /* DELIMITER */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, DELIMITER, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* ' ' */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

    /* deny */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, ACTION_DENY_STR, event_action, ACTION_DENY, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    deny_node = tmp_node;

    /* allow */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, ACTION_ALLOW_STR, event_action, ACTION_ALLOW, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    allow_node = tmp_node;

    /* return() */
    /* return */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, ACTION_RETURN_STR, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* parenthesis "(" */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, LEFT_PARENTHESIS, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* ' ' */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

     /* * wildcard, a sequence of characters */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_STR, NULL, event_action, ACTION_RETURN, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* ' ' */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

    /* parenthesis ")" */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, RIGHT_PARENTHESIS, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    return_node = tmp_node;

     /* ' ' */
    tmp_node = fbt_create_node(PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

    /* assign childs to action nodes */
    deny_node->down = tmp_node;
    allow_node->down = tmp_node;
    return_node->down = tmp_node;

     /* NEWLINE */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, NEWLINE, event_newline, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* reconnect NEWLINE node with root */
    tmp_node->down = parser_root;

    /* call stack support :/lib/XXX */

    /* DELIMITER */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_SYMBOL, DELIMITER, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* ' ' */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, WHITESPACE, NULL, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;
    tmp_node->down = tmp_node; /* this node points to itself, because more whitespaces may follow */

     /* * wildcard, a sequence of characters */
    tmp_node = fbt_append_successor(tmp_node, PARSE_NODE_TYPE_STR, NULL, event_callstack, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

     /* NEWLINE */
    tmp_node = fbt_append_child(tmp_node, PARSE_NODE_TYPE_SYMBOL, NEWLINE, event_newline, 0, NULL, NULL, NULL, NULL);
    if(tmp_node == NULL) return -1;

    /* reconnect NEWLINE node with root */
    tmp_node->down = parser_root;

  } /* if syscalls[i] != NULL */
 } /* for() */

 return 0;
}

/**
 * Checks if the policy file already exists.
 */
int policy_exists()
{
 struct stat st;
 if(stat(POLICY_FILE, &st) == 0) return 1;
 else return 0;
}
