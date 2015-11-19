#include <stdio.h>
#include "../../libstm-svn/libstm/src/fbt_translate.h"

//void *translate_noexecute(thread_local_data_t *tld, void *tu_address)
void * __wrap_translate_noexecute(void *tld, void *tu_address)
{
    printf ("translate_noexecute called %x\n", tu_address);
    return __real_translate_noexecute(tld, tu_address);
}

int main()
{
 printf("main()\n");
 return 0;
}