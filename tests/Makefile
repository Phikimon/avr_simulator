CC=avr-gcc
CFLAGS=-mmcu=attiny13a -Os -static

all: blink blink.bin

blink: blink.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s
#	avr-objdump -D elf
#	rm elf

blink.bin: blink
	avr-objcopy -I elf32-avr -O binary blink $@

clean:
	rm blink *.s || true
