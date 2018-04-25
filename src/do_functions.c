#include "attiny13.h"
#include "assert.h"


int is_reserved(int8_t offset);



#define DO_FUNC(INSTR_NAME, CODE)                   \
int do_##INSTR_NAME(struct attiny13* chip,          \
                    struct cmd* instr)              \
{                                                   \
    instr->progress++;                              \
    CODE;                                           \
    if (instr->progress == instr->duration)         \
        instr->progress = 0;                        \
    return ERR_SUCCESS;                             \
}

DO_FUNC(HANDLE_INTERRUPT,
{
    if (instr->progress == 2)
        attiny13_push_pc(chip);
    else if (instr->progress == 4)
        chip->PC = instr->args.arg[0];      // Jump to interrupt vector
})

int is_reserved(int8_t offset) {
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

#define __Rd chip->registers[instr->args.arg[1]]
#define __Rr chip->registers[instr->args.arg[0]]
#define __A  instr->args.arg[0]
#define __b  instr->args.arg[1]
#define __k  instr->args.addr

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
    if (instr->progress == 1)
        return ERR_SUCCESS;
    if (!is_reserved(__A))
        chip->io_registers[__A] &= ~_BV(__b);
    chip->PC++;
})

DO_FUNC(SBI,
{
    if (instr->progress == 1)
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
        chip->SREG &= _BV(SREG_Z);

    if (__Rd < 0) {                        // S = N ^ V, V = 0 => S = N
        chip->SREG |= _BV(SREG_N);
        chip->SREG |= _BV(SREG_S);
    } else {
        chip->SREG &= _BV(SREG_N);
        chip->SREG &= _BV(SREG_S);
    }

    chip->SREG &= _BV(SREG_V);

    chip->PC++;
})
DO_FUNC(ADD,
{
    int Rd7 = (__Rd >> 7);
    int Rr7 = (__Rr >> 7);
    __Rd = __Rd + __Rr;
    int R7 = (__Rd >> 7);
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

DO_FUNC(PUSH,
{
    if (instr->progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL <= (REGISTERS_NUM + IO_REGISTERS_NUM))
        return ERR_STACK_OVERFLOW;
    chip->data_memory[chip->SPL--] = __Rr;
    chip->PC++;
})

DO_FUNC(POP,
{
    if (instr->progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL == DATA_MEMORY_SIZE - 1)
       return ERR_EMPTY_STACK;
    __Rr = chip->data_memory[(chip->SPL)++];
    chip->PC++;
})

#define DO_CONDITIONAL_BRANCH(CONDITION)                        \
    do {                                                        \
        if (instr->progress == 1 && !(CONDITION)) {             \
            instr->duration = 1;    /* No branch */             \
            chip->PC++;                                         \
            return ERR_SUCCESS;                                 \
        }                                                       \
        chip->PC = (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;    \
        return ERR_SUCCESS;                                     \
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

DO_FUNC(CPI,
{
})

DO_FUNC(LDI,
{
})

DO_FUNC(NOP,
{
})

DO_FUNC(RET,
{
})

DO_FUNC(RETI,
{
    //Не забудь выставить I в SREG!
})

#undef DO_FUNC
#undef __Rd
#undef __Rr
#undef __A
#undef __b
#undef __k

