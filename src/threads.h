#include "gui_pins.h"
#include "attiny13.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <gtk/gtk.h>
#include <stdint.h>

enum {
    FIRST_CHIP  = 0,
    SECOND_CHIP = 1,
    CHIPS_NUM,

    INTERACT_SEM = 2,
    LOCK_SEM = 3,
    SEM_NUM = 4
};

enum {
    WAIT  = 0,
    CHECK = IPC_NOWAIT
};

enum { SEMBUF_SIZE = 16 };

struct simulator_s {
    GtkWindow* window;
    uint32_t pins_conn_mask[DOCK_PINS_NUM];
    struct attiny13* chips[CHIPS_NUM];
    int is_inited;
};

// Global, used in gui_callbacks.c
extern struct simulator_s simulator;

void simulator_step(long int step_num);
void simulator_init(void);

void launch_threads(int argc, char *argv[]);

void
shared_printf(const char* format, ...)
__attribute__((format(printf,1,2)));

void acquire_lock(void);
void release_lock(void);

#undef SEM_PUSH
#undef SEM_FLUSH


