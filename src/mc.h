#ifndef AVR_H
#define AVR_H

enum pin_state
{
    PIN_LOW = 0,
    PIN_HIGH,
    PIN_HI_Z,
};

struct mc
{
    //Public
    enum pin_state pin_values[6];
    //Private
    uint8_t PORTB, DDRB, PINB;
};

#endif
