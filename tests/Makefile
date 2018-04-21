CC=avr-gcc
CFLAGS=-mmcu=attiny13a -Os -static

blink: blink.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s
#	avr-objcopy -I binary -O elf32-avr $@ elf
#	avr-objdump -D elf
#	rm elf

clean:
	rm blink *.s || true
