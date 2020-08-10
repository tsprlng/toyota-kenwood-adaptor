.PHONY: all clean

all: yaris.hex

flash: all
	avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U flash:w:yaris.hex -B1000 -xrawfreq=0

fuses:
	avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U lfuse:w:0x6b:m -U hfuse:w:0xfd:m -B1000 -xrawfreq=0

yaris.hex: yaris.c
	avr-gcc -o yaris.hex -mmcu=attiny13a -DF_CPU=128000UL -Os yaris.c

yaris.c: sketch_jul29a.h preprocess.rb
	./preprocess.rb < sketch_jul29a.h > yaris.c
