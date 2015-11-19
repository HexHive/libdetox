#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void (*str_func)(char*);

const char code[] = "\x8b\x4c\x24\x04\x31\xd2\x31\xc0\x31\xdb\xb2\x1c\xb0\x04\xb3\x01\xcd\x80\xc3";
char *rodata_string = "code in rodata was executed\n";
char *heap_string = "code on heap was executed  \n";

void call_on_heap()
{
    str_func func = (str_func) malloc(strlen(code) + 1);
    strcpy((char*) func, code);
    func(heap_string);
}

void call_in_rodata()
{
    str_func func = (str_func) code;
    func(rodata_string);
}

int main(int argc, char **argv)
{
    call_in_rodata();
    call_on_heap();
    printf("end of program\n");
    return 0;
}
