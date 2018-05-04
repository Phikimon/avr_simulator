#include <avr/io.h>

int main(void)
{
    DDRB = 0x07;
    while(1)
    {
        PORTB = PINB >> 3;
    }
}
