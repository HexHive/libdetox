#ifndef FBT_SEC_SYSCALLS_H
#define FBT_SEC_SYSCALLS_H

#include <sys/types.h>
// #include <sys/stat.h>

// blacklisted files in /proc/self:l maps smaps numa_maps
struct stat_list {
    dev_t device_id;
    ino_t inode_no;
    struct stat_list *next;
};

/* performs various initialization tasks related to the system call checker */
void fbt_init_sec_syscalls();


/*
 * Function that determines if a system call shall be allowed.
 * syscall_nr is the number of the system call, provided to the
 * system call in the %eax register.
 * arg1 to arg6 are the arguments provided to the system call
 * in the %ebx, %ecx, %edx, %esi, %edi and %ebp registers.
 * If a system call is not authorized, we need to provide a return
 * value of the system call (as if it was executed). Write this into
 * the address pointed to by retval. You can choose to do as if the
 * system call was executed normally, or return some error (probably -1)
 * and set errno.
 */
int test_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int short_strings(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int allow_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int deny_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int record_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int intercept_mprotect(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int intercept_mmap(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int check_open_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

int check_openat_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);
                        
int fbt_check_execve(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval);

/** how many entries the call stack can have to still fit within one page */
#define STACK_LIMIT 1024

/**
 * offset of the return instruction pointer with respect to the base pointer
 * not in bytes, but in pointer arithmetic (so 1 is 4 bytes if we have pointers)
 */
#define RIP_OFFSET 1
#define RIP_OFFSET_SYSENTER 3

/*
 * get the call stack (all the return instruction pointers of the call
 * hierarchy
 */
int get_call_stack(void **base_ptr, void ***call_stack, int is_sysenter);

typedef int (*authorize_syscl_fptr_t)(int, int, int, int, int, int, int, int, int*);

/**
 * table of pointers to functions that authorize system calls.
 * defined in fbt_syscall_table.c
 */
//extern const authorize_syscl_fptr_t authorize_syscl_table[];
extern authorize_syscl_fptr_t authorize_syscl_table[];

#endif /* FBT_SEC_SYSCALLS_H */

