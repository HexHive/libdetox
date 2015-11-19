#define _GNU_SOURCE
#include <link.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fbt_mem_protection.h"
#include "fbt_rbtree.h"
#include "fbt_debug.h"

#define ITERATE 5

/**
 * initial size of the list of shared objects
 */
#define LIB_LIST_INITSIZE 64

/* type definitions */
typedef Elf32_Sym fbt_sym;  // Elf64_Sym if secuBT compiled as 64bit library

// function prototypes of private functions
static void fbt_lib_list_resize();
static int fbt_memprotect_callback(struct dl_phdr_info *info, size_t size,
                                    void *data);
#ifdef SECU_DETECT_SYMBOL_COLLISIONS
static uint32_t fbt_gnu_hash(const char *s);
static fbt_sym *fbt_lookup_symbol(const char *name, void *hashtab, fbt_sym *symtab, char *strtab);
static int fbt_check_sym_collision(fbt_sym *symtab, char *strtab, void *hashtab);
#endif

// global variables (only accessible from this file)
static struct   lib_list_entry *library_list = 0;
static int      lib_list_size = 0;
static int      lib_list_capacity = 0;
static int      rescan_lock = 0;
static const char ralloc_str[] = "runtime allocated memory";
static const char internal_str[] = "internal memory structure of the "
                                   "binary translator";
#ifdef SECU_DETECT_SYMBOL_COLLISIONS
static fbt_sym  *fbt_dyn_symtab = NULL; // dynamic linking symbol table
static char     *fbt_dyn_strtab = NULL; // dynamic linking string table
static fbt_sym  *exe_dyn_symtab = NULL; // symbol table of executable
static char     *exe_dyn_strtab = NULL; // string table of executable
static void     *exe_dyn_hashtab = NULL;    // hash table of executable
#endif
static int      fbt_dyn_nsyms = 0;      // number of symbols in table

/*
 * red-black tree of the sections of all loaded objects (the executable and
 * the loaded shared libraries) and all runtime-allocated memory with
 * PROT_EXEC flag set (execution allowed).
 */
static struct rb_node *sections_root = NULL;

#ifdef SECU_DETECT_SYMBOL_COLLISIONS
/**
 * whitelist of symbols that occur in secuBT, and may also occur in other
 * objects.
 */
static const char *fbt_sym_whitelist[] = {
    "_init", "_fini", "dlclose", "pthread_create"
};
#endif


void fbt_memprotect_init()
{
    if (0 == lib_list_capacity) {
        library_list = (struct lib_list_entry*) malloc(LIB_LIST_INITSIZE * sizeof(*library_list));
        if (NULL == library_list) {
            perror("fbt_memprotect_init: could not allocate memory for library list");
            exit(EXIT_FAILURE);
        }
        lib_list_capacity = LIB_LIST_INITSIZE;
    }
}

void fbt_lib_list_resize()
{
    library_list = (struct lib_list_entry*) realloc(library_list, 2 * lib_list_capacity * sizeof(*library_list));
    if (NULL == library_list) {
        perror("fbt_lib_list_resize: could not reallocate memory for library list");
        exit(EXIT_FAILURE);
    }
    lib_list_capacity *= 2;
}

#ifdef SECU_DETECT_SYMBOL_COLLISIONS
static uint32_t fbt_gnu_hash(const char *s)
{
        uint32_t h = 5381;
        unsigned char c;
        for (c = *s; c != '\0'; c = *++s)
                h = h * 33 + c;

        return h;
}

static fbt_sym *fbt_lookup_symbol(const char *name, void *hashtab, fbt_sym *symtab,
                         char *strtab)
{
    unsigned int nbuckets = *((int*) hashtab);      // number of hash buckets
    unsigned int symndx = *((int*) hashtab + 1);    // symbol table index of first hashed symbol
    unsigned int maskwords = *((int*) hashtab + 2); // # mask words index in the bloom filter - 1
//     unsigned int shift2 = *((unsigned int*) hashtab + 3);     // bloom filter hash shift
    unsigned long *bloom = (unsigned long*) ((int*) hashtab + 4);   // pointer to the bloom filter words
    unsigned int *hashbuck = (unsigned int*) (bloom + maskwords);   // pointer to the hash buckets
    unsigned int *hasharr = hashbuck + nbuckets;    // pointer to the hash values array
    unsigned int hash2;
    unsigned int n;
//     unsigned int bitmask;

    // generate hash value of the symbol
    unsigned int hash1 = fbt_gnu_hash(name);


// test against bloom filter disabled because it does not work...
//     hash2 = hash1 >> shift2;    // 2nd hash for the bloom filter
//     n = (hash1 / sizeof(long) * 8) & (maskwords - 1);
//     bitmask = (1 << (hash1 % (sizeof(long) * 8)))
//             | (1 << (hash2 % (sizeof(long) * 8)));
//     if ((bloom[n] & bitmask) != bitmask)
//         return NULL;

    // find hash chain and hash value
    n = hashbuck[hash1 % nbuckets];
    if (0 == n) // symbol not found
        return NULL;

    fbt_sym *sym = &symtab[n];
    unsigned int *hashval = &hasharr[n - symndx];

    // walk hash chain until symbol found or end of chain
    for (hash1 &= ~1; ; sym++) {
        hash2 = *hashval++;

        if ((hash1 == (hash2 & ~1)) && !strcmp(name, strtab + sym->st_name)) {
            return sym;
        }

        // test if we are at end of chain
        if (hash2 & 1)
            break;

    }

    // symbol was not found
    return NULL;
}

static int fbt_check_sym_collision(fbt_sym *symtab, char *strtab, void *hashtab)
{
    int i = 0;
    // loop over symbols of libfastbt.so
    for (i = 0; i < fbt_dyn_nsyms; i++) {
        unsigned char info = fbt_dyn_symtab[i].st_info;
        fbt_sym sym = fbt_dyn_symtab[i];


        char bind = ELF32_ST_BIND(info);
        char type = ELF32_ST_TYPE(info);

        // only process global functions and variables in libfastbt
        if ((sym.st_value != 0) && (STB_GLOBAL == bind)
                        && ((STT_FUNC == type) || (STT_OBJECT == type)))  {
            fbt_sym *coll = fbt_lookup_symbol(&fbt_dyn_strtab[sym.st_name],
                                                hashtab, symtab,
                                                strtab);
            if (NULL != coll) {
                // oops, collision detected, now check if in whitelist
                int j = 0;
                int in_whitelist = 0;
                for (j = 0; j < sizeof(fbt_sym_whitelist) / sizeof(char*); j++) {
                    if (!strcmp(fbt_sym_whitelist[j],
                                &fbt_dyn_strtab[sym.st_name])) {
                        in_whitelist = 1;
                        break;
                    }
                }
                if (!in_whitelist) {
                    INFO_LLPRINTF("secuBT has detected a symbol collision! "
                                "Conflicting symbol is %s\n",
                              &fbt_dyn_strtab[sym.st_name]);
                    return 0;
                }
            }
        }
    }
    return 1;
}
#endif

void fbt_memprotect_rescan()
{
    /*
     * Loop over loaded shared objects to see if some additional ones
     * were loaded. If that is the case, add the new ones to the library list
     * and their sections to the tree.
     */

    int i = 0;
    while (__sync_val_compare_and_swap(&rescan_lock, 0, 1)) {
        /* spin while waiting to acquire mutex */
        i++;
        if (0 == (i % 10000)) {
            printf("waiting to acquire rescan lock: spinned %d times\n", i);
        }
    }

    int objcount = 0;
    dl_iterate_phdr(&fbt_memprotect_callback, (void*) &objcount);

    for (i = 0; i < lib_list_size; i++) {
        INFO_LLPRINTF_VERBOSE("%s from %p to %p\n",
                       library_list[i].name,
                       library_list[i].base_addr,
                       library_list[i].base_addr + library_list[i].length);
    }

    // release mutex
    rescan_lock = 0;
}

void fbt_memprotect_add_valid(void *addr_begin, int len)
{
    struct mem_info *info = (struct mem_info*) malloc(sizeof(struct mem_info));
    info->node.addr_begin = addr_begin;
    info->node.addr_end = addr_begin + len;
    info->flags = INFO_RFLAG | INFO_XFLAG;

    info->lib_index = 0;    // TODO check if we can set this -1 or so
    info->sec_name = ralloc_str;
    sections_root = rb_insert(sections_root, (struct rb_node*) info);
}

/**
 * callback function used to scan the loaded libraries.
 * This function is used as callback in dl_iterate_phdr(3) to scan the sections
 * of a loaded library. The first object in the list (position 0) is always the
 * executable, the second one the vdso ("linux-gate.so"). The third object is
 * the secuBT library (libfastbt.so), because it is loaded with LD_PRELOAD.
 * The sections of the objects are added to the red-black-tree structure along
 * with information on whether they contain executable code.
 * For more info on dl_iterate_phdr, refer to its man page.
 * @param info the object info structure provided by dl_iterate_phdr
 * @param size the size of the info structure
 * @param data is used to store the position in the list of libraries
 */
int fbt_memprotect_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    int libnum = *((int*) data);

    // look for the base address in the library list
    int i = 0;
    for (i = 0; i < lib_list_size; i++) {
        if (library_list[i].base_addr ==  (void*) info->dlpi_addr) {
            // the entry for this object already exists in the library list
            (*((int*)data))++;
            return 0;
        }
    }

    if (lib_list_size >= lib_list_capacity) {
        fbt_lib_list_resize();
    }

    const char *name = info->dlpi_name;

    /*
     * The first entry of dl_iterate_phdr's table of objects is always the
     * executable itself, the second the vdso. They don't have dlpi_addr as
     * base address and have to be caught in order not to be processed again.
     * After these two come the shared objects.
     */
    if (0 == libnum) {
        if (lib_list_size > 0) {
            // executable has to be already be in the library list
            (*((int*)data))++;
            return 0;
        }
        name = "/proc/self/exe";
    } else if (1 == libnum) {
        if (lib_list_size > 1) {
            // vdso has to be already be in the library list
            (*((int*)data))++;
            return 0;
        }
        // ugly hack to get correct address of vdso
        void *vdso_addr = (void*) (info->dlpi_addr + info->dlpi_phdr[0].p_vaddr);

        // add vdso to library table
        library_list[lib_list_size].base_addr = vdso_addr;
        library_list[lib_list_size].length = sysconf(_SC_PAGESIZE);
        library_list[lib_list_size].name = (char *) malloc(5 * sizeof(char));
        strcpy(library_list[lib_list_size].name, "vdso");
        lib_list_size++;


        // add vdso entry to sections tree
        struct mem_info *info = malloc(sizeof(*info));
        info->node.addr_begin = vdso_addr;
        info->node.addr_end = vdso_addr + sysconf(_SC_PAGESIZE);
        info->sec_name = library_list[lib_list_size - 1].name;
        info->lib_index = lib_list_size - 1;
        info->flags = INFO_RFLAG | INFO_XFLAG;
        sections_root = rb_insert(sections_root, (struct rb_node*) info);

        (*((int*)data))++;
        return 0;
    }
    if (0 == strcmp(name, "")) {
        (*((int*)data))++;
        return 0;
    }

    // entry in library list
    library_list[lib_list_size].base_addr = (void*) info->dlpi_addr;
    library_list[lib_list_size].length = 0;
    library_list[lib_list_size].name = malloc(strlen(name) + 1);
    strcpy(library_list[lib_list_size].name, name);
    lib_list_size++;

    /*
     * we map library file again to memory, because when it is loaded for execution, the
     * section header table and the section containing the section name strings are not
     * mapped to memory.
     */
    // open the file -> file descriptor
    int fd = open(name, O_RDONLY);
    if (-1 == fd) {
        perror("fbt_iterate_sections: could not open executable or library file");
        (*((int*)data))++;
        return -1;
    }

    // find out file size
    struct stat filestat;
    if (-1 == fstat(fd, &filestat)) {
        perror("fbt_iterate_sections: fstat failure, cannot find out file size");
        return -1;
    }

    void *libmap = mmap(NULL, filestat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ((void*) -1 == libmap) {
        perror("fbt_iterate-sections: failed to map library to memory");
        close(fd);
        (*((int*)data))++;
        return -1;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr*) libmap;

    fbt_sym *dyn_symtab = NULL;
    char *dyn_strtab = NULL;
    void *dyn_hashtab = NULL;

    if (0 != header->e_shnum) {
        Elf32_Shdr *section_header = (Elf32_Shdr*) (libmap + header->e_shoff);
        //printf("\tsection header table: %p\n", (void*) section_header);

        int i = 0;
        for (i = 0; i < header->e_shnum; i++) {
            // only consider sections loaded into memory on execution
            if ((section_header[i].sh_size < 1)
                || ((section_header[i].sh_flags & SHF_ALLOC) == 0)) {
                continue;
            }

            // fill in node information
            struct mem_info *meminfo = (struct mem_info*) malloc(sizeof(*meminfo));
            meminfo->node.addr_begin = (void*) (info->dlpi_addr
                                + section_header[i].sh_addr);
            meminfo->node.addr_end = meminfo->node.addr_begin
                                + section_header[i].sh_size;

            // calculate start address of the executable
            if ((0 == libnum) && (0 == library_list[0].base_addr)) {
                library_list[0].base_addr = (void*) ((long) (meminfo->node.addr_begin)
                                & ~(sysconf(_SC_PAGESIZE) - 1));
            }

            // section name
            if (SHN_UNDEF != header->e_shstrndx) {
                const char *name_entry = (const char*) (libmap
                                 + section_header[header->e_shstrndx].sh_offset
                                 + section_header[i].sh_name);
                char *section_name = (char*) malloc(strlen(name_entry) + 1);
                strcpy(section_name, name_entry);
                meminfo->sec_name = section_name;
            } else {
                const char* no_sec_names = "no section name string table";
                char *section_name = (char*) malloc(strlen(no_sec_names) + 1);
                strcpy(section_name, no_sec_names);
                meminfo->sec_name = section_name;
            }
            // update size in memory
            int length = meminfo->node.addr_end
                                - library_list[lib_list_size - 1].base_addr;
            if (length > library_list[lib_list_size - 1].length) {
                library_list[lib_list_size - 1].length = length;
            }

            // flags
            meminfo->flags = INFO_RFLAG;
            if (section_header[i].sh_flags & SHF_WRITE) {
                meminfo->flags |= INFO_WFLAG;
            }
            if (2 == libnum) {
                // memory in libfastbt.so
                meminfo->flags |= INFO_BTFLAG;
            }
            if (section_header[i].sh_flags & SHF_EXECINSTR) {
                // only mark as executable if we memory not part of libfastbt.so
                meminfo->flags |= INFO_XFLAG;
            }
            meminfo->lib_index = lib_list_size - 1;

            sections_root = rb_insert(sections_root, (struct rb_node*) meminfo);

            // code for symbol checking
            if (SHT_DYNSYM == section_header[i].sh_type) {
                // we have found the dynamic linking symbol table
                dyn_symtab = (Elf32_Sym*) (info->dlpi_addr
                                                + section_header[i].sh_addr);
                if (2 == libnum) {
                    // handling libfastbt.so, need to know # of symbols
                    fbt_dyn_nsyms = section_header[i].sh_size / sizeof(fbt_sym);
                }
            }
            if (!strcmp(".dynstr", meminfo->sec_name)) {
                // string table for dynamic linking symbols
                dyn_strtab = (char*) (info->dlpi_addr
                                                + section_header[i].sh_addr);
            }
            if (!strcmp(".gnu.hash", meminfo->sec_name)) {
                // hash table for dynamic linking symbols (gnu format)
                dyn_hashtab = (void*) (info->dlpi_addr
                                                + section_header[i].sh_addr);
            }
        }
    }
#ifdef SECU_NX_PROG
    // only executed on first run of this function
    if (0 == libnum) {
        /*
         * Set the memory of the program non-executable. We cannot do the same
         * to the libraries, as we need some of them ourselves.
         */
	/* TODO: protect libs as well and compile bt statically */
        mprotect(library_list[0].base_addr, library_list[0].length,
                  PROT_READ | PROT_WRITE);
    }
#endif
#ifdef SECU_DETECT_SYMBOL_COLLISIONS
    if ((NULL != dyn_symtab) && (NULL != dyn_strtab) && (NULL != dyn_hashtab)) {
        if (0 == libnum) {
            /*
             * we can not check the symbols of the executable now because
             * libfastbt.so has not been processed yet.
             */
            exe_dyn_symtab = dyn_symtab;
            exe_dyn_strtab = dyn_strtab;
            exe_dyn_hashtab = dyn_hashtab;
        } else if (2 == libnum) {
            fbt_dyn_symtab = dyn_symtab;
            fbt_dyn_strtab = dyn_strtab;

            // ok, now we can check the symbols of the executable
            if (!fbt_check_sym_collision(exe_dyn_symtab, exe_dyn_strtab,
                                        exe_dyn_hashtab)) {
                // symbol collision detected
                llprint("Symbol collision in the executable\n");
                _exit(1);
            }
        } else if (libnum > 2) {
            // check symbols of library
            if (!fbt_check_sym_collision(dyn_symtab, dyn_strtab, dyn_hashtab)) {
                llprintf("Symbol collision in %s\n", name);
                _exit(1);
            }
        }
    } else {
        // one of the tables was not found!
        llprintf("Fatal: One of the tables for symbol checking was not found!\n"
                 "symtab: %p, strtab: %p, hashtab: %p\n", dyn_symtab,
                  dyn_strtab, dyn_hashtab);
        _exit(1);
    }
#endif
    munmap(libmap, filestat.st_size);
    close(fd);
    (*((int*)data))++;
    return 0;
}

/**
 * check if a memory location is executable.
 * This function checks if an address is in a executable section of the
 * executable or of a shared library. If so, it returns 1, otherwise 0.
 * @param tu_addr the address to check
 * @return 1 if it is in an executable section, 0 otherwise
 */
int fbt_memprotect_execquery(void *tu_addr)
{
    struct mem_info* info = (struct mem_info*) rb_query(sections_root, tu_addr);
    if (NULL == info) {
        // address not found in section tree -> rebuild and try again
        fbt_memprotect_rescan();
        info = (struct mem_info*) rb_query(sections_root, tu_addr);
        if (NULL == info) {
            // address still not found -> should not allow translation
            return 0;
        }
    }
    if (info->flags & INFO_XFLAG) {
        // address is in a section with executable code -> allow translation
        return 1;
    } else {
        // address lies in a section that should not contain executable code
        return 0;
    }
}

/**
 * retrieve information about a memory location.
 * Searches the red-black-tree of sections for a memory range that includes
 * tu_addr. If one is found, 1 is returned and the info struct is filled in.
 * If no information is available about this memory address, the function
 * returns 0.
 * The tree contains the information about the sections of the executable and
 * the libraries, as well as runtime-allocated memory that was made executable.
 * @param tu_addr the query address
 * @param info a pointer to the struct to fill in
 * @return 1 if information about tu_addr was found, 0 otherwise
 */
int fbt_memprotect_info(void *tu_addr, struct mem_info *info)
{
    struct mem_info *meminfo = (struct mem_info*) rb_query(sections_root, tu_addr);
    if (NULL == meminfo) {
        // query address not found in section tree
        info->node.addr_begin = info->node.addr_end = 0;
        info->flags = 0;
        return 0;
    } else {
        info->node.addr_begin = meminfo->node.addr_begin;
        info->node.addr_end = meminfo->node.addr_end;
        info->obj_name = library_list[meminfo->lib_index].name;
        info->sec_name = meminfo->sec_name;
        info->flags = meminfo->flags;
        return 1;
    }
}

/**
 * unlock a data structure and add it to the lockdown list.
 * Unlocks a data structure given by an alloc_chunk and also adds it to the
 * lockdown list so it will be write-protected again before returning control
 * to the guest code.
 * @param tld pointer to the thread local data
 * @param chunk the allocated memory to unlock
 */
static void fbt_ids_unlock_helper(struct thread_local_data *tld,
                                   struct alloc_chunk *chunk)
{
//     struct mem_alloc_data *mem_alloc = &(tld->mem_alloc);

    // unlock the memory
    int length = chunk->node.addr_end - chunk->node.addr_begin;
    mprotect(chunk->node.addr_begin, length, PROT_READ | PROT_WRITE);

//     // insert at the beginning of the lockdown list
//     chunk->next_lockdown = mem_alloc->lockdown_list;
//     mem_alloc->lockdown_list = chunk;
}

/**
 * remove write-protection from all internal memory structures.
 * This function removes the write protection from all internal memory
 * structures of the binary translator in the current thread. It tries to unlock
 * adjoining memory allocations using a single mprotect call to speed things up
 * considerably.
 * @param tld pointer to the thread local data
 */
void fbt_ids_unlock(struct thread_local_data *tld)
{
    /*
     * A chunk is an allocation of memory. A span is several adjoining
     * allocations of memory that can be unlocked together using only one
     * mprotect call.
     */
    struct alloc_chunk *chunk = tld->mem_alloc.chunks;
    void *span_begin = NULL;
    void *span_end = NULL;

    while (NULL != chunk) {
        if (chunk->node.addr_begin == span_end) {
            // we can add the chunk to the span
            span_end = chunk->node.addr_end;
        } else {
            // cannot add to existing span. unlock existing span
            if (NULL != span_begin) {
                mprotect(span_begin, span_end - span_begin,
                          PROT_READ | PROT_WRITE | PROT_EXEC);
            }
            // start new span with current chunk

            span_begin = chunk->node.addr_begin;
            span_end = chunk->node.addr_end;
        }
        chunk = chunk->next;
    }
    // unlock last span
    if (NULL != span_begin) {
        mprotect(span_begin, span_end - span_begin,
                  PROT_READ | PROT_WRITE | PROT_EXEC);
    }

}

/* unlocks the data structure that contains the argument addr */
void fbt_ids_unlockat(struct thread_local_data *tld, void *addr)
{
    struct alloc_chunk *chunk
            = (struct alloc_chunk*) rb_query(tld->mem_alloc.chunks_tree, addr);
    if (NULL != chunk) {
        fbt_ids_unlock_helper(tld, chunk);
    }
}

/* write-protects all data structures */
void fbt_ids_lockdown(struct thread_local_data *tld)
{
    struct alloc_chunk *chunk = tld->mem_alloc.chunks;
    void *span_begin = NULL;
    void *span_end = NULL;
    char span_flags = 0;
    unsigned int qlen=0;
    
    while (NULL != chunk) {
        if ((span_end == chunk->node.addr_begin)
                        && (span_flags == chunk->flags)) {
            // we can append to the existing span
            span_end = chunk->node.addr_end;
        } else {
            // close the current span and unlock it
            if (NULL != span_begin) {
                int flags = 0;
                if (0 == span_flags) {
                    flags = PROT_NONE;
                } else {
                    if (INFO_RFLAG & span_flags) {
                        flags |= PROT_READ;
                    }
                    if (INFO_XFLAG & span_flags) {
                        flags |= PROT_EXEC;
                    }
                }
                mprotect(span_begin, span_end - span_begin, flags);
            }
            // open the new span
            span_begin = chunk->node.addr_begin;
            span_end = chunk->node.addr_end;
            span_flags = chunk->flags;
        }
        chunk = chunk->next;
	qlen++;
    }
    if (NULL != span_begin) {
        int flags = 0;
        if (0 == span_flags) {
            flags = PROT_NONE;
        } else {
            if (INFO_RFLAG & span_flags) {
                flags |= PROT_READ;
            }
            if (INFO_XFLAG & span_flags) {
                flags |= PROT_EXEC;
            }
        }
        mprotect(span_begin, span_end - span_begin, flags);
    }

    INFO_LLPRINTF_VERBOSE("locked down %d allocation blocks\n", qlen);
}


/**
 * make an internal data structure executable.
 * This function sets the executable flag in the corresponding alloc_chunk
 * struct and makes the memory of the data structure that contains addr
 * executable.
 * @param tld the thread local data pointer
 * @param addr the address where memory shall be executable
 */
void fbt_ids_setexec(struct thread_local_data *tld, void *addr)
{
    struct alloc_chunk *chunk
            = (struct alloc_chunk*) rb_query(tld->mem_alloc.chunks_tree, addr);
    if (NULL == chunk) {
        return;
    }

    chunk->flags |= INFO_XFLAG;

    mprotect(chunk->node.addr_begin,
              chunk->node.addr_end - chunk->node.addr_begin,
              PROT_READ | PROT_WRITE | PROT_EXEC);

}
