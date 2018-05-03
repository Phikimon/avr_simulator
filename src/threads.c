//#include "gui.h"
#include "attiny13.h"
#include "threads.h"
#include "gui.h"
#include <stdlib.h>
#include <stdio.h>

// Declarations
static void semop_push(struct sembuf* sbuf,
                       int*           sbuf_counter,
                       unsigned short num, short val, short flag);

static void semop_flush(struct sembuf* sbuf,
                        int*           sbuf_counter);

static void* chip_thread(void* chip_ptr);
static void* gui_thread(void* chips_ptr);
static void refresh_pins_connections(void);

struct simulator_s simulator;

static int semid = 0;

#define CREATE_THREAD(PTHREAD_PTR, FUNC, ARG)                   \
do {                                                            \
    int ptc_ret = pthread_create(PTHREAD_PTR, NULL, FUNC, ARG); \
    if (ptc_ret == -1) {                                        \
        perror("pthread");                                      \
        exit(1);                                                \
    }                                                           \
} while (0)

// Definitions

/* Public */

void launch_threads(void)
{
    pthread_t        gui, chip_pthreads[CHIPS_NUM];
    struct attiny13  chips[CHIPS_NUM] = {0};

    semid = semget(IPC_PRIVATE, CHIPS_NUM, IPC_CREAT|0666);
    if (semid == -1) {
        perror("semget");
        exit(1);
    }

    for (int i = 0; i < CHIPS_NUM; i++) {
        chips[i].number = i;
        CREATE_THREAD(&chip_pthreads[i], chip_thread, (void*)&chips[i]);
    }
    CREATE_THREAD(&gui, gui_thread, (void*)chips);

    pthread_join(gui, NULL);
    for (int i = 0; i < CHIPS_NUM; i++) {
        pthread_join(chip_pthreads[0], NULL);
    }
}

#undef CREATE_THREAD

#define SEM_PUSH(NUM, VAL, FLAG)    \
do {                                \
    semop_push(sbuf, &sbuf_counter, \
               NUM, VAL, FLAG);     \
} while (0)

#define SEM_FLUSH()                   \
do {                                  \
    semop_flush(sbuf, &sbuf_counter); \
} while (0)

#define SEM_NOT_LESS_THAN(NUM, VAL, FLAG) \
do {                                      \
    SEM_PUSH(NUM, -(VAL), FLAG);          \
    SEM_PUSH(NUM, +(VAL), 0);             \
} while (0)

#define SEM_EQUAL_TO(NUM, VAL, FLAG) \
do {                                 \
    SEM_PUSH(NUM, -(VAL), 0);        \
    SEM_PUSH(NUM,      0, FLAG);     \
    SEM_PUSH(NUM, +(VAL), 0);        \
} while (0)

void simulator_step(long int step_num)
{
    struct sembuf sbuf[SEMBUF_SIZE] = {0};
    int sbuf_counter = 0;

    refresh_pins_connections();
    for (int i = 0; i < step_num; i++) {

        SEM_PUSH(FIRST_CHIP,  +1, 0);
        SEM_PUSH(SECOND_CHIP, +1, 0);
        SEM_FLUSH();
        // Just waiting for threads to interpret tick
        SEM_PUSH(FIRST_CHIP,  0, 0);
        SEM_PUSH(SECOND_CHIP, 0, 0);
        SEM_FLUSH();

        //gui_dump_memory();
        //gui_dump_registers();

        //refresh_pins_connections();
        gui_refresh_pins_states();
        //dump_pins_states();
    }
}

/* Private */

static void semop_push(struct sembuf* sbuf,
                       int*           sbuf_counter,
                       unsigned short num, short val, short flag)
{
    sbuf[*sbuf_counter].sem_op  = val;
    sbuf[*sbuf_counter].sem_flg = flag;
    sbuf[*sbuf_counter].sem_num = num;
    (*sbuf_counter)++;
}

static void semop_flush(struct sembuf* sbuf,
                        int*           sbuf_counter)
{
    int semop_ret = semop(semid, sbuf, *sbuf_counter);
    if (semop_ret == -1) {
        perror("semop");
        exit(1);
    }
    *sbuf_counter = 0;
}

static void* chip_thread(void* chip_ptr)
{
    struct sembuf sbuf[SEMBUF_SIZE] = {0};
    int sbuf_counter = 0;
    struct attiny13* chip = chip_ptr;
    int number = chip->number;

    while (1) {
        SEM_EQUAL_TO(number, +1, WAIT);
        SEM_FLUSH();

        int e;
        if ((e = execute_cycle(chip)) != ERR_SUCCESS) {
            printf_gui(simulator.window,
                       "Error in execute_cycle: %d", e);
        }

        SEM_PUSH(number, -1, 0);
        SEM_FLUSH();
    }

    pthread_exit(0);
}


static void* gui_thread(void* chips_ptr)
{
    struct attiny13* chips = (struct attiny13*)chips_ptr;

    for (int i = 0; i < ATTINY_PINS_NUM; i++) {
        simulator.pins_connections[i] = PIN_NC;
    }
    // Please note, 'simulator.chips = chips;' won't work
    for (int i = 0; i < CHIPS_NUM; i++) {
        simulator.chips[i]  = &chips[i];
    }

    char program_name[] = "avr_ximulator";
    char* program_name_ptr = ((char*)program_name);
    simulator.window = (GtkWindow*)gui_configure(1, &program_name_ptr);

    gtk_main();

    pthread_exit(0);
}

// Private

static void refresh_pins_connections(void)
{
    //Refresh PINB
    //Put error if short-circuit
    for (int i = 0; i < ATTINY_PINS_NUM; i++) {

    }
}

#undef SEM_PUSH
#undef SEM_FLUSH
#undef NOT_LESS_THAN
#undef EQUAL_TO
