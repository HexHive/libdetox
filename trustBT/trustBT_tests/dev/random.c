#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

int main()
{
    int ptr;
    long val;
    int rnd;
    struct timeval tv;
    if (-1 == gettimeofday(&tv, NULL)) {
        perror("Could not use time as rand seed");
    }

    srand(1);

    val = 436736000;

    ptr = rand_r(&val);
    //ptr = (void*) (rand_r(&(tv.tv_usec)) & (~(sysconf(_SC_PAGESIZE) - 1)));
    printf("random value: %d\n", ptr);

    rnd = rand();
    printf("rand: %d\n", rnd);

    return 0;
}