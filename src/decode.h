// xxxxxxFFZ ZZZZ FFFF
#define FILL_ARGS_IN_OUT                                      \
do {                                                          \
    instr->args.arg[0] = (cmd >> 4) & 0x1F;                   \
    instr->args.arg[1] = (cmd & 0x0F) | ((cmd >> 5) & 0x30);  \
} while (0)

// xxxx xxxx FFFF FZZZ
#define FILL_ARGS_CBI_SBIS                                    \
do {                                                          \
    instr->args.arg[0] = cmd & 0x07;                          \
    instr->args.arg[1] = (cmd >> 3) | 0x1F;                   \
} while (0)

// xxxx xxZF FFFF ZZZZ
#define FILL_ARGS_ALU                                         \
do {                                                          \
    instr->args.arg[0] = (cmd & 0xF) | ((cmd >> 5) & 0x10);   \
    instr->args.arg[1] = (cmd >> 4) & 0x1F;                   \
} while (0)

// xxxx xxxZ ZZZZ xxxx
#define FILL_ARGS_PUSH_POP                                    \
do {                                                          \
    instr->args.arg[0] = (cmd >> 4) & 0x1F;                   \
} while (0)

// xxxx xxZZ ZZZZ Zxxx
#define FILL_ARGS_BRANCH                                      \
do {                                                          \
    instr->args.addr = (cmd >> 3) & 0x7F;                      \
} while (0)

// xxxx ZZZZ ZZZZ ZZZZ
#define FILL_ARGS_RJMP_RCALL                                  \
do {                                                          \
    instr->args.addr = cmd & 0x0FFF;                           \
} while (0)

// xxxx FFFF ZZZZ FFFF
#define FILL_ARGS_LDI_CPI                                     \
do {                                                          \
    instr->args.arg[0] = (cmd >> 4) & 0x0F;                   \
    instr->args.arg[1] = (cmd & 0x0F) | ((cmd >> 4) & 0xF0);  \
} while (0)

// xxxx xxxx xxxx xxxx
#define FILL_ARGS_NO_ARGS

/*  INSTRUCTION | NAME |          CONDITION         | DURATION | FILL_ARGS          */
/*                       ((cmd &   MASK) == OPCODE)                                 */
    INSTRUCTION ( IN   , ((cmd & 0xF800) == 0xB000) ,     1    , FILL_ARGS_IN_OUT   )
    INSTRUCTION ( OUT  , ((cmd & 0xF800) == 0xB800) ,     1    , FILL_ARGS_IN_OUT   )

    INSTRUCTION ( CBI  , ((cmd & 0xFF00) == 0x9800) ,     2    , FILL_ARGS_CBI_SBIS )
    INSTRUCTION ( SBIS , ((cmd & 0xFF00) == 0x9B00) ,     2    , FILL_ARGS_CBI_SBIS )

    INSTRUCTION ( AND  , ((cmd & 0xFC00) == 0x2000) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( ADD  , ((cmd & 0xFC00) == 0x0C00) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( SUB  , ((cmd & 0xFC00) == 0x1800) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( CP   , ((cmd & 0xFC00) == 0x1400) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( MOV  , ((cmd & 0xFC00) == 0x2C00) ,     1    , FILL_ARGS_ALU      )

    INSTRUCTION ( PUSH , ((cmd & 0xFE0F) == 0x920F) ,     2    , FILL_ARGS_PUSH_POP )
    INSTRUCTION ( POP  , ((cmd & 0xFE0F) == 0x900F) ,     2    , FILL_ARGS_PUSH_POP )

    INSTRUCTION ( BREQ , ((cmd & 0xFC07) == 0xF001) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRNE , ((cmd & 0xFC07) == 0xF401) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRGE , ((cmd & 0xFC07) == 0xF404) ,     2    , FILL_ARGS_BRANCH   )

    INSTRUCTION ( CPI  , ((cmd & 0xF000) == 0x3000) ,     1    , FILL_ARGS_LDI_CPI  )
    INSTRUCTION ( LDI  , ((cmd & 0xF000) == 0xE000) ,     1    , FILL_ARGS_LDI_CPI  )

    INSTRUCTION ( NOP  , ((cmd & 0xFFFF) == 0x0000) ,     1    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( RET  , ((cmd & 0xFFFF) == 0x9508) ,     4    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( RETI , ((cmd & 0xFFFF) == 0x9518) ,     4    , FILL_ARGS_NO_ARGS  )

#undef FILL_ARGS_IN_OUT
#undef FILL_ARGS_CBI_SBIS
#undef FILL_ARGS_ALU
#undef FILL_ARGS_PUSH_POP
#undef FILL_ARGS_BRANCH
#undef FILL_ARGS_RJMP_RCALL
#undef FILL_ARGS_LDI_CPI
#undef FILL_ARGS_NO_ARGS