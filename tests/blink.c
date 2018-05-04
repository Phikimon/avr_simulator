#include <avr/io.h>

int main(void)
{
    DDRB = 0xFF; //Nakes PORTB as Output
    while(1) //infinite loop
    {
        PORTB = 0xFF;
        PORTB = 0xFF;
        PORTB = 0xFF;
        PORTB = 0xFF;
        PORTB = 0xFF;
        PORTB= 0x00;
        PORTB= 0x00;
        PORTB= 0x00;
        PORTB= 0x00;
        PORTB= 0x00;
    }
}
