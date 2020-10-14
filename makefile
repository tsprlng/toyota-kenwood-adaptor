SERIAL_DEV = /dev/ttyUSB0

gcc = avr-gcc -mmcu=attiny13a -DF_CPU=128000UL
picocom = sudo picocom -b 115200 $(SERIAL_DEV)
avrdude = sudo podman run --rm -i -a stdin -a stdout -a stderr --device $(SERIAL_DEV) -v ${PWD}:/pwd -w /pwd rubberduck/avr avrdude -c buspirate -P $(SERIAL_DEV) -p t13 -B1000 -xrawfreq=0 -V

.PHONY: all clean flash fuses

all: yaris.hex

clean:
	rm -f yaris.hex yaris.o yaris_gen.c yaris.debug.o yaris.debug.hex

flash: yaris.hex
	$(picocom) <<< m1
	$(avrdude) -U flash:w:$<:i
	$(picocom) <<< m2
	$(picocom) <<< W

fuses:
	$(avrdude) -U lfuse:w:0x7b:m -U hfuse:w:0xfd:m
	# max. (64ms) startup delay; 128kHz clock; min. (1.8v) brown-out detection level

%.hex: %.o
	avr-objcopy -O ihex $< $@

yaris.o: yaris_gen.c
	$(gcc) -o $@ -Os $<

yaris_gen.c: yaris.c preprocess.rb
	./preprocess.rb < $< > $@
