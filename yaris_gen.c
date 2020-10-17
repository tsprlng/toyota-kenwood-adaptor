#define ONE_TIME 2250UL
#define ZERO_TIME 1120UL
#define REPEAT_TIME 110000UL
#define INPUT_1 1
#define INPUT_A 2  // ADC1 aka PB2
#define ADC_A 1  // ADC1 aka PB2
#define OUTPUT_1 3
#define LED 4

// fake no-op definition to satisfy arduino IDE, that will be replaced by external macro:
#define _key_definition(x) _delay_ms(1000)

#include <avr/io.h>
#include <avr/interrupt.h>  // for cli()
#include <util/delay.h>

typedef enum {
  LOW,
  HIGH
} DigitalState;

typedef enum {
  K_NONE,
  K_VOL_DOWN,
  K_VOL_UP,
  K_BACK,
  K_FORWARD,
  K_MODE,
  K_PAUSE,
  K_SOURCE,
  N_KEYS
} Key;

static inline uint16_t analog_read(){
  ADMUX = (1 << REFS0) | ADC_A;  // Set ADC to read our defined ADC_A channel, comparing with the internal 1.1v reference for higher resolution.
  ADCSRA |= (1 << ADSC) | (1 << ADEN);  // Start conversion
  while(!bit_is_set(ADCSRA,ADIF));  // Loop until conversion is complete
  ADCSRA |= (1 << ADIF);  // Clear ADIF by writing a 1 (this sets the value to 0)

  return(ADC);
}

static inline DigitalState digital_read() {
  return (PINB & (1<<INPUT_1)) ? HIGH : LOW;
}
static inline void on() { DDRB |= (1<<OUTPUT_1); }
static inline void off() { DDRB &= ~(1<<OUTPUT_1); }
static inline void ledOn() { PORTB |= (1<<LED); }
static inline void ledOff() { PORTB &= ~(1<<LED); }

void send_agc(){
  on();
  _delay_us(9000);
  off();
  _delay_us(4500);
}

static void send_1() {
  on();
  _delay_us(560);
  off();
  _delay_us(ONE_TIME - 560);
}
static void send_0() {
  on();
  _delay_us(560);
  off();
  _delay_us(ZERO_TIME - 560);
}
static void send_b9(){
  send_1(); send_0(); send_0(); send_1(); send_1(); send_1(); send_0(); send_1(); send_0(); send_1(); send_1(); send_0(); send_0(); send_0(); send_1(); send_0();
}

static void send_key(Key key) {
  if (key == K_MODE){ 
    send_b9(); send_0(); send_1(); send_1(); send_0(); send_1(); send_0(); send_0(); send_0(); send_1(); send_0(); send_0(); send_1(); send_0(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_PAUSE){ 
    send_b9(); send_0(); send_1(); send_1(); send_1(); send_0(); send_0(); send_0(); send_0(); send_1(); send_0(); send_0(); send_0(); send_1(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_SOURCE){ 
    send_b9(); send_1(); send_1(); send_0(); send_0(); send_1(); send_0(); send_0(); send_0(); send_0(); send_0(); send_1(); send_1(); send_0(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_VOL_DOWN){ 
    send_b9(); send_1(); send_0(); send_1(); send_0(); send_1(); send_0(); send_0(); send_0(); send_0(); send_1(); send_0(); send_1(); send_0(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_VOL_UP){ 
    send_b9(); send_0(); send_0(); send_1(); send_0(); send_1(); send_0(); send_0(); send_0(); send_1(); send_1(); send_0(); send_1(); send_0(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_BACK){ 
    send_b9(); send_0(); send_1(); send_0(); send_1(); send_0(); send_0(); send_0(); send_0(); send_1(); send_0(); send_1(); send_0(); send_1(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
  if (key == K_FORWARD){ 
    send_b9(); send_1(); send_1(); send_0(); send_1(); send_0(); send_0(); send_0(); send_0(); send_0(); send_0(); send_1(); send_0(); send_1(); send_1(); send_1(); send_1(); 
    _delay_us(REPEAT_TIME - 16*ZERO_TIME - 16*ONE_TIME);
; }
}

static void send_repeat() {
  ledOn();
  on();
  _delay_us(9000);
  off();
  _delay_us(2250);
   on();
  _delay_us(560);
  off();
  ledOff();
  _delay_us(REPEAT_TIME - (9000 + 2250 + 560));
}

static inline void setup() {
  WDTCR = (1<<4);  // Set WDCE to unset WDE (disable watchdog timer).
  DDRB = (1<<LED);  // LED pin is proper output; output pin starts in tri-state and DDRB will be toggled to drag it to ground.
  PORTB = (1<<INPUT_A) | (1<<INPUT_1);  // Activate pull-ups on both inputs.
}

static Key held_key_() {
  Key found = K_NONE;
  uint16_t analog = analog_read();
  if (digital_read() == LOW){ found = found ? N_KEYS : K_MODE; }
  if (analog > 160 && analog < 500){ found = found ? N_KEYS : K_VOL_DOWN; }
  if (analog > 100 && analog < 150){ found = found ? N_KEYS : K_VOL_UP; }
  if (analog > 55 && analog < 90){ found = found ? N_KEYS : K_FORWARD; }
  if (analog > 25 && analog < 52){ found = found ? N_KEYS : K_BACK; }
  return found >= N_KEYS ? K_NONE : found;
    // If multiple key presses are detected, something is wrong, so do nothing.
}

static Key held_key() {
  // Always read twice to check, as a cheap form of analog debouncing.
  Key k = held_key_();
  return (k == held_key_() ? k : K_NONE);
}

void handleSourceSwitch() {
  while (held_key() == K_MODE){
    ledOn();
    _delay_ms(20);
    ledOff();
    _delay_ms(80);
    ledOn();
    send_agc();
    send_key(K_SOURCE);
    on();
    _delay_us(560);
    off();
    ledOff();
    while (held_key() == K_MODE) {
    }
    for (int toWait=25; toWait; --toWait) {  // Timeout until exiting back to normal main loop.
      _delay_ms(50);
      Key k = held_key();
      if (k == K_NONE){ continue; }  // Nothing pressed; keep waiting until timeout.
      else if (k == K_MODE) { break; }  // MODE pressed again, so go round to switch source a second time.
      else { return; }  // A different key was pressed, so exit back to the normal main loop immediately.
    }
  }
}

void handleModeKey() {
  ledOn();
  send_agc();
  send_key(K_MODE);  // Mute on first press.
  on();
  _delay_us(560);
  off();
  ledOff();

  int toHold = 10;
  while (toHold && held_key() == K_MODE) {
    _delay_ms(50);
    --toHold;
  }
  if (!toHold) {  // Key was held! We will switch source now instead.
    ledOn();
    send_agc();
    send_key(K_MODE);  // Undo initial mute first.
    on();
    _delay_us(560);
    off();
    ledOff();

    _delay_ms(20);
    handleSourceSwitch();  // Enter second loop where further single presses of MODE temporarily mean "switch source".
    return;
  }
  else if (held_key() == K_NONE) {  // Key was released -- wait for potential double tap...
    ledOff();
    for (int toWait=6; toWait; --toWait) {
      _delay_ms(50);
      Key k = held_key();
      if (k == K_NONE){ continue; }  // Nothing pressed; keep waiting until timeout.
      else if (k == K_MODE) {  // Double tap! We must send play/pause!
        ledOn();
        send_agc();
        send_key(K_MODE);  // Undo initial mute first.
        on();
        _delay_us(560);
        off();
        ledOff();

        _delay_ms(20);
        ledOn();
        send_agc();
        send_key(K_PAUSE);
        on();
        _delay_us(560);
        off();
        ledOff();
        while (held_key() == K_MODE) {}  // Wait for key to be released before exiting.
        break;
      }
      else { break; }  // A different key was pressed, so exit back to the normal main loop immediately.
    }
  }
}

int main() {
  setup();
  while(1){
    Key key = held_key();
    if (key == K_NONE){
      continue;  // Just keep waiting, if nothing was pressed.
    }
    if (key == K_MODE){
      handleModeKey();  // This key is special and multifunctional, so has its own handler function.
      continue;
    }

    // For all other keys, just behave like a normal remote, sending the code and repeating while held.
    ledOn();
    send_agc();
    send_key(key);
    on();
    _delay_us(560);
    off();
    ledOff();
    while (held_key() == key) {
      send_repeat();
    }
  }
  return 0;
}
