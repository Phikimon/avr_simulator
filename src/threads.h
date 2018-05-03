#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <gtk/gtk.h>
#include "gui_pins.h"

enum {
    FIRST_CHIP  = 0,
    SECOND_CHIP = 1,
    CHIPS_NUM,
};

enum {
    WAIT  = 0,
    CHECK = IPC_NOWAIT
};

enum { SEMBUF_SIZE = 16 };

struct simulator_s {
    GtkWindow* window;
    int pins_connections [ATTINY_PINS_NUM];
    struct attiny13* chips[CHIPS_NUM];
};

// Global, used in gui_callbacks.c
extern struct simulator_s simulator;


void simulator_step(long int step_num);
void launch_threads(void);

#undef SEM_PUSH
#undef SEM_FLUSH


