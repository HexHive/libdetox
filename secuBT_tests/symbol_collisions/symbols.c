#define _GNU_SOURCE
#include <link.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

/* See if using the same function name in the executable causes a problem. Is not the case... */
int ind_jump(int a, int b)
{
    if (a > b) {
        return a - 1;
    }
    return b + 1;
}

int callback1(struct dl_phdr_info *info, size_t size, void *data)
{
    const char *name = info->dlpi_name;
    char procname[FILENAME_MAX];
    
    if (2 == *((int*) data)) {
        // so far, libfastbt.so always seems to be the third listet object
        printf("using object %s at base address %p\n", info->dlpi_name, 
                info->dlpi_addr);
        dlerror();
        // next line causes segfault; we cannot use base address as handle
        void *sym_ptr = dlsym(info->dlpi_addr, "authorize_syscl_table");
        const char *errormsg = dlerror();
        if (NULL == errormsg) {
            printf("symbol lookup successful: %p\n", sym_ptr);
        } else {
            printf("symbol lookup error: %s\n", errormsg);
        }
        
    }

    (*((int*)data))++;
    return 0;
}

int main(int argc, char **argv)
{
    int a = 8;
    int b = 9;
    
    /* 
     * see if LD_PRELOAD also can replace symbols in executables, or only in 
     * shared libraries. result: only in shared libraries. ind_jump is not 
     * replaced.
     */
    int c = ind_jump(a, b);
    printf("c: %d\n", c);
    
    /* try to read secuBT symbols using dlsym(), shall not be allowed */
    dlerror();
    void *sym_ptr = dlsym(RTLD_DEFAULT, "authorize_syscl_table");
    const char *errormsg = dlerror();
    
    if (NULL == errormsg) {
        printf("symbol lookup for authorize_syscl_table successful: %p\n", sym_ptr);
    } else {
        printf("symbol lookup error: %s\n", errormsg);
    }
    
    /* try to read dlsym symbol using dlsym(), shall be allowed */
    dlerror();
    sym_ptr = dlsym(RTLD_DEFAULT, "dlsym");
    errormsg = dlerror();
    
    if (NULL == errormsg) {
        printf("symbol lookup for dlsym successful: %p\n", sym_ptr);
    } else {
        printf("symbol lookup error: %s\n", errormsg);
    }
    
    // see if we can use the base address as the library handle for dlsym
    // --> no, does not work
//     int i = 0;
//     dl_iterate_phdr(&callback1, (void*) &i);

	extern void *o_pthread_create;
	/* 
	 * now let's see if we can read o_pthread_create. if we read it 
	 * from libtestme, it will print "0x1234abcd", if it is read from 
	 * libfastbt.so, it will print something else.
	 * result: it works, some different value is read if libfastbt.so is loaded
	 */
	printf("reading o_pthread_create: %p\n", o_pthread_create);

    printf("RTLD_DEFAULT: %p, RTLD_NEXT: %p\n", RTLD_DEFAULT, RTLD_NEXT);
    
    return 0;
}
