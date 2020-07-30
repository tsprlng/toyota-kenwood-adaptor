#define ONE_TIME 2250
#define ZERO_TIME 1120
#define REPEAT_TIME 110000
#define INPUT_1 7
#define OUTPUT_1 2

// fake no-op definition to satisfy arduino IDE, that will be replaced by external macro:
#define _key_definition(x) _delay_ms(1000)

typedef enum {
  K_NONE,
  K_VOL_DOWN,
  N_KEYS
} Key;

inline void on() { pinMode(OUTPUT_1, OUTPUT); }
inline void off() { pinMode(OUTPUT_1, INPUT); }

inline void one() {
  on();
  _delay_us(560);
  off();
  _delay_us(ONE_TIME - 560);
}
inline void zero() {
  on();
  _delay_us(560);
  off();
  _delay_us(ZERO_TIME - 560);
}

inline void repeat() {
  on();
  _delay_us(9000);
  off();
  _delay_us(2250);
   on();
  _delay_us(560);
  off();
  _delay_us(REPEAT_TIME - (9000 + 2250 + 560));
}

void _handler_dfa() {}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, LOW);
  pinMode(OUTPUT_1, OUTPUT); digitalWrite(OUTPUT_1, LOW);
  pinMode(OUTPUT_1, INPUT);
  pinMode(INPUT_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INPUT_1), _handler_dfa, CHANGE);
}

inline Key held_key() {
  Key found = K_NONE;
  if (digitalRead(INPUT_1) == LOW){
    found = found ? N_KEYS : K_VOL_DOWN;
  }
  return found >= N_KEYS ? K_NONE : found;
}

inline void send_key(Key key) {
  if (key == K_VOL_DOWN){ _key_definition(0xb915); }
}

void loop() {
  Key key = held_key();
  if (key == K_NONE){
    interrupts();
    //sleep();  // TODO: not a thing in arduino world apparently?
    return;
  }

  noInterrupts();
  digitalWrite(LED_BUILTIN, HIGH);
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
  digitalWrite(LED_BUILTIN, LOW);
}
