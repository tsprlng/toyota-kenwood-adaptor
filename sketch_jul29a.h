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

static inline uint16_t analogRead(uint8_t channel){
  ADMUX = (1 << REFS0) | channel;     // read channel vs internal voltage ref
  ADCSRA |= (1 << ADSC);              // Start conversion
  while(!bit_is_set(ADCSRA,ADIF));    // Loop until conversion is complete
  ADCSRA |= (1 << ADIF);              // Clear ADIF by writing a 1 (this sets the value to 0)

  return(ADC);
}

static inline void digitalWrite(uint8_t pin, DigitalState state) {
  if (state == LOW){
    DDRB |= (1<<pin);
  } else {
    DDRB &= ~(1<<pin);
  }
}
static inline DigitalState digitalRead(uint8_t pin) {
  if(PINB & (1<<pin)) {
    return HIGH;
  }
  return LOW;
}
static inline void on() { digitalWrite(OUTPUT_1, HIGH); }
static inline void off() { digitalWrite(OUTPUT_1, LOW); }

static inline void one() {
  on();
  _delay_us(560);
  off();
  _delay_us(ONE_TIME - 560);
}
static inline void zero() {
  on();
  _delay_us(560);
  off();
  _delay_us(ZERO_TIME - 560);
}

static inline void repeat() {
  on();
  _delay_us(9000);
  off();
  _delay_us(2250);
   on();
  _delay_us(560);
  off();
  _delay_us(REPEAT_TIME - (9000 + 2250 + 560));
}

static inline void setup() {
  WDTCR = (1<<4);  // set WDCE to unset WDE
  cli();  // no interrupts at any time
  DDRB = (1<<LED);
  PORTB = (1<<INPUT_A) | (1<<INPUT_1);
  ADMUX = (1 << REFS0) | ADC_A;     // read channel vs internal voltage ref
}

static inline Key held_key() {
  Key found = K_NONE;
  int analog = analogRead(ADC_A);
  if (digitalRead(INPUT_1) == LOW){ found = found ? N_KEYS : K_MODE; }
  if (analog > 115 && analog < 280){ found = found ? N_KEYS : K_VOL_DOWN; }
  if (analog > 80 && analog < 105){ found = found ? N_KEYS : K_VOL_UP; }
  if (analog > 40 && analog < 80){ found = found ? N_KEYS : K_FORWARD; }
  if (analog > 10 && analog < 35){ found = found ? N_KEYS : K_BACK; }
  return found >= N_KEYS ? K_NONE : found;
    // multiple key presses => something is wrong, so do nothing
}

static inline void send_key(Key key) {
  if (key == K_MODE){ _key_definition(0xb916); }
  if (key == K_PAUSE){ _key_definition(0xb90e); }
  if (key == K_SOURCE){ _key_definition(0xb913); }
  if (key == K_VOL_DOWN){ _key_definition(0xb915); }
  if (key == K_VOL_UP){ _key_definition(0xb914); }
  if (key == K_BACK){ _key_definition(0xb90a); }
  if (key == K_FORWARD){ _key_definition(0xb90b); }
}

void handleSourceSwitch() {
  while (held_key() == K_MODE){
    digitalWrite(LED, HIGH);
    //digitalWrite(LED_TX, HIGH);
    //digitalWrite(LED_RX, HIGH);
    on();
    _delay_us(9000);
    off();
    _delay_us(4500);
    send_key(K_SOURCE);
    on();
    _delay_us(560);
    off();
    while (held_key() == K_MODE) {
      repeat();
    }
    digitalWrite(LED, LOW);
    //digitalWrite(LED_TX, LOW);
    //digitalWrite(LED_RX, LOW);
    for (int toWait=25; toWait; --toWait) {
      _delay_ms(50);
      Key k = held_key();
      if (k == K_NONE){ continue; }
      else if (k == K_MODE) { break; }  // double tap! we must switch again
      else { return; }
    }
  }
}

void handleModeKey() {
  digitalWrite(LED, HIGH);
  //digitalWrite(LED_RX, HIGH);
  on();
  _delay_us(9000);
  off();
  _delay_us(4500);
  send_key(K_MODE);  // mute on first press
  on();
  _delay_us(560);
  off();

  int toHold = 10;
  while (toHold && held_key() == K_MODE) {
    _delay_ms(50);
    --toHold;
  }
  if (!toHold) {  // key was held! we will switch source now instead
    on();
    _delay_us(9000);
    off();
    _delay_us(4500);
    send_key(K_MODE);  // undo initial mute
    on();
    _delay_us(560);
    off();

    _delay_ms(20);
    handleSourceSwitch();
    return;
  }
  else if (held_key() == K_NONE) {  // key was released -- wait for potential double tap
    digitalWrite(LED, LOW);
    for (int toWait=6; toWait; --toWait) {
      _delay_ms(50);
      Key k = held_key();
      if (k == K_NONE){ continue; }
      else if (k == K_MODE) {  // double tap! we must pause
        digitalWrite(LED, HIGH);
        //digitalWrite(LED_TX, HIGH);
        on();
        _delay_us(9000);
        off();
        _delay_us(4500);
        send_key(K_MODE);  // undo initial mute
        on();
        _delay_us(560);
        off();

        _delay_ms(20);
        on();
        _delay_us(9000);
        off();
        _delay_us(4500);
        send_key(K_PAUSE);
        on();
        _delay_us(560);
        off();
        while (held_key() == K_MODE) {
          repeat();
        }
        break;
      }
      else { break; }
    }
  }

  digitalWrite(LED, LOW);
  //digitalWrite(LED_TX, LOW);
  //digitalWrite(LED_RX, LOW);
}

static inline void loop() {
  Key key = held_key();
  if (key == K_NONE){
    return;
  }
  if (key == K_MODE){
    handleModeKey();  // this one is special and multifunctional
    return;
  }

  digitalWrite(LED, HIGH);
  on();
  _delay_us(9000);
  off();
  _delay_us(4500);
  send_key(key);
  on();
  _delay_us(560);
  off();
  while (held_key() == key) {
    repeat();
  }
  digitalWrite(LED, LOW);
}

void main() {
  setup();
  while(1){ loop(); }
}
