#include "threads.h"

#define NDEBUG

#ifndef NDEBUG
int main(void)
{
    int argc = 3;
    char argv_buf[3][32] = { "main", "./tests/blink.bin", "./tests/blink.bin" };
    char* argv[] = { argv_buf[0], argv_buf[1], argv_buf[2] };
#else
int main(int argc, char *argv[])
{
#endif

    launch_threads(argc, argv);

    return 0;
}

