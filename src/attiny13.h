#ifndef ATTINY13_H
#define ATTINY13_H
#include "common.h"
#include <stdint.h>

struct attiny13 {
    /*
       Data Memory:
       1. 32 general purpose registers
       2. 64 IO registers(some are reserved)
       3. 64 bytes of SRAM
       They can be accessed via:
       0. Any - 'data_memory'  pointer
       1. GPR - 'registers'    pointer
       2. IOR - 'io_registers' pointer with
       NAME_OFFSET offset, also directly by
       name as variables
       3. SRAM - 'sram' pointer
     */
    int8_t* data_memory;
    uint8_t* io_registers;
    int8_t  registers[REGISTERS_NUM];

#define IO_REGISTER(NAME, OFFSET) \
    uint8_t  NAME;
#define RESERVED_REGISTER(OFFSET) \
    uint8_t __reserved_##OFFSET##__;

#include "registers_list.h"

#undef IO_REGISTER
#undef RESERVED_REGISTER

    int8_t  sram[SRAM_SIZE];
    /* Data Memory end */

    /* Flash memory: 1024 bytes */
    int8_t  flash_memory[FLASH_MEMORY_SIZE];
    uint16_t PC;
};

struct cmd {
    union cmd_args {
        int8_t arg[2];
        uint16_t addr;
    } args;
    int progress;
    int is_finished;
    int (*func)(struct attiny13* chip, struct cmd* instr);
};

/* Public */
    int attiny13_ctor           (struct attiny13* chip);
    int execute_cycle           (struct attiny13* chip);
/* Private */
    int check_interrupt         (struct attiny13* chip, struct cmd* instr);
    int refresh_interrupt_flags (struct attiny13* chip);
    int decode                  (struct attiny13* chip, struct cmd* instr, int16_t cmd);
    int do_HANDLE_INTERRUPT     (struct attiny13* chip, struct cmd* instr);
    int attiny13_push_pc        (struct attiny13* chip);


int do_IN   (struct attiny13* chip, struct cmd* instr);
int do_OUT  (struct attiny13* chip, struct cmd* instr);

int do_CBI  (struct attiny13* chip, struct cmd* instr);
int do_SBIS (struct attiny13* chip, struct cmd* instr);

int do_AND  (struct attiny13* chip, struct cmd* instr);
int do_ADD  (struct attiny13* chip, struct cmd* instr);
int do_SUB  (struct attiny13* chip, struct cmd* instr);
int do_CP   (struct attiny13* chip, struct cmd* instr);
int do_MOV  (struct attiny13* chip, struct cmd* instr);

int do_PUSH (struct attiny13* chip, struct cmd* instr);
int do_POP  (struct attiny13* chip, struct cmd* instr);

int do_BREQ (struct attiny13* chip, struct cmd* instr);
int do_BRNE (struct attiny13* chip, struct cmd* instr);
int do_BRGE (struct attiny13* chip, struct cmd* instr);

int do_CPI  (struct attiny13* chip, struct cmd* instr);
int do_LDI  (struct attiny13* chip, struct cmd* instr);

int do_NOP  (struct attiny13* chip, struct cmd* instr);
int do_RET  (struct attiny13* chip, struct cmd* instr);
int do_RETI (struct attiny13* chip, struct cmd* instr);


#endif
