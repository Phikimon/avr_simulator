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

DO_FUNC(AND,
{
})

DO_FUNC(ADD,
{
})

DO_FUNC(SUB,
{
})

DO_FUNC(CP,
{
})

DO_FUNC(MOV,
{
})

DO_FUNC(PUSH,
{
})

DO_FUNC(POP,
{
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
