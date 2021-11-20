FLAGS = -mmcu=attiny461 -DF_CPU=750000UL -O2 -std=c99 $(DEBUG) $(ALTCODE)

slowglow: tree.hex 

%.hex: %.elf ws2812.h
	avr-objcopy -O ihex $< $@

clean:
	rm -f *.elf *.hex

random.c: random.h

%.elf: %.c random.c
	avr-gcc $(FLAGS) $^ -o $@
	avr-size -C --mcu=attiny461 $@
