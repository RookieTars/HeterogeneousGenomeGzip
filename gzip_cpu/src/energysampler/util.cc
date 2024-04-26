#include "utils.h"
#include <ctime>
#include <cstdio>

long getTimeNs()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void o(int n)
{
    printf("%d\n", n);
    fflush(stdout);
}
