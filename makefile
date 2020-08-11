.PHONY: all clean

all: yaris.hex

clean:
	rm yaris.hex yaris.o yaris_gen.c

flash: all
	avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U flash:w:yaris.hex:i -B1000 -xrawfreq=0

fuses:
	avrdude -c buspirate -P /dev/ttyUSB0 -p t13 -U lfuse:w:0x6b:m -U hfuse:w:0xfd:m -B1000 -xrawfreq=0

yaris.hex: yaris.o
	avr-objcopy -O ihex yaris.o yaris.hex

yaris.o: yaris_gen.c
	avr-gcc -o yaris.o -mmcu=attiny13a -DF_CPU=128000UL -Os -ffunction-sections -fdata-sections -Wl,--gc-sections yaris_gen.c

yaris_gen.c: yaris.c preprocess.rb
	./preprocess.rb < yaris.c > yaris_gen.c
