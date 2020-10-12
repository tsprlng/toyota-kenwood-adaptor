.PHONY: all clean flash fuses debug

all: yaris.hex

clean:
	rm -f yaris.hex yaris.o yaris_gen.c yaris.debug.o yaris.debug.hex

flash: all
	sudo picocom -b 115200 /dev/ttyUSB0 <<< m1
	sudo podman run --rm -i -a stdin -a stdout -a stderr --device /dev/ttyUSB0 -v ${PWD}:/mount rubberduck/avr avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U flash:w:/mount/yaris.hex:i -B1000 -xrawfreq=0 -V
	sudo picocom -b 115200 /dev/ttyUSB0 <<< m2
	sudo picocom -b 115200 /dev/ttyUSB0 <<< W

fuses:
	sudo podman run --rm -i -a stdin -a stdout -a stderr --device /dev/ttyUSB0 -v ${PWD}:/mount rubberduck/avr avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U lfuse:w:0x7b:m -U hfuse:w:0xfd:m -B1000 -xrawfreq=0
	# max. (64ms) startup delay; 128kHz clock; min. (1.8v) brown-out detection level

yaris.hex: yaris.o
	avr-objcopy -O ihex yaris.o yaris.hex

yaris.o: yaris_gen.c
	avr-gcc -o yaris.o -mmcu=attiny13a -DF_CPU=128000UL -Os yaris_gen.c

debug: yaris.debug.o
yaris.debug.o: yaris_gen.c
	avr-gcc -o yaris.debug.o -mmcu=attiny13a -DF_CPU=128000UL -g -Os yaris_gen.c

yaris_gen.c: yaris.c preprocess.rb
	./preprocess.rb < yaris.c > yaris_gen.c
