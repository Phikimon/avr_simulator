#include "attiny13.h"
#include <string.h>

int attiny13_ctor(struct attiny13* chip)
{
    chip->data_memory  = chip->registers;
    chip->io_registers = &(chip->data_memory[IO_REGISTERS_OFFSET]);
    (void)memset(chip->data_memory,  0x00, DATA_MEMORY_SIZE);
    (void)memset(chip->flash_memory, 0x00, FLASH_MEMORY_SIZE);
    return 0;
}

int execute_cycle(struct attiny13* chip)
{
    refresh_interrupt_flags(chip);

    int16_t cmd = chip->flash_memory[chip->PC];
    cmd = (cmd >> 8) | (cmd << 8); // Change endian
    static struct cmd instr = {0};
    int decode_err = ERR_SUCCESS;

    if (instr.is_finished) {
        decode_err = decode(chip, &instr, cmd);
        if (decode_err)
            return decode_err;
    }

    return instr.func(chip, &instr);
}


int refresh_interrupt_flags(struct attiny13* chip)
{
    static int INT0_prev_state = PIN_HI_Z;
    if (INT0_prev_state == PIN_HI_Z) {
        INT0_prev_state = GET_PIN_VALUE(PIN_INT0);
    }

    int is_INT0_enabled = chip->GIMSK & _BV(INT0);
    int INT0_trigger    = chip->MCUCR & _BV(ISC01) & _BV(ISC00);
    if (is_INT0_enabled) {                        // INT0 enabled
        switch (INT0_trigger) {  // Set INT0 flag in GIFR
            case TRIG_LOW_LEVEL:
                chip->GIFR &= ~_BV(INTF0); // When INT0 is level triggered GIFR
                                           // INTF0 flag is always cleared
                                           // Level is sampled in check_interrupt()
                break;
            case TRIG_LOGIC_CHANGE:
                if (GET_PIN_VALUE(PIN_INT0) != INT0_prev_state)
                    chip->GIFR |= _BV(INTF0);
                break;
            case TRIG_FALLING_EDGE:
                if ((GET_PIN_VALUE(PIN_INT0) == PIN_LOW) && (INT0_prev_state == PIN_HIGH))
                    chip->GIFR |= _BV(INTF0);
                break;
            case TRIG_RISING_EDGE:
                if ((GET_PIN_VALUE(PIN_INT0) == PIN_HIGH) && (INT0_prev_state == PIN_LOW))
                    chip->GIFR |= _BV(INTF0);
                break;
            default:
                break;
        }
    }
    INT0_prev_state = GET_PIN_VALUE(PIN_INT0);

    static int PCINT0_prev_state[PIN_NUM] = {PIN_HI_Z};
    if (PCINT0_prev_state[0] == PIN_HI_Z) {
        for (int i = 0; i < PIN_NUM; i++) {
            PCINT0_prev_state[i] = GET_PIN_VALUE(i);
        }
    }
    int is_PCINT_enabled = chip->GIMSK & _BV(PCIE);
    if (is_PCINT_enabled) {
        for (int i = 0; i < PIN_NUM; i++) {
            if ((chip->PCMSK & _BV(i)) && //< PCINT interrupt on pin enabled
                (PCINT0_prev_state[i] != GET_PIN_VALUE(i))) {
                chip->GIFR |= _BV(PCIF);
                break;
            }
        }
    }
    for (int i = 0; i < PIN_NUM; i++)
        PCINT0_prev_state[i] = GET_PIN_VALUE(i);

    return ERR_SUCCESS;
}

int decode(struct attiny13* chip, struct cmd* instr, int16_t cmd)
{
    if ((check_interrupt(chip, instr) == ERR_INTERRUPT))
        return ERR_SUCCESS;
#define INSTRUCTION(NAME, CONDITION, DURATION, FILL_ARGS)     \
{                                                             \
    if (CONDITION) {                                          \
        instr->func = do_##NAME;                              \
        instr->progress = instr->is_finished = 0;             \
        FILL_ARGS;                                            \
        return ERR_SUCCESS;                                   \
    }                                                         \
} // Do-while-0 is not used intentionally

#include "decode.h"

#undef INSTRUCTION
    return ERR_INVALID_OPCODE;
}

int check_interrupt(struct attiny13* chip, struct cmd* instr)
{
    int are_interrupts_enabled = chip->SREG & _BV(SREG_I);

    int is_INT0_enabled = chip->GIMSK & _BV(INT0);
    int INT0_trigger    = chip->MCUCR & _BV(ISC01) & _BV(ISC00);

    int is_INT0_low_level_interrupt  = (INT0_trigger == TRIG_LOW_LEVEL) &&
                                       (GET_PIN_VALUE(PIN_INT0) == PIN_LOW);
    int is_INT0_pin_change_interrupt = chip->GIFR & _BV(INTF0); // Flag is set

    int is_PCINT_enabled   = chip->GIMSK & _BV(PCIE);
    int is_PCINT_interrupt = chip->GIFR  & _BV(PCIF);

    if ((are_interrupts_enabled) && (is_INT0_enabled) &&
        (is_INT0_low_level_interrupt || is_INT0_pin_change_interrupt)) {
        instr->args.arg[0] = INTERRUPT_VECTOR_INT0;
        chip->GIFR &= ~_BV(INTF0); // Clear interrupt flag
        goto is_interrupt;
    }

    if ((are_interrupts_enabled) && (is_PCINT_enabled) &&
        (is_PCINT_interrupt)) {
        instr->args.arg[0] = INTERRUPT_VECTOR_PCINT;
        chip->GIFR &= ~_BV(PCIF); // Clear interrupt flag
        goto is_interrupt;
    }

    return ERR_SUCCESS;

is_interrupt:
    instr->func = do_HANDLE_INTERRUPT;
    instr->progress = 0;
    instr->is_finished = 0;
    chip->SREG &= ~(1 << SREG_I); // Clear global interrupt flag
    return ERR_INTERRUPT;
}

int attiny13_push_pc(struct attiny13* chip) {
    chip->SPL -= 2;
    if(chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM))
        return ERR_STACK_OVERFLOW;
    *(uint16_t*)(chip->data_memory + chip->SPL) = chip->PC;
    return ERR_SUCCESS;
}
