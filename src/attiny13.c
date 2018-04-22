#include "attiny13.h"
#include <string.h>

int attiny13_ctor(struct attiny13* chip)
{
    chip->data_memory  = chip->registers;
    chip->io_registers = &(chip->data_memory[IO_REGISTERS_OFFSET]);
    (void)memset(chip->data_memory,  0x00, DATA_MEMORY_SIZE);
    (void)memset(chip->flash_memory, 0x00, FLASH_MEMORY_SIZE);
    chip->adc_value = 0;
    for (int i = 0; i < PIN_NUM; i++)
    {
        chip->pin_values[i] = PIN_HI_Z;
    }
    return 0;
}
