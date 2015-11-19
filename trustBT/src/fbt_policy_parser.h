#ifndef FBT_POLICY_PARSER_H
#define FBT_POLICY_PARSER_H

#include "fbt_sandbox.h"

/* the policy specification file */
#define POLICY_FILE "/policy.tbt"

/* parse tree node types */
#define PARSE_NODE_TYPE_SYMBOL 1
#define PARSE_NODE_TYPE_STR 2
#define PARSE_NODE_TYPE_COMMENT 3

/* policy modes */
#define BLACKLIST_MODE 1
#define WHITELIST_MODE 2

/* some symbols */
#define MODE_SYMBOL "mode"
#define MODE1 "blacklist"
#define MODE2 "whitelist"
#define ARGS_DELIMITER ","
#define DELIMITER ":"
#define LEFT_PARENTHESIS "("
#define RIGHT_PARENTHESIS ")"
#define WHITESPACE " "
#define NEWLINE "\n"
#define NEWLINE_CHAR '\n'
#define COMMENT "#"

#define WILDCARD "*"
#define WILDCARD_CHAR '*'
#define STRING_DELIMITER '"'
#define STR_DEL "\""

#define INVALID_CHARS ",:()\t\n"
#define NULL_STR "null"

/* action types */
#define ACTION_DENY 1
#define ACTION_ALLOW 2
#define ACTION_RETURN 3

#define ACTION_CREATE 4
#define ACTION_CREATE_RET 5

#define ACTION_DENY_STR "deny"
#define ACTION_ALLOW_STR "allow"
#define ACTION_RETURN_STR "return"

typedef int (*parse_event_fptr_t)(int, char*, void*); /* parse event handler function typedef */

/**
 * The parse tree node struct.
 */
struct parser_node {
    int type; /* node type */
    char* symbol; /* symbol string */
    parse_event_fptr_t parse_event; /* event handler */
    int arg1; /* int arg */
    char* arg2; /* char* arg */
    void* arg3; /* void* arg */
    struct parser_node *next; /* pointer to next node */
    struct parser_node *down; /* pointer to next level's node */
};

int fbt_create_parsetree(struct syscall_desc**); /* creates the parse tree */
int fbt_load_policy(struct syscall_desc**, struct policy_rule**, int*); /* loads a policy file */
int fbt_free_parsetree(); /* frees the parsetree -> not implemented */

struct parser_node* fbt_create_node(int, char*, parse_event_fptr_t, int, char*, void*, struct parser_node*, struct parser_node*); /* creates a parse node */
struct parser_node* fbt_append_child(struct parser_node*, int, char*, parse_event_fptr_t, int, char*, void*, struct parser_node*, struct parser_node*); /* creates and appends a parse node as child */
struct parser_node* fbt_append_successor(struct parser_node*, int, char*, parse_event_fptr_t, int, char*, void*, struct parser_node*, struct parser_node*); /* creates and appends a parse node as successor */

/* parse event handlers */
int event_new_rule(int, char*, void*); /* handler called if a new rule starts */
int event_add_parameter(int, char*, void*); /* handler called if a new parameter got parsed */
int event_rule_action(int, char*, void*); /* handler called if an action got parsed */
int event_mode(int, char*, void*); /* handler called in case of a mode definition */
int event_newline(int, char*, void*); /* handler called if a newline is encountered */
int event_callstack(int, char*, void*); /* handler to set the call origin object */

int strmatch(char*, char*); /* a simple string matching function */
int policy_exists(); /* checks if the policy file already exists */

#endif /* FBT_POLICY_PARSER_H */
