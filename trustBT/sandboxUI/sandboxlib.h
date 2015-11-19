/**
 * sandboxlib.h - Header file for sandboxlib functions.
 */

#ifndef SANDBOXLIB_H
#define SANDBOXLIB_H

/* max number of syscalls */
#define NR_SYSCALLS 332

/* the syscall specification file */
#define SYSCALLS_SPEC_FILE "./syscalls.tbt"
#define MAX_SIZE_SYMB 256
#define MAX_NR_ARGS 6

/* policy file name */
#define POLICY_FILE "./newpolicy.tbt"

/* nr of argument types (following this statement) */
#define ARG_NR_OF_TYPES 5

/* the type id numbers */
#define ARG_TYPE_INT 1
#define ARG_TYPE_PTR 2
#define ARG_TYPE_STR 3
#define ARG_TYPE_UNDEF 4
#define ARG_TYPE_VOID 5

/* the type strings */
#define ARG_TYPE_INT_STR "int"
#define ARG_TYPE_PTR_STR "ptr"
#define ARG_TYPE_STR_STR "string"
#define ARG_TYPE_UNDEF_STR "undef"
#define ARG_TYPE_VOID_STR "void"

/* ipc key and message types */
#define IPC_KEY "/usr/lib/libfastbt.so"
#define MSG_TYPE_INIT 1
#define MSG_TYPE_INIT_REP 2
#define MSG_TYPE_REQ 3
#define MSG_TYPE_REP 4

/* maximum string length for struct fields */
#define MAX_STRING_LEN 256

/* actions */
#define ACTION_DENY 1
#define ACTION_ALLOW 2
#define ACTION_RETURN 3
#define ACTION_CREATE 4
#define ACTION_CREATE_RET 5

/* wildcard string */
#define WILDCARD "*"
#define STRING_DELIMITER '"'
#define ARGS_DELIMITER ","

/* policy modes */
#define UNDEFINED 0
#define BLACKLIST 1
#define WHITELIST 2

/**
 * The syscall description.
 */
struct syscall_desc {
  int nr; /* syscall number */
  char* name; /* syscall name */
  int nr_args; /* number of arguments */
  int* arguments; /* argument types as int array */
  int ret_type; /* return type */
};

/**
 * The policy rule struct.
 */
struct policy_rule {
    struct syscall_desc *syscall; /* pointer to the according syscall description */
    char** arg_rules; /* the policy rule's arguments */
    int action; /* policy action */
    char* ret_string; /* the return value as a string */
    int retvalue; /* the return value as integer */
    struct policy_rule *next; /* a pointer to the next rule */
};

/**
 * The sandbox initialization message.
 * Same message struct is used for request(from sandbox to user interface) and reply(from user interface to sandbox).
 */
struct sandbox_init {
  long mtype;
  int mode; /* established policy mode */
};

/**
 * The sandbox request message.
 * This message struct is used for a action request. (from sandbox to user interface)
 * The fields specify a specific system call with all the arguments.
 */
struct sandbox_request {
  long mtype;
  int syscallnr; /* syscall number */
  char args[MAX_NR_ARGS][MAX_STRING_LEN]; /* arguments as array of strings */
};

/**
 * The sandbox reply message.
 * This message struct is used for a reply. (from user interface to sandbox)
 * The fields specify a specific action for a specific system call.
 */
struct sandbox_reply {
  long mtype;
  int syscallnr; /* syscall number */
  int action; /* action type */
  int retvalue; /* return value if needed */
  char args[MAX_NR_ARGS][MAX_STRING_LEN]; /* the policy rule arguments if a new rule gets created */
};

extern struct syscall_desc* syscalls[]; /* the syscalls description table */
extern int policy_mode; /* the policy mode */
extern struct policy_rule *rules[]; /* the policy rules */

int load_syscall_specification(); /* loads the syscall specification */
int init_sandbox_ipc(pid_t, struct sandbox_init*); /* init sandbox ipc - receives the sandbox init message */
int init_sandbox_ipc_reply(struct sandbox_init*); /* reply to the sandbox init message */
int wait_for_syscall(struct sandbox_request*); /* wait for syscalls and write values to sandbox_request* */
int notify_sandbox(struct sandbox_reply*); /* notify sandbox according to specified sandbox_reply* */

int add_policy_rule(int, struct sandbox_reply*); /* add a new policy rule */
int write_policy(char*); /* write policy file */

#endif /* SANDBOXLIB_H */