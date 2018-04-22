#include "gui.h"
#include "attiny13.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    configure_gui(argc, argv);

    printf("Hello AVR Ximul!\n");

    return 0;
}

