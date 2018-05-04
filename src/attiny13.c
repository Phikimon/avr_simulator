#include "attiny13.h"
#include "threads.h"
#include "gui.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

int attiny13_ctor(struct attiny13* chip, const char* filename)
{
    chip->data_memory  = chip->registers;
    chip->io_registers = (uint8_t*)&(chip->data_memory[IO_REGISTERS_OFFSET]);
    (void)memset(chip->data_memory,  0x00, DATA_MEMORY_SIZE  * sizeof(*chip->data_memory));
    (void)memset(chip->flash_memory, 0x00, FLASH_MEMORY_SIZE * sizeof(*chip->flash_memory));
    chip->PC = 0x0000;
    chip->SPL = DATA_MEMORY_SIZE - 1;    // 0x9F
    chip->cmd.progress = 0;
    //Copy file in memory
    assert(filename);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf_gui(simulator.window, "Unable to open file. Try again");
        return -1;
    }
    (void)read(fd, chip->flash_memory, FLASH_MEMORY_SIZE);
    if (close(fd) == -1) {
        printf_gui(simulator.window, "Error closing file descriptor");
        return -1;
    }

    return 0;
}

int execute_cycle(struct attiny13* chip)
{
    refresh_interrupt_flags(chip);

    int decode_err = ERR_SUCCESS;
    uint16_t cmd = 0;

    if (chip->cmd.progress == 0) {
        cmd = chip->flash_memory[chip->PC];
//        cmd = (cmd >> 8) | (cmd << 8);
        if ((decode_err = decode(chip, cmd))) {
            return decode_err;
        }
    }

    return chip->cmd.func(chip);
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

int decode(struct attiny13* chip, uint16_t cmd)
{
    if ((check_interrupt(chip) == ERR_INTERRUPT))
        return ERR_SUCCESS;
    acquire_lock();
    printf("%4X", cmd);
#define INSTRUCTION(NAME, CONDITION, DURATION, FILL_ARGS)     \
{                                                             \
    if (CONDITION) {                                          \
        chip->cmd.func = do_##NAME;                           \
        chip->cmd.progress = 0;                               \
        chip->cmd.duration = DURATION;                        \
        FILL_ARGS;                                            \
        printf("\t%s\n", #NAME);                              \
        return ERR_SUCCESS;                                   \
    }                                                         \
} // Do-while-0 is not used intentionally
    fflush(stdout);
    release_lock();

#include "decode.h"

#undef INSTRUCTION
    return ERR_INVALID_OPCODE;
}

int check_interrupt(struct attiny13* chip)
{
    int are_interrupts_enabled = GET_FLAG_I;

    int is_INT0_enabled = chip->GIMSK & _BV(INT0);
    int INT0_trigger    = chip->MCUCR & _BV(ISC01) & _BV(ISC00);

    int is_INT0_low_level_interrupt  = (INT0_trigger == TRIG_LOW_LEVEL) &&
                                       (GET_PIN_VALUE(PIN_INT0) == PIN_LOW);
    int is_INT0_pin_change_interrupt = chip->GIFR & _BV(INTF0); // Flag is set

    int is_PCINT_enabled   = chip->GIMSK & _BV(PCIE);
    int is_PCINT_interrupt = chip->GIFR  & _BV(PCIF);

    if ((are_interrupts_enabled) && (is_INT0_enabled) &&
        (is_INT0_low_level_interrupt || is_INT0_pin_change_interrupt)) {
        chip->cmd.args.arg[0] = INTERRUPT_VECTOR_INT0;
        chip->GIFR &= ~_BV(INTF0); // Clear interrupt flag
        goto is_interrupt;
    }

    if ((are_interrupts_enabled) && (is_PCINT_enabled) &&
        (is_PCINT_interrupt)) {
        chip->cmd.args.arg[0] = INTERRUPT_VECTOR_PCINT;
        chip->GIFR &= ~_BV(PCIF); // Clear interrupt flag
        goto is_interrupt;
    }

    return ERR_SUCCESS;

is_interrupt:
    chip->cmd.func = do_HANDLE_INTERRUPT;
    chip->cmd.progress = 0;
    chip->cmd.duration = HANDLE_INTERRUPT_DURATION;
    chip->SREG &= ~(1 << SREG_I); // Clear global interrupt flag
    return ERR_INTERRUPT;
}

int attiny13_push_pc(struct attiny13* chip)
{
    if(chip->SPL < (REGISTERS_NUM + IO_REGISTERS_NUM + 1))
        return ERR_STACK_OVERFLOW;
    *(uint16_t*)(chip->data_memory + chip->SPL - 1) = chip->PC + 1;
    chip->SPL -= 2;
    return ERR_SUCCESS;
}
