.PHONY: all clean

all: yaris.hex

flash: all
	avrdude -p ATmega32U4 -c avr109 -P /dev/ttyACM0 -U flash:w:yaris.hex -v

yaris.hex: yaris.ino
	arduino-builder -hardware /usr/share/arduino/hardware -tools /usr/bin -fqbn archlinux-arduino:avr:micro -build-path /home/tds/tmp -compile yaris.ino
	cp /home/tds/tmp/yaris.ino.hex ./yaris.hex

yaris.ino: sketch_jul29a.h preprocess.rb
	./preprocess.rb < sketch_jul29a.h > yaris.ino
