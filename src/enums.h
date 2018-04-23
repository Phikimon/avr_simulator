#ifndef ENUMS_H
#define ENUMS_H

enum pin_state {
    PIN_LOW = 0,
    PIN_HIGH,
    PIN_HI_Z,
};


// SIZES CONSTANTS
enum { REGISTERS_NUM       = 32 }; // 0x20
enum { IO_REGISTERS_NUM    = 64 }; // 0x40
enum { SRAM_SIZE           = 64 }; // 0x40
enum { DATA_MEMORY_SIZE    = REGISTERS_NUM    +
                             IO_REGISTERS_NUM +
                             SRAM_SIZE };
enum { FLASH_MEMORY_SIZE   = 0x0200 }; // 1024
enum { PIN_NUM             = 6 };
// OFFSETS CONSTANTS
enum { REGISTERS_OFFSET    = 0             };
enum { IO_REGISTERS_OFFSET = REGISTERS_NUM };
enum { SRAM_OFFSET         = REGISTERS_NUM    +
                             IO_REGISTERS_NUM };

#define IO_REGISTER(NAME, OFFSET) \
enum { NAME##_OFFSET = OFFSET };
#define RESERVED_REGISTER(OFFSET)

#include "registers_list.h"

#undef RESERVED_REGISTER
#undef IO_REGISTER

enum ERR {
    ERR_SUCCESS = 0,
    ERR_INTERRUPT,
    ERR_INVALID_OPCODE
};

enum SREG_BITS {
    SREG_C = 0,
    SREG_Z = 1,
    SREG_N = 2,
    SREG_V = 3,
    SREG_S = 4,
    SREG_H = 5,
    SREG_T = 6,
    SREG_I = 7
};

#endif
