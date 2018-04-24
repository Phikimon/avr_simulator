#include "attiny13.h"

#define DO_FUNC(INSTR_NAME, CODE)                   \
int do_##INSTR_NAME(struct attiny13* chip,          \
                    struct cmd* instr)              \
{                                                   \
    CODE;                                           \
    if (instr->progress == INSTR_NAME##_DURATION) { \
        instr->progress = 0;                        \
        instr->is_finished = 1;                     \
    } else {                                        \
        instr->progress++;                          \
    }                                               \
    return ERR_SUCCESS;                             \
}

DO_FUNC(HANDLE_INTERRUPT,
{
    if (instr->progress == 1)
        attiny13_push_pc(chip);
    else if (instr->progress == 3)
        chip->PC = instr->args.arg[0];      // Jump to interrupt vector
})

DO_FUNC(IN,
{
})

DO_FUNC(OUT,
{
})

DO_FUNC(CBI,
{
})

DO_FUNC(SBIS,
{
})
#define Rd chip->registers[instr->args.arg[1]]
#define Rr chip->registers[instr->args.arg[0]]
DO_FUNC(AND,
{
    Rd = Rd & Rr;

    if(Rd == 0)
        chip->SREG |= _BV(SREG_Z);
    else
        chip->SREG &= _BV(SREG_Z);

    if(Rd < 0) {                        // S = N ^ V, V = 0 => S = N
        chip->SREG |= _BV(SREG_N);
        chip->SREG |= _BV(SREG_S);
    } else {
        chip->SREG &= _BV(SREG_N);
        chip->SREG &= _BV(SREG_S);
    }

    chip->SREG &= _BV(SREG_V);


})
DO_FUNC(ADD,
{
    int Rd7 = (Rd >> 7);
    int Rr7 = (Rr >> 7);
    Rd = Rd + Rr;
    int R7 = (Rd >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7);
    SREG_bits[SREG_Z] = !Rd;
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
})

DO_FUNC(SUB,
{
    int Rd7 = (Rd >> 7);
    int Rr7 = (Rr >> 7);
    Rd = Rd - Rr;
    int R7 = (Rd >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7);
    SREG_bits[SREG_Z] = !Rd;
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }
})

DO_FUNC(CP,
{
    int Rd7 = (Rd >> 7);
    int Rr7 = (Rr >> 7);
    int R7 = ((Rd - Rr) >> 7);
    int SREG_bits[5] = {0};
    SREG_bits[SREG_C] = (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7);
    SREG_bits[SREG_Z] = !(Rd - Rr);
    SREG_bits[SREG_N] = R7;
    SREG_bits[SREG_V] = (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7);
    SREG_bits[SREG_S] = SREG_bits[SREG_N] ^ SREG_bits[SREG_V];
    for (int i = 0; i < 5; i++) {
        if (SREG_bits[i])
            chip->SREG |= _BV(i);
        else
            chip->SREG &= ~_BV(i);
    }

})

DO_FUNC(MOV,
{
    Rd = Rr;
})

DO_FUNC(PUSH,
{
    if(chip->SPL <= (REGISTERS_NUM + IO_REGISTERS_NUM))
        return ERR_STACK_OVERFLOW;
    chip->data_memory[chip->SPL--] = Rr;
})

DO_FUNC(POP,
{
    if(chip->SPL == DATA_MEMORY_SIZE - 1)
       return ERR_EMPTY_STACK;
    Rr = chip->data_memory[(chip->SPL)++];
})

DO_FUNC(BREQ,
{
})

DO_FUNC(BRNE,
{
})

DO_FUNC(BRGE,
{
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
#undef Rd
#undef Rr
