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
            return 0;
    }
}

#define __d  chip->cmd.args.arg[1]
#define __Rd chip->registers[__d]    // Destination (and source) register in the Register File
#define __r  chip->cmd.args.arg[0]
#define __Rr chip->registers[__r]    // Source register in the Register File
#define __K  chip->cmd.args.arg[0]   // Constant data
#define __k  chip->cmd.args.arg[0]   // Constant address
#define __A  chip->cmd.args.arg[0]   // I/O location address
#define __b  chip->cmd.args.arg[1]   // Bit in the Register File or I/O Register (3-bit)
#define __s  chip->cmd.args.arg[1]   // Bit in the Status Register (3-bit)
#define __q  chip->cmd.args.arg[0]   // Displacement for direct addressing (6-bit)

#define SET_FLAG(FLAG, CONDITION)    \
do {                                 \
    if(CONDITION)                    \
        chip->SREG |=  _BV(FLAG);    \
    else                             \
        chip->SREG &= ~_BV(FLAG);    \
} while (0)


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

#define DO_LOGIC_INSTRUCTION                                        \
do {                                                                \
    SET_FLAG(SREG_Z, __Rd == 0);                                    \
    SET_FLAG(SREG_N, __Rd < 0);                                     \
    SET_FLAG(SREG_S, __Rd < 0);   /* S = N ^ V, V = 0 => S = N */   \
    chip->SREG &= ~_BV(SREG_V);   /* V = 0 */                       \
    chip->PC++;                                                     \
} while (0)

DO_FUNC(AND,
{
    __Rd &= __Rr;
    DO_LOGIC_INSTRUCTION;
})

DO_FUNC(ANDI,
{
    __Rd &= __K;
    DO_LOGIC_INSTRUCTION;
})


DO_FUNC(OR,
{
    __Rd |= __Rr;
    DO_LOGIC_INSTRUCTION;
})


DO_FUNC(ORI,
{
    __Rd |= __K;
    DO_LOGIC_INSTRUCTION;
})


DO_FUNC(EOR,
{
    __Rd ^= chip->registers[chip->cmd.args.arg[0] & 0x03];   // r <= 3
    DO_LOGIC_INSTRUCTION;
})


DO_FUNC(SBR,
{
    __Rd |= __K;
    DO_LOGIC_INSTRUCTION;
})

DO_FUNC(CBR,
{
    __Rd &= (0xFF - __K);
    DO_LOGIC_INSTRUCTION;
})

#undef DO_LOGIC_INSTRUCTION

DO_FUNC(NEG,
{
    __Rd = - __Rd;

    SET_FLAG(SREG_C, __Rd);
    SET_FLAG(SREG_Z, __Rd == 0);
    SET_FLAG(SREG_N, __Rd < 0);
    SET_FLAG(SREG_V, __Rd == 0x80); // -128
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

DO_FUNC(COM,
{
    __Rd = 0xFF - __Rd;

    chip->SREG |= _BV(SREG_C);
    SET_FLAG(SREG_Z, __Rd == 0);
    SET_FLAG(SREG_N, __Rd < 0);
    chip->SREG &= ~_BV(SREG_N);
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

#define DO_ADD_ADC(CODE)                                        \
do {                                                            \
    int Rd7 = (__Rd >> 7);                                      \
    int Rr7 = (__Rr >> 7);                                      \
    CODE;                                                       \
    int R7 = (__Rd >> 7);                                       \
                                                                \
    SET_FLAG(SREG_C, (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7));  \
    SET_FLAG(SREG_Z, __Rd == 0);                                \
    SET_FLAG(SREG_N, R7);                                       \
    SET_FLAG(SREG_V, (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7 & R7));   \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);                  \
                                                                \
    chip->PC++;                                                 \
} while (0)


DO_FUNC(ADD,
{
    DO_ADD_ADC(__Rd += __Rr);
})

DO_FUNC(ADC,
{
    DO_ADD_ADC(__Rd += __Rr + GET_FLAG_C);
})

#undef DO_ADD_ADC

DO_FUNC(ADIW,
{
    int Rdh7 = chip->registers[__d + 1] >> 7;
    int16_t R = ((int16_t)chip->registers[__d + 1] | (int16_t)__Rd) + __K;
    chip->registers[__d + 1] = R >> 8;
    __Rd = R & 0x0F;
    int R15 = R >> 15;

    SET_FLAG(SREG_C, ~R15 & Rdh7);
    SET_FLAG(SREG_Z, R == 0);
    SET_FLAG(SREG_N, R15);
    SET_FLAG(SREG_V, ~Rdh7 & R15);
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

#define DO_SUB_SBC(CODE)                                        \
do {                                                            \
    int Rd7 = (__Rd >> 7);                                      \
    int Rr7 = (__Rr >> 7);                                      \
    CODE;                                                       \
    int R7 = (__Rd >> 7);                                       \
                                                                \
    SET_FLAG(SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));  \
    SET_FLAG(SREG_Z, __Rd == 0);                                \
    SET_FLAG(SREG_N, R7);                                       \
    SET_FLAG(SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));   \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);                  \
                                                                \
    chip->PC++;                                                 \
} while (0)

DO_FUNC(SUB,
{
    DO_SUB_SBC(__Rd -= __Rr);
})

DO_FUNC(SBC,
{
    DO_SUB_SBC(__Rd -= __Rr + GET_FLAG_C);
})

#undef DO_SUB_SBC

#define DO_SUBI_SBCI(CODE)                                      \
do {                                                            \
    int Rd7 = __Rd >> 7;                                        \
    int K7  = __K  >> 7;                                        \
    CODE;                                                       \
    int R7 = __Rd >> 7;                                         \
                                                                \
    SET_FLAG(SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));    \
    SET_FLAG(SREG_Z, __Rd == 0);                                \
    SET_FLAG(SREG_N, R7);                                       \
    SET_FLAG(SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));     \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);                  \
                                                                \
    chip->PC++;                                                 \
} while (0)


DO_FUNC(SUBI,
{
    DO_SUBI_SBCI(__Rd -= __K);
})

DO_FUNC(SBCI,
{
    DO_SUBI_SBCI(__Rd -= __K + GET_FLAG_C);
})

#undef DO_SUBI_SBCI

DO_FUNC(SBIW,
{
    int Rdh7 = chip->registers[__d + 1] >> 7;
    int16_t R = ((int16_t)chip->registers[__d + 1] | (int16_t)__Rd) - __K;
    chip->registers[__d + 1] = R >> 8;
    __Rd = R & 0x0F;
    int R15 = R >> 15;

    SET_FLAG(SREG_C, R15 & ~Rdh7);
    SET_FLAG(SREG_Z, R == 0);
    SET_FLAG(SREG_N, R15);
    SET_FLAG(SREG_V, R15 & ~Rdh7);
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

DO_FUNC(DEC,
{
    __Rd--;

    SET_FLAG(SREG_Z, __Rd == 0);
    SET_FLAG(SREG_N, __Rd < 0);
    SET_FLAG(SREG_V, __Rd == 0x7F); // 127
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

DO_FUNC(INC,
{
    __Rd++;

    SET_FLAG(SREG_Z, __Rd == 0);
    SET_FLAG(SREG_N, __Rd < 0);
    SET_FLAG(SREG_V, __Rd == 0x80); // -128
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

#define DO_CP_CPC(COMPARISON)                                   \
do {                                                            \
    int Rd7 = (__Rd >> 7);                                      \
    int Rr7 = (__Rr >> 7);                                      \
    int R7 = ((COMPARISON) >> 7);                               \
                                                                \
    SET_FLAG(SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));  \
    SET_FLAG(SREG_Z, __Rd == __Rr);                             \
    SET_FLAG(SREG_N, R7);                                       \
    SET_FLAG(SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));   \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);                  \
                                                                \
    chip->PC++;                                                 \
} while (0)



DO_FUNC(CP,
{
    DO_CP_CPC(__Rd - __Rr);
})

DO_FUNC(CPC,
{
    DO_CP_CPC(__Rd - __Rr - GET_FLAG_C);
})

#undef DO_CP_CPC

DO_FUNC(CPI,
{
    int Rd7 = (__Rd >> 7);
    int K7 = (__K >> 7);
    int R7 = ((__Rd - __K) >> 7);

    SET_FLAG(SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));
    SET_FLAG(SREG_Z, __Rd == __K);
    SET_FLAG(SREG_N, R7);
    SET_FLAG(SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);

    chip->PC++;
})

DO_FUNC(MOV,
{
    __Rd = __Rr;
    chip->PC++;
})


DO_FUNC(MOVW,
{
    *((int16_t*)&__Rd) = *((int16_t*)&__Rr);
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

DO_FUNC(TST,
{

    SET_FLAG(SREG_Z, __Rd == 0);
    SET_FLAG(SREG_N, __Rd < 0);
    SET_FLAG(SREG_S, __Rd < 0);    // S = N ^ V, V = 0 => S = N
    chip->SREG &= ~_BV(SREG_V);    // V = 0

    chip->PC++;
})

#define DO_SHIFT(C_FLAG, RD, N_FLAG)            \
do {                                            \
    SET_FLAG(SREG_C, C_FLAG);                   \
    __Rd = RD;                                  \
    SET_FLAG(SREG_N, N_FLAG);                   \
    SET_FLAG(SREG_Z, __Rd == 0);                \
    SET_FLAG(SREG_V, GET_FLAG_N ^ GET_FLAG_C);  \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);  \
    chip->PC++;                                 \
} while (0)

#define DO_ROTATE(C_FLAG, RD)                   \
do {                                            \
    int old_C = GET_FLAG_C;                     \
    SET_FLAG(SREG_C, C_FLAG);                   \
    __Rd = RD;                                  \
    SET_FLAG(SREG_N, __Rd < 0);                 \
    SET_FLAG(SREG_Z, __Rd == 0);                \
    SET_FLAG(SREG_V, GET_FLAG_N ^ GET_FLAG_C);  \
    SET_FLAG(SREG_S, GET_FLAG_N ^ GET_FLAG_V);  \
    chip->PC++;                                 \
} while (0)


DO_FUNC(LSR,
{
    DO_SHIFT(__Rd & 0x01, __Rd >> 1, 0);
})

DO_FUNC(LSL,
{
    DO_SHIFT(__Rd >> 7, __Rd << 1, __Rd < 0);
})

DO_FUNC(ROL,
{
    DO_ROTATE(__Rd >> 7, (__Rd << 1) | old_C);
})

DO_FUNC(ROR,
{
    DO_ROTATE(__Rd & 0x01, (__Rd >> 1) | (old_C << 7));
})

DO_FUNC(ASR,
{
    DO_SHIFT(__Rd & 0x01, (__Rd & 0x80) | (__Rd >> 1), __Rd < 0);
})

#undef DO_SHIFT
#undef DO_ROTATE

DO_FUNC(SWAP,
{
    __Rd = (__Rd >> 4) | (__Rd << 4);
    chip->PC++;
})

DO_FUNC(PUSH,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM))
        return ERR_STACK_OVERFLOW;
    chip->data_memory[chip->SPL--] = __Rd;
    chip->PC++;
})

DO_FUNC(POP,
{
    if (chip->cmd.progress == 1)
        return ERR_SUCCESS;
    if (chip->SPL >= DATA_MEMORY_SIZE - 1)
       return ERR_EMPTY_STACK;
    __Rd = chip->data_memory[++chip->SPL];
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
    chip->PC = (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;
})

DO_FUNC(RJMP,
{
    if(chip->cmd.progress == 1)
        return ERR_SUCCESS;

    chip->PC = (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;
})

#define DO_SCIP(CONDITION)                                              \
do {                                                                    \
    if((chip->cmd.progress < chip->cmd.duration) && !(CONDITION)) {     \
        chip->cmd.duration = 1;     /* No scip */                       \
        chip->PC += 1;                                                  \
    }                                                                   \
    else                                                                \
        chip->PC += 2;                                                  \
} while (0)


DO_FUNC(CPSE,
{
    DO_SCIP(__Rd == __Rr);
})

DO_FUNC(SBRC,
{
    DO_SCIP((__Rr & _BV(__b)) == 0);
})

DO_FUNC(SBRS,
{
    DO_SCIP(__Rr & _BV(__b));
})

DO_FUNC(SBIC,
{
    DO_SCIP(is_reserved(__A) || ((is_reserved(__A) == 0) && (chip->io_registers[__A] & _BV(__b)) == 0));
})

DO_FUNC(SBIS,
{
    DO_SCIP((is_reserved(__A) == 0) && (chip->io_registers[__A] & _BV(__b)));
})

#undef DO_SCIP

#define DO_CONDITIONAL_BRANCH(CONDITION)                        \
do {                                                            \
    if (chip->cmd.progress == 1 && !(CONDITION)) {              \
        chip->cmd.duration = 1;    /* No branch */              \
        chip->PC++;                                             \
    }                                                           \
    else                                                        \
        chip->PC = (chip->PC + __k + 1) % FLASH_MEMORY_SIZE;    \
} while (0)


DO_FUNC(BRBC,
{
    DO_CONDITIONAL_BRANCH((chip->SREG & _BV(__s)) == 0);
})

DO_FUNC(BRBS,
{
    DO_CONDITIONAL_BRANCH(chip->SREG & _BV(__s));
})

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

DO_FUNC(ICALL,
{
    if(chip->cmd.progress < chip->cmd.duration)
        return ERR_SUCCESS;

    if(chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM + 1))
        return ERR_STACK_OVERFLOW;
    *(uint16_t*)(chip->data_memory + chip->SPL - 1) = chip->PC + 1;
    chip->SPL -= 2;
    chip->PC = Z_ADDR % FLASH_MEMORY_SIZE;
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

DO_FUNC(LDI,
{
    __Rd = __K;
    chip->PC++;
})

#define MODE_UNCHANGED  0
#define MODE_INC        1
#define MODE_DEC        2

#define DO_LD(ADDR, MODE, REG_NUM)                           \
do {                                                         \
    if(chip->cmd.progress < chip->cmd.duration)              \
        return ERR_SUCCESS;                                  \
                                                             \
    if(MODE == MODE_DEC)                                     \
        chip->registers[REG_NUM]--;                          \
    if((REGISTERS_NUM <= ADDR) &&       /* reserved */       \
       (ADDR < REGISTERS_NUM + IO_REGISTERS_NUM) &&          \
       is_reserved(ADDR - REGISTERS_NUM))                    \
        __Rd = 0x00;                                         \
    else                                                     \
        __Rd = chip->data_memory[ADDR % DATA_MEMORY_SIZE];   \
    if(MODE == MODE_INC)                                     \
        chip->registers[REG_NUM]++;                          \
    chip->PC++;                                              \
} while (0)


DO_FUNC(LD_X,
{
    DO_LD(X_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(LD_X_INC,
{
    DO_LD(X_ADDR, MODE_INC, 26);
})

DO_FUNC(LD_X_DEC,
{
    DO_LD(X_ADDR, MODE_DEC, 26);
})

DO_FUNC(LD_Y,
{
    DO_LD(Y_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(LD_Y_INC,
{
    DO_LD(Y_ADDR, MODE_INC, 28);
})

DO_FUNC(LD_Y_DEC,
{
    DO_LD(Y_ADDR, MODE_DEC, 28);
})

DO_FUNC(LD_Z,
{
    DO_LD(Z_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(LD_Z_INC,
{
    DO_LD(Z_ADDR, MODE_INC, 30);
})

DO_FUNC(LD_Z_DEC,
{
    DO_LD(Z_ADDR, MODE_DEC, 30);
})

DO_FUNC(LDD_Y,
{
    DO_LD(Y_ADDR + __q, MODE_UNCHANGED, 0);
})

DO_FUNC(LDD_Z,
{
    DO_LD(Z_ADDR + __q, MODE_UNCHANGED, 0);
})
#undef DO_LD

DO_FUNC(LDS,
{
    uint8_t addr = (__k & 0x0F) | ((__k >> 5) & 0x03) | ((__k << 2) & 0x40) | ((!((__k >> 4) & 0x01)) << 7); // k bits:~4 4 6 5 3 2 1 0
    if((addr < REGISTERS_NUM + IO_REGISTERS_NUM) && is_reserved(addr - REGISTERS_NUM))  // reserved
        __Rd = 0x00;
    else
        __Rd = chip->data_memory[addr % DATA_MEMORY_SIZE];
    chip->PC++;
})

#define DO_ST(ADDR, MODE, REG_NUM)                          \
do {                                                        \
    if(chip->cmd.progress < chip->cmd.duration)             \
        return ERR_SUCCESS;                                 \
    if(MODE == MODE_DEC)                                    \
        chip->registers[REG_NUM]--;                         \
    if(!((REGISTERS_NUM <= ADDR) &&     /* not reserved */  \
       (ADDR < REGISTERS_NUM + IO_REGISTERS_NUM) &&         \
       is_reserved(ADDR - REGISTERS_NUM)))                  \
        chip->data_memory[ADDR % DATA_MEMORY_SIZE] = __Rd;  \
    if(MODE == MODE_INC)                                    \
        chip->registers[REG_NUM]++;                         \
    chip->PC++;                                             \
} while (0)

DO_FUNC(ST_X,
{
    DO_ST(X_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(ST_X_INC,
{
    DO_ST(X_ADDR, MODE_INC, 26);
})

DO_FUNC(ST_X_DEC,
{
    DO_ST(X_ADDR, MODE_DEC, 26);
})

DO_FUNC(ST_Y,
{
    DO_ST(Y_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(ST_Y_INC,
{
    DO_ST(Y_ADDR, MODE_INC, 28);
})

DO_FUNC(ST_Y_DEC,
{
    DO_ST(Y_ADDR, MODE_DEC, 28);
})

DO_FUNC(ST_Z,
{
    DO_ST(Z_ADDR, MODE_UNCHANGED, 0);
})

DO_FUNC(ST_Z_INC,
{
    DO_ST(Z_ADDR, MODE_INC, 30);
})

DO_FUNC(ST_Z_DEC,
{
    DO_ST(Z_ADDR, MODE_DEC, 30);
})

DO_FUNC(STD_Y,
{
    DO_ST(Y_ADDR + __q, MODE_UNCHANGED, 0);
})

DO_FUNC(STD_Z,
{
    DO_ST(Z_ADDR + __q, MODE_UNCHANGED, 0);
})

#undef DO_ST
#undef MODE_UNCHANGED
#undef MODE_INC
#undef MODE_DEC

DO_FUNC(STS,
{
    uint8_t addr = (__k & 0x0F) | ((__k >> 5) & 0x03) | ((__k << 2) & 0x40) | ((!((__k >> 4) & 0x01)) << 7); // k bits:~4 4 6 5 3 2 1 0
    if(!((addr < REGISTERS_NUM + IO_REGISTERS_NUM) && is_reserved(addr - REGISTERS_NUM)))   // not reserved
        chip->data_memory[addr % DATA_MEMORY_SIZE] = __Rd;
    chip->PC++;
})

DO_FUNC(LPM_R0,
{
    if(chip->cmd.progress < chip->cmd.duration)
        return ERR_SUCCESS;
    chip->registers[0] = ((uint8_t*)chip->flash_memory)[Z_ADDR % FLASH_MEMORY_SIZE];
    chip->PC++;
})

DO_FUNC(LPM,
{
    if(chip->cmd.progress < chip->cmd.duration)
        return ERR_SUCCESS;
    __Rd = ((uint8_t*)chip->flash_memory)[Z_ADDR % FLASH_MEMORY_SIZE];
    chip->PC++;
})

DO_FUNC(LPM_INC,
{
    if(chip->cmd.progress < chip->cmd.duration)
        return ERR_SUCCESS;
    __Rd = ((uint8_t*)chip->flash_memory)[Z_ADDR % FLASH_MEMORY_SIZE];
    uint16_t inc_addr = Z_ADDR + 1;
    chip->registers[31] = inc_addr >> 8;
    chip->registers[30] = inc_addr & 0x0F;
    chip->PC++;
})

DO_FUNC(BSET,
{
    chip->SREG |= _BV(__s);
    chip->PC++;
})

DO_FUNC(BCLR,
{
    chip->SREG &= ~_BV(__s);
    chip->PC++;
})

DO_FUNC(BLD,
{
    if(GET_FLAG_T)
        __Rr |= _BV(__b);
    else
        __Rr &= ~_BV(__b);
    chip->PC++;
})

DO_FUNC(BST,
{
    if(__Rr & _BV(__b))
        chip->SREG |= _BV(SREG_T);
    else
        chip->SREG &= ~_BV(SREG_T);
    chip->PC++;
})

DO_FUNC(SEC,
{
    chip->SREG |= _BV(SREG_C);
    chip->PC++;
})

DO_FUNC(CLC,
{
    chip->SREG &= ~_BV(SREG_C);
    chip->PC++;
})

DO_FUNC(SEN,
{
    chip->SREG |= _BV(SREG_N);
    chip->PC++;
})

DO_FUNC(CLN,
{
    chip->SREG &= ~_BV(SREG_N);
    chip->PC++;
})

DO_FUNC(SEZ,
{
    chip->SREG |= _BV(SREG_Z);
    chip->PC++;
})

DO_FUNC(CLZ,
{
    chip->SREG &= ~_BV(SREG_Z);
    chip->PC++;
})

DO_FUNC(SES,
{
    chip->SREG |= _BV(SREG_S);
    chip->PC++;
})

DO_FUNC(CLS,
{
    chip->SREG &= ~_BV(SREG_S);
    chip->PC++;
})

DO_FUNC(SEV,
{
    chip->SREG |= _BV(SREG_V);
    chip->PC++;
})

DO_FUNC(CLV,
{
    chip->SREG &= ~_BV(SREG_V);
    chip->PC++;
})

DO_FUNC(SET,
{
    chip->SREG |= _BV(SREG_T);
    chip->PC++;
})

DO_FUNC(CLT,
{
    chip->SREG &= ~_BV(SREG_T);
    chip->PC++;
})

DO_FUNC(BREAK,
{
    // There will be something
})

#undef DO_FUNC
#undef DO_CONDITIONAL_BRANCH
#undef SET_FLAG
#undef __d
#undef __Rd
#undef __r
#undef __Rr
#undef __K
#undef __k
#undef __A
#undef __b
#undef __s
