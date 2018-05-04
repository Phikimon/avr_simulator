// xxxx xZZF FFFF ZZZZ
#define FILL_ARGS_IN_OUT                                         \
do {                                                             \
    chip->cmd.args.arg[0] = (cmd & 0x0F) | ((cmd >> 5) & 0x30);  \
    chip->cmd.args.arg[1] = (cmd >> 4) & 0x1F;                   \
} while (0)

// xxxx xxxx ZZZZ ZFFF
#define FILL_ARGS_CBI_SBI                                        \
do {                                                             \
    chip->cmd.args.arg[0] = (cmd >> 3) | 0x1F;                   \
    chip->cmd.args.arg[1] = cmd & 0x07;                          \
} while (0)

// xxxx xxZF FFFF ZZZZ
#define FILL_ARGS_ALU                                            \
do {                                                             \
    chip->cmd.args.arg[0] = (cmd & 0xF) | ((cmd >> 5) & 0x10);   \
    chip->cmd.args.arg[1] = (cmd >> 4) & 0x1F;                   \
} while (0)

// xxxx xxxZ ZZZZ xxxx
#define FILL_ARGS_PUSH_POP                                       \
do {                                                             \
    chip->cmd.args.arg[0] = (cmd >> 4) & 0x1F;                   \
} while (0)

// xxxx xxZZ ZZZZ Zxxx
#define FILL_ARGS_BRANCH                                         \
do {                                                             \
    chip->cmd.args.addr = (cmd >> 3) & 0x7F;                     \
} while (0)

// xxxx ZZZZ ZZZZ ZZZZ
#define FILL_ARGS_RJMP_RCALL                                     \
do {                                                             \
    chip->cmd.args.addr = cmd & 0x0FFF;                          \
} while (0)

// xxxx ZZZZ FFFF ZZZZ
#define FILL_ARGS_LDI_CPI                                        \
do {                                                             \
    chip->cmd.args.arg[0] = (cmd & 0x0F) | ((cmd >> 4) & 0xF0);  \
    chip->cmd.args.arg[1] = ((cmd >> 4) & 0x0F) + 16;            \
} while (0)

// xxxx xxdd dddd dddd
#define FILL_ARGS_LONG_D                                         \
do {                                                             \
    chip->cmd.args.arg[1] = cmd & 0x001F;                        \
} while (0)

//xxxx xxxx dddd xxxx
#define FILL_ARGS_SER                                            \
do {                                                             \
    chip->cmd.args.arg[1] = ((cmd >> 4) & 0x0F) + 16;            \
} while (0)

// xxxx xxxx xxxx xxxx
#define FILL_ARGS_NO_ARGS

/*  INSTRUCTION | NAME |          CONDITION         | DURATION | FILL_ARGS          */
/*                       ((cmd &   MASK) == OPCODE)                                 */
    INSTRUCTION ( IN    , ((cmd & 0xF800) == 0xB000) ,     1    , FILL_ARGS_IN_OUT   )
    INSTRUCTION ( OUT   , ((cmd & 0xF800) == 0xB800) ,     1    , FILL_ARGS_IN_OUT   )

    INSTRUCTION ( CBI   , ((cmd & 0xFF00) == 0x9800) ,     2    , FILL_ARGS_CBI_SBI  )
    INSTRUCTION ( SBI   , ((cmd & 0xFF00) == 0x9A00) ,     2    , FILL_ARGS_CBI_SBI  )

    INSTRUCTION ( AND   , ((cmd & 0xFC00) == 0x2000) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( OR    , ((cmd & 0xFC00) == 0x2800) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( NEG   , ((cmd & 0xFC00) == 0x9401) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( ADD   , ((cmd & 0xFC00) == 0x0C00) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( SUB   , ((cmd & 0xFC00) == 0x1800) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( DEC   , ((cmd & 0xFC00) == 0x940A) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( INC   , ((cmd & 0xFC00) == 0x9403) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( CP    , ((cmd & 0xFC00) == 0x1400) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( MOV   , ((cmd & 0xFC00) == 0x2C00) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( CLR   , ((cmd & 0xFC00) == 0x2400) ,     1    , FILL_ARGS_LONG_D   )
    INSTRUCTION ( SER   , ((cmd & 0xFF0F) == 0xEF0F) ,     1    , FILL_ARGS_SER      )
    INSTRUCTION ( TST   , ((cmd & 0xFC00) == 0x2000) ,     1    , FILL_ARGS_LONG_D   )
    INSTRUCTION ( LSR   , ((cmd & 0xFE0F) == 0x9406) ,     1    , FILL_ARGS_ALU      )

    INSTRUCTION ( PUSH  , ((cmd & 0xFE0F) == 0x920F) ,     2    , FILL_ARGS_PUSH_POP )
    INSTRUCTION ( POP   , ((cmd & 0xFE0F) == 0x900F) ,     2    , FILL_ARGS_PUSH_POP )

    INSTRUCTION ( LD_X  , ((cmd & 0xFE0F) == 0x900C) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( LD_Y  , ((cmd & 0xFE0F) == 0x8008) ,     1    , FILL_ARGS_ALU      )
    INSTRUCTION ( LD_Z  , ((cmd & 0xFE0F) == 0x8000) ,     1    , FILL_ARGS_ALU      )

    INSTRUCTION ( ST_X  , ((cmd & 0xFE0F) == 0x920C) ,     2    , FILL_ARGS_ALU      )
    INSTRUCTION ( ST_Y  , ((cmd & 0xFE0F) == 0x8208) ,     2    , FILL_ARGS_ALU      )
    INSTRUCTION ( ST_Z  , ((cmd & 0xFE0F) == 0x8200) ,     2    , FILL_ARGS_ALU      )

    INSTRUCTION ( LPM   , ((cmd & 0xFE0F) == 0x9004) ,     3    , FILL_ARGS_ALU      )

    INSTRUCTION ( RCALL , ((cmd & 0xF000) == 0xD000) ,     3    , FILL_ARGS_RJMP_RCALL )
    INSTRUCTION ( RJMP  , ((cmd & 0xF000) == 0xC000) ,     2    , FILL_ARGS_RJMP_RCALL )

    INSTRUCTION ( BREQ  , ((cmd & 0xFC07) == 0xF001) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRNE  , ((cmd & 0xFC07) == 0xF401) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRGE  , ((cmd & 0xFC07) == 0xF404) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRCS  , ((cmd & 0xFC07) == 0xF000) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRCC  , ((cmd & 0xFC07) == 0xF400) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRSH  , ((cmd & 0xFC07) == 0xF400) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRLO  , ((cmd & 0xFC07) == 0xF000) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRMI  , ((cmd & 0xFC07) == 0xF002) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRPL  , ((cmd & 0xFC07) == 0xF402) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRLT  , ((cmd & 0xFC07) == 0xF004) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRTS  , ((cmd & 0xFC07) == 0xF006) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRTC  , ((cmd & 0xFC07) == 0xF406) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRVS  , ((cmd & 0xFC07) == 0xF003) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRVC  , ((cmd & 0xFC07) == 0xF403) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRIE  , ((cmd & 0xFC07) == 0xF007) ,     2    , FILL_ARGS_BRANCH   )
    INSTRUCTION ( BRID  , ((cmd & 0xFC07) == 0xF407) ,     2    , FILL_ARGS_BRANCH   )


    INSTRUCTION ( CPI   , ((cmd & 0xF000) == 0x3000) ,     1    , FILL_ARGS_LDI_CPI  )
    INSTRUCTION ( LDI   , ((cmd & 0xF000) == 0xE000) ,     1    , FILL_ARGS_LDI_CPI  )

    INSTRUCTION ( NOP   , ((cmd & 0xFFFF) == 0x0000) ,     1    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( RET   , ((cmd & 0xFFFF) == 0x9508) ,     4    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( RETI  , ((cmd & 0xFFFF) == 0x9518) ,     4    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( IJMP  , ((cmd & 0xFFFF) == 0x9409) ,     2    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( SEI   , ((cmd & 0xFFFF) == 0x9478) ,     1    , FILL_ARGS_NO_ARGS  )
    INSTRUCTION ( CLI   , ((cmd & 0xFFFF) == 0x94F8) ,     1    , FILL_ARGS_NO_ARGS  )

#undef FILL_ARGS_IN_OUT
#undef FILL_ARGS_CBI_SBI
#undef FILL_ARGS_ALU
#undef FILL_ARGS_PUSH_POP
#undef FILL_ARGS_BRANCH
#undef FILL_ARGS_RJMP_RCALL
#undef FILL_ARGS_LDI_CPI
#undef FILL_ARGS_LONG_D
#undef FILL_ARGS_SER
#undef FILL_ARGS_NO_ARGS
