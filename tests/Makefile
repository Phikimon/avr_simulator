CC=avr-gcc
CFLAGS=-mmcu=attiny13a -Os -static

all: blink.bin receive.bin

blink: blink.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s

receive: receive.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s

blink.bin: blink
	avr-objcopy -I elf32-avr -O binary $< $@

receive.bin: receive
	avr-objcopy -I elf32-avr -O binary $< $@

clean:
	rm blink receive *.s *.bin || true
