#define ONE_TIME 2250
#define ZERO_TIME 1120
#define REPEAT_TIME 110000
#define INPUT_1 7
#define INPUT_A A2
#define OUTPUT_1 2

// fake no-op definition to satisfy arduino IDE, that will be replaced by external macro:
#define _key_definition(x) _delay_ms(1000)

typedef enum {
  K_NONE,
  K_VOL_DOWN,
  K_VOL_UP,
  K_BACK,
  K_FORWARD,
  K_MODE,
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
  pinMode(INPUT_A, INPUT_PULLUP);
  analogReference(INTERNAL);  // 2.56v
  attachInterrupt(digitalPinToInterrupt(INPUT_1), _handler_dfa, CHANGE);
}

inline Key held_key() {
  Key found = K_NONE;
  int analog = analogRead(INPUT_A);
  if (digitalRead(INPUT_1) == LOW){ found = found ? N_KEYS : K_MODE; }
  if (analog > 115 && analog < 280){ found = found ? N_KEYS : K_VOL_DOWN; }
  if (analog > 80 && analog < 105){ found = found ? N_KEYS : K_VOL_UP; }
  if (analog > 40 && analog < 80){ found = found ? N_KEYS : K_FORWARD; }
  if (analog > 10 && analog < 35){ found = found ? N_KEYS : K_BACK; }
  return found >= N_KEYS ? K_NONE : found;
    // multiple key presses => something is wrong, so do nothing
}

inline void send_key(Key key) {
  if (key == K_MODE){ _key_definition(0xb916); }  // just mute, for now: TODO detect holds/double taps?
    // play/pause is 0xb90e
    // source switch is 0xb913
  if (key == K_VOL_DOWN){ _key_definition(0xb915); }
  if (key == K_VOL_UP){ _key_definition(0xb914); }
  if (key == K_BACK){ _key_definition(0xb90a); }
  if (key == K_FORWARD){ _key_definition(0xb90b); }
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
