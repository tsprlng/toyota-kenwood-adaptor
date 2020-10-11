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
  ADCSRA |= (1 << ADSC) | (1 << ADEN);// Start conversion
  while(!bit_is_set(ADCSRA,ADIF));    // Loop until conversion is complete
  ADCSRA |= (1 << ADIF);              // Clear ADIF by writing a 1 (this sets the value to 0)

  return(ADC);
}

static inline void on() { DDRB |= (1<<OUTPUT_1); }
static inline void off() { DDRB &= ~(1<<OUTPUT_1); }
static inline void ledOn() { PORTB |= (1<<LED); }
static inline void ledOff() { PORTB &= ~(1<<LED); }

static inline void setup() {
  WDTCR = (1<<4);  // set WDCE to unset WDE
  DDRB = (1<<LED);
  PORTB = (1<<INPUT_A) | (1<<INPUT_1);
  ADMUX = (1 << REFS0) | ADC_A;     // read channel vs internal voltage ref
}

void show_digit(int n){
  for(int i=0; i<n; ++i){
    _delay_ms(180);
    ledOn();
    _delay_ms(180);
    ledOff();
  }
}

void show_number(int n){
  ledOff();

  int digit = n / 100;
  if(n>=100){
    _delay_ms(100);
    _delay_ms(100);
    show_digit(digit);
    _delay_ms(100);
  }
  digit = (n%100) / 10;
  if(n>=10){
    _delay_ms(100);
    _delay_ms(100);
    show_digit(digit);
    _delay_ms(100);
  }
  digit = n % 10;
  if(1){
    _delay_ms(100);
    _delay_ms(100);
    show_digit(digit);
    _delay_ms(100);
  }
  ledOff();
  _delay_ms(1000);
}


int main() {
  setup();
  while(1){
    uint16_t analog = analogRead(ADC_A);
    if (analog > 512) { continue; }
    _delay_ms(100);
    analog = analogRead(ADC_A);
    show_number(analog);
  }
  return 0;
}
