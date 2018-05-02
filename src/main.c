#include "gui.h"
#include "attiny13.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    gui_configure(argc, argv);

    printf("Hello AVR Ximul!\n");

    return 0;
}

