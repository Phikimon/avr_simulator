#ifndef COMMON_H
#define COMMON_H

#define GET_PIN_VALUE(PIN_NUM) ((enum PIN_STATE)((chip->PINB >> PIN_NUM) & 1))
#define _BV(n) (1 << n)

enum PIN_STATE {
    PIN_LOW  = 0,
    PIN_HIGH = 1,
    PIN_HI_Z,
};

enum PINS {
    PB0 = 0,
    PB1 = 1,
    PB2 = 2,
    PB3 = 3,
    PB4 = 4,
    PB5 = 5
};

// SIZES CONSTANTS
enum { REGISTERS_NUM       = 32 }; // 0x20
enum { IO_REGISTERS_NUM    = 64 }; // 0x40
enum { SRAM_SIZE           = 64 }; // 0x40
enum { DATA_MEMORY_SIZE    = REGISTERS_NUM    +
                             IO_REGISTERS_NUM +
                             SRAM_SIZE };
enum { PIN_INT0 = PB1 };
enum { FLASH_MEMORY_SIZE   = 0x0200 }; // 1024
enum { PIN_NUM             = 6 };
// OFFSETS CONSTANTS
enum { REGISTERS_OFFSET    = 0             };
enum { IO_REGISTERS_OFFSET = REGISTERS_NUM };
enum { SRAM_OFFSET         = REGISTERS_NUM    +
                             IO_REGISTERS_NUM };

enum IO_REGISTER_OFFSET {
#define RESERVED_REGISTER(OFFSET)
#define IO_REGISTER(NAME, OFFSET) \
    NAME##_OFFSET = OFFSET,

#include "registers_list.h"

#undef RESERVED_REGISTER
#undef IO_REGISTER
};

enum INSTRUCTION_DURATION {
    HANDLE_INTERRUPT_DURATION = 4,
#define INSTRUCTION(NAME, CONDITION, DURATION, FILL_ARGS)     \
    NAME##_DURATION = DURATION,
#include "decode.h"
#undef INSTRUCTION
};

enum ERR {
    ERR_SUCCESS = 0,
    ERR_INTERRUPT,
    ERR_INVALID_OPCODE,
    ERR_STACK_OVERFLOW
};

enum INTERRUPT_VECTOR_TABLE {
    INTERRUPT_VECTOR_INT0   = 0x0001,
    INTERRUPT_VECTOR_PCINT = 0x0002
};

enum TRIG_INT0 {
    TRIG_LOW_LEVEL    = 0b00,
    TRIG_LOGIC_CHANGE = 0b01,
    TRIG_FALLING_EDGE = 0b10,
    TRIG_RISING_EDGE  = 0b11
};

enum BITS_SREG {
    SREG_C = 0,
    SREG_Z = 1,
    SREG_N = 2,
    SREG_V = 3,
    SREG_S = 4,
    SREG_H = 5,
    SREG_T = 6,
    SREG_I = 7
};

enum BITS_ADCSRB {
    ADTS0   = 0,
    ADTS1   = 1,
    ADTS2   = 2,
    ACME    = 6
};

enum BITS_ADCL {
    ADCL0   = 0,
    ADCL1   = 1,
    ADCL2   = 2,
    ADCL3   = 3,
    ADCL4   = 4,
    ADCL5   = 5,
    ADCL6   = 6,
    ADCL7   = 7
};

enum BITS_ADCH {
    ADCH0   = 0,
    ADCH1   = 1,
    ADCH2   = 2,
    ADCH3   = 3,
    ADCH4   = 4,
    ADCH5   = 5,
    ADCH6   = 6,
    ADCH7   = 7
};

enum BITS_ADCSRA {
    ADPS0   = 0,
    ADPS1   = 1,
    ADPS2   = 2,
    ADIE    = 3,
    ADIF    = 4,
    ADATE   = 5,
    ADSC    = 6,
    ADEN    = 7
};

enum BITS_ADMUX {
    MUX0    = 0,
    MUX1    = 1,
    ADLAR   = 5,
    REFS0   = 6
};

enum BITS_ACSR {
    ACIS0   = 0,
    ACIS1   = 1,
    ACIE    = 3,
    ACI     = 4,
    ACO     = 5,
    ACBG    = 6,
    ACD     = 7
};

enum BITS_DIDR0 {
    AIN0D   = 0,
    AIN1D   = 1,
    ADC1D   = 2,
    ADC3D   = 3,
    ADC2D   = 4,
    ADC0D   = 5
};

enum BITS_PCMSK {
    PCINT0  = 0,
    PCINT1  = 1,
    PCINT2  = 2,
    PCINT3  = 3,
    PCINT4  = 4,
    PCINT5  = 5
};

enum BITS_PINB {
    PINB0   = 0,
    PINB1   = 1,
    PINB2   = 2,
    PINB3   = 3,
    PINB4   = 4,
    PINB5   = 5
};

enum BITS_DDRB {
    DDB0    = 0,
    DDB1    = 1,
    DDB2    = 2,
    DDB3    = 3,
    DDB4    = 4,
    DDB5    = 5
};

enum BITS_PORTB {
    PORTB0  = 0,
    PORTB1  = 1,
    PORTB2  = 2,
    PORTB3  = 3,
    PORTB4  = 4,
    PORTB5  = 5
};

enum BITS_EECR {
    EERE    = 0,
    EEWE    = 1,
    EEPE    = 1,
    EEMWE   = 2,
    EEMPE   = 2,
    EERIE   = 3,
    EEPM0   = 4,
    EEPM1   = 5
};

enum BITS_EEDR {
    EEDR0   = 0,
    EEDR1   = 1,
    EEDR2   = 2,
    EEDR3   = 3,
    EEDR4   = 4,
    EEDR5   = 5,
    EEDR6   = 6,
    EEDR7   = 7
};

enum BITS_EEARL {
    EEAR0   = 0,
    EEAR1   = 1,
    EEAR2   = 2,
    EEAR3   = 3,
    EEAR4   = 4,
    EEAR5   = 5
};

enum BITS_WDTCR {
    WDP0    = 0,
    WDP1    = 1,
    WDP2    = 2,
    WDE     = 3,
    WDCE    = 4,
    WDP3    = 5,
    WDTIE   = 6,
    WDTIF   = 7
};

enum BITS_PRR {
    PRADC   = 0,
    PRTIM0  = 1
};

enum BITS_CLKPR {
    CLKPS0  = 0,
    CLKPS1  = 1,
    CLKPS2  = 2,
    CLKPS3  = 3,
    CLKPCE  = 7
};

enum BITS_GTCCR {
    PSR10   = 0,
    TSM     = 7
};

enum BITS_OCR0B {
    OCR0B_0 = 0,
    OCR0B_1 = 1,
    OCR0B_2 = 2,
    OCR0B_3 = 3,
    OCR0B_4 = 4,
    OCR0B_5 = 5,
    OCR0B_6 = 6,
    OCR0B_7 = 7
};

enum BITS_DWDR {
    DWDR0   = 0,
    DWDR1   = 1,
    DWDR2   = 2,
    DWDR3   = 3,
    DWDR4   = 4,
    DWDR5   = 5,
    DWDR6   = 6,
    DWDR7   = 7
};

enum BITS_TCCR0A {
    WGM00   = 0,
    WGM01   = 1,
    COM0B0  = 4,
    COM0B1  = 5,
    COM0A0  = 6,
    COM0A1  = 7
};

enum BITS_BODCR {
    BODSE   = 0,
    BODS    = 1
};

enum BITS_OSCCAL {
    CAL0    = 0,
    CAL1    = 1,
    CAL2    = 2,
    CAL3    = 3,
    CAL4    = 4,
    CAL5    = 5,
    CAL6    = 6
};

enum BITS_TCNT0 {
    TCNT0_0 = 0,
    TCNT0_1 = 1,
    TCNT0_2 = 2,
    TCNT0_3 = 3,
    TCNT0_4 = 4,
    TCNT0_5 = 5,
    TCNT0_6 = 6,
    TCNT0_7 = 7
};

enum BITS_TCCR0B {
    CS00    = 0,
    CS01    = 1,
    CS02    = 2,
    WGM02   = 3,
    FOC0B   = 6,
    FOC0A   = 7
};

enum BITS_MCUSR {
    PORF    = 0,
    EXTRF   = 1,
    BORF    = 2,
    WDRF    = 3
};

enum BITS_MCUCR {
    ISC00   = 0,
    ISC01   = 1,
    SM0     = 3,
    SM1     = 4,
    SE      = 5,
    PUD     = 6
};

enum BITS_OCR0A {
    OCR0A_0 = 0,
    OCR0A_1 = 1,
    OCR0A_2 = 2,
    OCR0A_3 = 3,
    OCR0A_4 = 4,
    OCR0A_5 = 5,
    OCR0A_6 = 6,
    OCR0A_7 = 7
};

enum BITS_SPMCSR {
    SPMEN   = 0,
    PGERS   = 1,
    PGWRT   = 2,
    RFLB    = 3,
    CTPB    = 4
};

enum BITS_TIFR0 {
    TOV0    = 1,
    OCF0A   = 2,
    OCF0B   = 3
};

enum BITS_TIMSK0 {
    TOIE0   = 1,
    OCIE0A  = 2,
    OCIE0B  = 3
};

enum BITS_GIFR {
    PCIF    = 5,
    INTF0   = 6
};

enum BITS_GIMSK {
    PCIE    = 5,
    INT0    = 6,
};

#endif
