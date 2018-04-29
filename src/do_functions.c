#include "attiny13.h"
#include "assert.h"

static int is_reserved(int8_t offset);


#define DO_FUNC(INSTR_NAME, CODE)                   \
int do_##INSTR_NAME(struct attiny13* chip)          \
{                                                   \
    chip->cmd.progress++;                           \
    CODE;                                           \
    if (chip->cmd.progress == chip->cmd.duration)   \
        chip->cmd.progress = 0;                     \
    return ERR_SUCCESS;                             \
}

DO_FUNC(HANDLE_INTERRUPT,
{
    if (chip->cmd.progress == 2)
        attiny13_push_pc(chip);
    else if (chip->cmd.progress == 4)
        chip->PC = chip->cmd.args.arg[0];      // Jump to interrupt vector
})

static int is_reserved(int8_t offset) {
#define IO_REGISTER(NAME, OFFSET)   \
    case OFFSET:                    \
        return 0;
#define RESERVED_REGISTER(OFFSET)   \
    case OFFSET:                    \
        return 1;
    switch (offset) {
#include "registers_list.h"
#undef IO_REGISTER
#undef RESERVED_REGISTER
        default:
            assert(!"Wrong xyuo-mayo");
    }
}

#define __Rd chip->registers[chip->cmd.args.arg[1]]
#define __Rr chip->registers[chip->cmd.args.arg[0]]
#define __A  chip->cmd.args.arg[0]
#define __b  chip->cmd.args.arg[1]
#define __k  chip->cmd.args.addr
#define __K  chip->cmd.args.arg[0]

DO_FUNC(IN,
{
    if (is_reserved(__A))
        __Rd = 0;
    else
        __Rd = chip->io_registers[__A];
    chip->PC++;
})

DO_FUNC(OUT,
{
    if (!is_reserved(__A))
        chip->io_registers[__A] = __Rd;
    chip->PC++;
})


DO_FUNC(CBI,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (!is_reserved(__A))
        chip->io_registers[__A] &= ~_BV(__b);
    chip->PC++;
})

DO_FUNC(SBI,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (!is_reserved(__A))
        chip->io_registers[__A] |= _BV(__b);
    chip->PC++;
})

DO_FUNC(AND,
{
    __Rd = __Rd & __Rr;

    if (__Rd == 0)
        chip->SREG |= _BV(SREG_Z);
    else
        chip->SREG &= ~_BV(SREG_Z);

    if (__Rd < 0) {                        // S = N ^ V, V = 0 => S = N
        chip->SREG |= _BV(SREG_N);
        chip->SREG |= _BV(SREG_S);
    } else {
        chip->SREG &= ~_BV(SREG_N);
        chip->SREG &= ~_BV(SREG_S);
    }

    chip->SREG &= ~_BV(SREG_V);     // V = 0

    chip->PC++;
})

DO_FUNC(OR,
{
    __Rd = __Rd | __Rr;

    if (__Rd == 0)
        chip->SREG |= _BV(SREG_Z);
    else
        chip->SREG &= ~_BV(SREG_Z);

    if (__Rd < 0) {                        // S = N ^ V, V = 0 => S = N
        chip->SREG |= _BV(SREG_N);
        chip->SREG |= _BV(SREG_S);
    } else {
        chip->SREG &= ~_BV(SREG_N);
        chip->SREG &= ~_BV(SREG_S);
    }

    chip->SREG &= ~_BV(SREG_V);     // V = 0

    chip->PC++;
})

DO_FUNC(NEG,
{
    __Rd = - __Rd;
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = !!__Rd;
    SREG_bits[SREG_Z] = !__Rd;
    SREG_bits[SREG_N] = __Rd << 7;
    SREG_bits[SREG_V] = (__Rd == 0x80); // -128
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})


DO_FUNC(ADD,
{
    int Rd7 = (__Rd >> 7);
    int Rr7 = (__Rr >> 7);
    __Rd = __Rd + __Rr;
    int R7 = (__Rd >> 7);                                           // WRONG FLAGS QT ( H FLAG)
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7);
    SREG_bits[SREG_Z] = !__Rd;
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})

DO_FUNC(SUB,
{
    int Rd7 = (__Rd >> 7);
    int Rr7 = (__Rr >> 7);
    __Rd = __Rd - __Rr;
    int R7 = (__Rd >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7);
    SREG_bits[SREG_Z] = !__Rd;
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})

DO_FUNC(DEC,
{
    __Rd = __Rd - 1;
    int SREG_bits[5] = {0};
    SREG_bits[SREG_Z] = !__Rd;
    SREG_bits[SREG_N] = __Rd >> 7;
    SREG_bits[SREG_V] = (__Rd == 0x7F); // 127
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = SREG_Z; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})

DO_FUNC(INC,
{
    __Rd = __Rd + 1;
    int SREG_bits[5] = {0};
    SREG_bits[SREG_Z] = !__Rd;
    SREG_bits[SREG_N] = __Rd >> 7;
    SREG_bits[SREG_V] = (__Rd == 0x80); // -128
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = SREG_Z; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})


DO_FUNC(CP,
{
    int Rd7 = (__Rd >> 7);
    int Rr7 = (__Rr >> 7);
    int R7 = ((__Rd - __Rr) >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7);
    SREG_bits[SREG_Z] = !(__Rd - __Rr);
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})

DO_FUNC(MOV,
{
    __Rd = __Rr;
    chip->PC++;
})

DO_FUNC(CLR,
{
    __Rd = 0;
    chip->SREG |= _BV(SREG_Z);
    chip->SREG &= ~(_BV(SREG_N) | _BV(SREG_V) | _BV(SREG_S));
    chip->PC++;
})

DO_FUNC(SER,
{
    __Rd = 0xFF;
    chip->PC++;
})
DO_FUNC(PUSH,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM))
        return ERR_STACK_OVERFLOW;
    chip->data_memory[chip->SPL--] = __Rr;
    chip->PC++;
})

DO_FUNC(POP,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL >= DATA_MEMORY_SIZE - 1)
       return ERR_EMPTY_STACK;
    __Rr = chip->data_memory[++chip->SPL];
    chip->PC++;
})

DO_FUNC(RCALL,
{
    if(chip->cmd.progress < 3)
        return ERR_SUCCESS;

    if(chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM + 1))
        return ERR_STACK_OVERFLOW;
    *(uint16_t*)(chip->data_memory + chip->SPL - 1) = chip->PC + 1;
    chip->SPL -= 2;
    chip->PC += (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;
})

DO_FUNC(RJMP,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;

    chip->PC += (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;
})


#define DO_CONDITIONAL_BRANCH(CONDITION)                            \
    do {                                                            \
        if (chip->cmd.progress == 1 && !(CONDITION)) {              \
            chip->cmd.duration = 1;    /* No branch */              \
            chip->PC++;                                             \
        }                                                           \
        else if (chip->cmd.progress == 2)                           \
            chip->PC = (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;    \
    } while (0)


DO_FUNC(BREQ,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_Z);
})

DO_FUNC(BRNE,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_Z == 0);
})

DO_FUNC(BRGE,
{
    DO_CONDITIONAL_BRANCH((GET_FLAG_N ^ GET_FLAG_V) == 0);
})

DO_FUNC(BRCS,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_C == 1);
})

DO_FUNC(BRCC,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_C == 0);
})

DO_FUNC(BRSH,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_C == 0);
})

DO_FUNC(BRLO,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_C == 1);
})

DO_FUNC(BRMI,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_N == 1);
})

DO_FUNC(BRPL,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_N == 0);
})

DO_FUNC(BRLT,
{
    DO_CONDITIONAL_BRANCH((GET_FLAG_N ^ GET_FLAG_V) == 1);
})

DO_FUNC(BRTS,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_T == 1);
})

DO_FUNC(BRTC,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_T == 0);
})

DO_FUNC(BRVS,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_V == 1);
})

DO_FUNC(BRVC,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_V == 0);
})

DO_FUNC(BRIE,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_I == 1);
})

DO_FUNC(BRID,
{
    DO_CONDITIONAL_BRANCH(GET_FLAG_I == 0);
})

DO_FUNC(CPI,
{
    int Rd7 = (__Rd >> 7);
    int K7 = (__K >> 7);
    int R7 = ((__Rd - __K) >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7);
    SREG_bits[SREG_Z] = !(__Rd - __K);
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
    chip->PC++;
})

DO_FUNC(LDI,
{
    __Rd = __K;
    chip->PC++;
})

DO_FUNC(NOP,
{
    chip->PC++;
})

DO_FUNC(RET,
{
    if(chip->cmd.progress < 4)
        return ERR_SUCCESS;
    if(chip->SPL >= DATA_MEMORY_SIZE - 3)
        return ERR_EMPTY_STACK;
    chip->PC = *(uint16_t*)(chip->data_memory + chip->SPL + 1);
    chip->SPL += 2;
})

DO_FUNC(RETI,
{
    if(chip->cmd.progress < 4)
        return ERR_SUCCESS;
    if(chip->SPL >= DATA_MEMORY_SIZE - 3)
        return ERR_EMPTY_STACK;
    chip->PC = *(uint16_t*)(chip->data_memory + chip->SPL + 1);
    chip->SPL += 2;
    chip->SREG |= _BV(SREG_I);
})

DO_FUNC(IJMP,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;
    chip->PC = Z_ADDR;
})

DO_FUNC(SEI,
{
    chip->SREG |= _BV(SREG_I);
    chip->PC++;
})

DO_FUNC(CLI,
{
    chip->SREG &= ~_BV(SREG_I);
    chip->PC++;
})

DO_FUNC(LD_X,
{
    __Rd = chip->data_memory[X_ADDR % DATA_MEMORY_SIZE];
    chip->PC++;
})

DO_FUNC(LD_Y,
{
    __Rd = chip->data_memory[Y_ADDR % DATA_MEMORY_SIZE];
    chip->PC++;
})

DO_FUNC(LD_Z,
{
    __Rd = chip->data_memory[Z_ADDR % DATA_MEMORY_SIZE];
    chip->PC++;
})

DO_FUNC(ST_X,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;
    chip->data_memory[X_ADDR % DATA_MEMORY_SIZE] = __Rd;
    chip->PC++;
})

DO_FUNC(ST_Y,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;
    chip->data_memory[Y_ADDR % DATA_MEMORY_SIZE] = __Rd;
    chip->PC++;
})

DO_FUNC(ST_Z,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;
    chip->data_memory[Z_ADDR % DATA_MEMORY_SIZE] = __Rd;
    chip->PC++;
})

DO_FUNC(LPM,
{
    if(chip->cmd.progress < 3)
        return ERR_SUCCESS;
    __Rd = ((uint8_t*)chip->flash_memory)[Z_ADDR % FLASH_MEMORY_SIZE];
    chip->PC++;
})






#undef DO_FUNC
#undef __Rd
#undef __Rr
#undef __A
#undef __b
#undef __k
