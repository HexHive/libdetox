#ifndef FBT_SANDBOX_H
#define FBT_SANDBOX_H

#include "fbt_sec_syscalls.h"

/* from kernel 2.6.31 */
//#define NR_SYSCALLS 242
/* from fbt_syscall_table.c */
#define NR_SYSCALLS 333

/* the file where to log syscalls if TRUST_SANDBOX_LOG is enabled */
#define LOG_FILE "/sandboxlog.tbt"

/* the syscall specification file */
#define SYSCALLS_SPEC_FILE "/syscalls.tbt"
#define MAX_SIZE_SYMB 256
#define MAX_NR_ARGS 6

/* authorization modes */
#define AUTH_DEF_DENY 0
#define AUTH_DEF_ALLOW 1

#define AUTH_DENY "default:deny"
#define AUTH_ALLOW "default:allow"

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

/**
 * The syscall description.
 */
struct syscall_desc {
  int nr; /* syscall number */
  char* name; /* syscall name */
  int nr_args; /* number of arguments */
  int* arguments; /* argument types as int array */
  int ret_type; /* return type */
#ifdef TRUST_HANDLE_SPECIAL_SYSCALLS
  authorize_syscl_fptr_t  old_authorize; /* old authorization function */
#endif
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
    char* call_origin; /* the object name of the location where the call came from */
    struct policy_rule *next; /* a pointer to the next rule */
};

typedef int (*verify_syscall_argument_fptr_t)(char*, int, struct syscall_desc*, struct policy_rule*); /* typedef for argument verification functions */

int fbt_init_sandbox(); /* initializes sandbox */
int fbt_load_syscall_specification(); /* loads the systemcalls specification */
int fbt_sandbox_authorization(int, int, int, int, int, int, int, int, int*); /* checks if a systemcall is authorized according to the policy table */

int verify_arg_int(char*, int, struct syscall_desc*, struct policy_rule*); /* verifies int arguments */
int verify_arg_ptr(char*, int, struct syscall_desc*, struct policy_rule*); /* verifies ptr arguments */
int verify_arg_string(char*, int, struct syscall_desc*, struct policy_rule*); /* verifies str arguments */
int verify_arg_undef(char*, int, struct syscall_desc*, struct policy_rule*); /* verifies undef arguments */
int verify_arg_void(char*, int, struct syscall_desc*, struct policy_rule*); /* verifies void arguments */

void abort_no_syscalldef_found(int, int, int, int, int, int, int); /* aborts in case the systemcall wasn't specified */
void abort_policy_violation(struct syscall_desc*, struct policy_rule*, int, int, int, int, int, int, int); /* aborts in case of a policy violation -> blacklist mode */
void abort_no_rule_found(struct syscall_desc*, int, int, int, int, int, int, int); /* aborts if no rule was found -> whitelist mode */
void abort_from_deny(struct syscall_desc*, int, int, int, int, int, int); /* abort if the user asked to deny the syscall -> interactive mode */

void print_rules(); /* prints the rules table */
void print_rule(struct policy_rule*); /* prints a specific rule */

int log_syscall(struct syscall_desc*, int, int, int, int, int, int, int, char*); /* logs a syscall in policy format */
void log_close(); /* closes the log file */


/******************************************/
/* interactive mode structs and functions */
/******************************************/
#define IPC_KEY "/usr/lib/libfastbt.so"
#define MSG_TYPE_INIT 1
#define MSG_TYPE_INIT_REP 2
#define MSG_TYPE_REQ 3
#define MSG_TYPE_REP 4
#define MAX_STRING_LEN 256

/* modes */
#define UNDEFINED 0
#define BLACKLIST 1
#define WHITELIST 2

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

struct sandbox_init init_sandbox_ipc(struct sandbox_init); /* initializes the interactive mode IPC */
int notify_UI(struct sandbox_request*, struct sandbox_reply*); /* notifies the user interface */
int free_ipc(); /* frees all IPC resources */
void fill_sreq_with_args(struct sandbox_request*, struct syscall_desc*, int*); /* prepares a sandbox_request struct */
void insert_policy_rule(int, struct sandbox_reply*); /* inserts a new policy rule into the rules table */
int handle_interactive_action(struct sandbox_request*, struct sandbox_reply*, struct syscall_desc*, int*, int, int*); /* handles an interactive syscall */

#endif /* FBT_SANDBOX_H */
