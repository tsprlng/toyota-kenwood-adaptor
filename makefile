.PHONY: all clean

all: yaris.hex

yaris.hex: yaris.ino
	arduino-builder -hardware /usr/share/arduino/hardware -tools /usr/bin -fqbn archlinux-arduino:avr:micro -compile yaris.ino

yaris.ino: sketch_jul29a.h preprocess.rb
	./preprocess.rb < sketch_jul29a.h > yaris.ino
