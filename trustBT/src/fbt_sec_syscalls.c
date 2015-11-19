/**
 * IMPORTANT WARNING.
 * Be careful what functions you call in these functions! Some functions that
 * use system calls are not reentrant! For example printf and malloc are NOT
 * reentrant. Therefore you must not call printf when authorizing write syscalls
 * or malloc when authorizing mmap2 system calls.
 * In general, you must not call a non-reentrant function when authorizing
 * system calls that could occur in said non-reentrant function.
 *
 * small list of system calls and non-reentrant functions that use them
 * mmap2        malloc, calloc, realloc, posix_memalign
 * munmap       free
 * write        printf et al.
 */

#define _ATFILE_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "fbt_sec_syscalls.h"
#include "fbt_mem_protection.h"
#include "fbt_llio.h"
#include "fbt_debug.h"
#include "fbt_private_datatypes.h"
#include "libfastbt.h"

static struct stat_list *fbt_blacklist;
static const char *ld_preload;

/**
 * adds the device and inode numbers of the specified file to a blacklist.
 */
int fbt_blacklist_file(const char *path)
{
    struct stat info;
    int retval = stat(path, &info);
    if (-1 != retval) {
        /*
         * we use malloc here, so this function may only be used in places
         * where it is guaranteed that we are not inside a malloc call in the
         * guest code!
         */
        struct stat_list *list_elem = (struct stat_list*) malloc(sizeof(struct stat_list));
        list_elem->device_id = info.st_dev;
        list_elem->inode_no = info.st_ino;
        list_elem->next = fbt_blacklist;
        fbt_blacklist = list_elem;
    }
    return retval;
}

/**
 * initialization of data structures related to safe system calls.
 * This currently populates the list of files that guest code may not open.
 */
void fbt_init_sec_syscalls()
{
    ld_preload = getenv("LD_PRELOAD");
    //llprintf("setting ld_preload to %s\n", ld_preload);
    fbt_blacklist = NULL;
#ifndef SECU_BLOCK_PROC_ACCESS
    /*
     * If we do not block access to /proc in general, at least block access
     * to some sensitive files to the current process. Forked or child
     * processes could still read them, however!
     */
    fbt_blacklist_file("/proc/self/maps");
    fbt_blacklist_file("/proc/self/smaps");
    fbt_blacklist_file("/proc/self/numa_maps");
    fbt_blacklist_file("/proc/self/stat");
#endif /* SECU_BLOCK_PROC_ACCESS */
}



/**
 * test authorization of system calls
 * Authorize or deny a system call based on the system call number and / or
 * the arguments to the system call.
 * @param syscall_nr the system call number stored in %eax
 * @param arg1 the first argument, stored in %ebx
 * @param arg2 the second argument, stored in %ecx
 * @param arg3 the third argument, stored in %edx
 * @param arg4 the fourth argument, stored in %esi
 * @param arg5 the fifth argument, stored in %edi
 * @param arg6 the sixth argument, stored in %ebp
 * @param retval pointer to the integer where a fake return value may be put
 * @return 1 if the system call is to be allowed, 0 otherwise
 */
int test_syscall(int syscall_nr,
                int arg1,
                int arg2,
                int arg3,
                int arg4,
                int arg5,
                int arg6,
                int is_sysenter,
                int *retval)
{
    // printf is not reentrant, we cannot call printf inside itself
//     if (4 != syscall_nr) {  // only write syscalls occur inside printf
//         if (arg3 > 12) {
//             //errno = EINVAL;
//             *retval = arg3;
//             return 0;
//         }
    llprintf("system call nr: %d, args(%d, %d, %d, %d, %d, %d)\n",
              syscall_nr,
              arg1,
              arg2,
              arg3,
              arg4,
              arg5,
              arg6);


    void **call_stack;
    int call_stack_size = get_call_stack((void**) arg6, &call_stack, is_sysenter);
    llprintf("call stack of size %d:\n", call_stack_size);
    int i = 0;
    struct mem_info info;
    for (i = 0; i < call_stack_size; i++) {
        if (fbt_memprotect_info(call_stack[i], &info)) {
            llprintf("[%d]: %p in %s, section %s\n",
                i,
                call_stack[i],
                info.obj_name,
                info.sec_name
                );
        } else {
            llprintf("[%d]: %p in unknown memory\n", i, call_stack[i]);
        }
    }

    freepage(call_stack);
//     }
    return 1;
}

/* only allow write syscalls if at most 12 characters are printed */
int short_strings(int syscall_nr,
                int arg1,
                int arg2,
                int arg3,
                int arg4,
                int arg5,
                int arg6,
                int is_sysenter,
                int *retval)
{
    if (arg3 > 12) {
        errno = EINVAL;
        *retval = arg3;
        return 0;
    }
    return 1;
}

int allow_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    return 1;
}

int deny_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    *retval = -1;
    return 0;
}

int record_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    // TODO record arguments, call stack
    return 1;
}

int intercept_mprotect(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    // ensure we don't make memory structures of BT executable
    struct thread_local_data *tld = get_tld();
    struct rb_node* node = rb_query(tld->mem_alloc.chunks_tree, (void*) arg1);
    if (NULL != node) {
        // is an internal data structure of the binary translator!
        *retval = -1;
        errno = EFAULT;
        return 0;
    }

    struct mem_info info;
    if (fbt_memprotect_info((void*) arg1, &info)) {
        if (info.flags & INFO_BTFLAG) {
            // tries to modify memory of libfastbt.so
            *retval = -1;
            errno = EFAULT;
            return 0;
        } else {
            INFO_LLPRINTF("mprotect that affects mapping of %s in section %s,\n"
                           "address %p, length %d found\n", info.obj_name,
                           info.sec_name, (void*) arg1, arg2);
        }
    }
#ifdef SECU_ALLOW_RUNTIME_ALLOC
    if (arg3 & PROT_EXEC) {
        fbt_memprotect_add_valid((void*) arg1, arg2);
//         INFO_LLPRINTF("intercepted mprotect call that sets memory executable\n");
    }
#endif /* SECU_ALLOW_RUNTIME_ALLOC */

    return 1;
}

int intercept_mmap(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
#ifdef SECU_ALLOW_RUNTIME_ALLOC
    if ((arg3 & PROT_EXEC) && (arg4 & MAP_ANONYMOUS)) {
        // runtime allocation of executable memory without file backing
        *retval = (int) mmap((void*) arg1, arg2, arg3, arg4 & (~MAP_FIXED),
                               arg5, arg6);
        fbt_memprotect_add_valid((void*) retval, arg2);
        return 0;
    }
 #endif /* SECU_ALLOW_RUNTIME_ALLOC */

    if (arg4 & MAP_FIXED) {
        /*
         * mapping would be forcedly located at specified address, and overwrite
         * any mapping already present. We cannot allow that. Ths system call is
         * rewritten without MAP_FIXED
         */
        *retval = (int) mmap((void*) arg1, arg2, arg3, arg4 & (~MAP_FIXED),
                               arg5, arg6);
        return 0;
    }
    return 1;
}

/**
 * check if guest code tries to open a blacklisted file.
 * Uses stat(2) to determine the device id and inode number of the pathname of
 * an open syscall and checks it against a blacklist. If the file already exists
 * and is in the blacklist, the syscall is denied with an EACCES error.
 */
int check_open_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    struct stat info;
    if (-1 != stat((char*) arg1, &info)) {
        struct stat_list *list_elem = fbt_blacklist;
        while (NULL != list_elem) {
            if ((list_elem->device_id == info.st_dev) &&
                            (list_elem->inode_no == info.st_ino)) {
                *retval = -1;
                errno = EACCES;
                INFO_LLPRINTF("secuBT: denied opening of %s\n", arg1);
                return 0;
            }
            list_elem = list_elem->next;
        }
#ifdef SECU_BLOCK_PROC_ACCESS
        int proc_dev = info.st_dev;
        if (-1 != stat("/proc/self/maps", &info)) {
            if (info.st_dev == proc_dev) {
                *retval = -1;
                errno = EACCES;
                INFO_LLPRINTF_VERBOSE("secuBT: denied opening of file %s because "
                            "it is located in /proc\n", arg1);
                return 0;
            }
        }
#endif /* SECU_BLOCK_PROC_ACCESS */
    }
    return 1;
}

int check_openat_syscall(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    // let's do the system call and analyze the file descriptor afterwards
    int fd = openat(arg1, (const char*) arg2, arg3);
    struct stat info;
    if (-1 != fstat(fd , &info)) {
#ifdef SECU_BLOCK_PROC_ACCESS
        int proc_dev = info.st_dev;
        if (-1 != stat("/proc/self/maps", &info)) {
            if (info.st_dev == proc_dev) {
                *retval = -1;
                errno = EACCES;
                INFO_LLPRINTF_VERBOSE("secuBT: denied opening of file %s because "
                            "it is located in /proc\n", arg1);
                close(fd);
                return 0;
            }
        }
#endif /* SECU_BLOCK_PROC_ACCESS */
        struct stat_list *list_elem = fbt_blacklist;
        while (NULL != list_elem) {
            if ((list_elem->device_id == info.st_dev) &&
                            (list_elem->inode_no == info.st_ino)) {
                *retval = -1;
                errno = EACCES;
                INFO_LLPRINTF("secuBT: denied opening of %s\n", arg1);
                close(fd);
                return 0;
            }
            list_elem = list_elem->next;
        }
    }
    close(fd);
    return 1;
}

/**
 * check in execve syscalls if the environment variables are ok.
 * Specifically, we have to make sure that our library is still in
 * LD_PRELOAD. Otherwise, a way to break out of binary translation would be
 * system("LD_PRELOAD=\"\" evil_command");
 */
int fbt_check_execve(int syscall_nr,
                        int arg1,
                        int arg2,
                        int arg3,
                        int arg4,
                        int arg5,
                        int arg6,
                        int is_sysenter,
                        int *retval)
{
    //char *ld_preload = getenv("LD_PRELOAD");
    INFO_LLPRINTF("blocked execve system call of %s\n", arg1);
    return 0;
    char **envp = (char**) arg3;
    int result = 0;
    int i = 0;
    while (NULL != *envp) {
        char *env_var = *envp;
        if (0 == strncmp("LD_PRELOAD=", env_var, 11)) {
            // we have found LD_PRELOAD in the environment variables array
            if (0 == strncmp(env_var + 11, ld_preload, 256)) {
                // unmodified LD_PRELOAD
                result = 1;
            } else {
                result = 0;
            }
        }
        i++;
        envp++;
    }
    if (0 == result) {
        *retval = -1;
        errno = EACCES;
        INFO_LLPRINTF("command %s not executed due to modified LD_PRELOAD\n",
                       arg1);
    } else {
        INFO_LLPRINTF_VERBOSE("command %s executed with LD_PRELOAD=%s\n",
                               arg1, getenv("LD_PRELOAD"));
    }
    return result;
}



#define STACK_ALLOC 8

/**
 * get the call stack.
 * This function retrieves the call stack of a function call. The calling
 * function assumes ownership of the memory of the return value and is
 * responsible for freeing it with freepage() when it is not used any more.
 * @param base_ptr the base pointer, i. e. the contents of the %ebp register
 * @param call_stack pointer to an array of return instruction pointers
 * @return the number of entries in the call stack
 */
int get_call_stack(void **base_ptr, void ***call_stack, int is_sysenter)
{
    void **stack = (void**) allocpage();
    int stack_height = 0;
    void *rip = 0;

//     void *rip = *(base_ptr + RIP_OFFSET);

//     while (fbt_memprotect_execquery(rip)) {
    while (base_ptr && (stack_height < STACK_LIMIT)) {
        if ((0 == stack_height) && is_sysenter) {
            rip = *(base_ptr + RIP_OFFSET_SYSENTER);
        } else {
            rip = *(base_ptr + RIP_OFFSET);
        }

        // entry into call stack
        stack[stack_height] = rip;

        // iterate to next stack frame
        base_ptr = *base_ptr;
        stack_height++;
    }
    *call_stack = stack;
    return stack_height;
}


