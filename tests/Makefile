CC=avr-gcc
CFLAGS=-mmcu=attiny13a -O1 -static

all: blink.bin receive.bin

blink: blink.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s

receive: receive.c
	avr-gcc $@.c $(CFLAGS) -o $@
	avr-gcc $@.c $(CFLAGS) -S -o $@.s

blink.bin: blink
	avr-objcopy -I elf32-avr -O binary $< $@
	avr-objdump -D $< > objdump_1
	avr-objdump -D $< > objdump_2

receive.bin: receive
	avr-objcopy -I elf32-avr -O binary $< $@
	avr-objdump -D $< > objdump_2

clean:
	rm objdump_* blink receive *.s *.bin || true
