#ifndef ATTINY13_H
#define ATTINY13_H
#include "enums.h"
#include <stdint.h>

struct attiny13 {
/* Public  */
    enum pin_state pin_values[PIN_NUM];
    uint16_t       adc_value;
/* Private */
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
    uint8_t* data_memory, *io_registers;
    uint8_t  registers[REGISTERS_NUM];

#define IO_REGISTER(NAME, OFFSET) \
    uint8_t  NAME;
#define RESERVED_REGISTER(OFFSET) \
    uint8_t __reserved_##OFFSET##__;

#include "registers_list.h"

#undef IO_REGISTER
#undef RESERVED_REGISTER

    uint8_t  sram[SRAM_SIZE];
    /* Data Memory end */

    /* Flash memory: 1024 bytes */
    uint8_t  flash_memory[FLASH_MEMORY_SIZE];
};

#endif
