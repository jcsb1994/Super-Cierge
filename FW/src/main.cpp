//***********************************************************************************
// Copyright 2022 JCSB1994
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//***********************************************************************************
//
// Description:
//    This project operates a rgb led inserted in a candle
//    the candle pulses red and green until it goes to sleep after ~1h inactivity
//    Once shaken, it flickers blue and red
//
//***********************************************************************************

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define R_LED_PIN (0)
#define G_LED_PIN (1)
#define B_LED_PIN (3) // no PWM on this pin

#define VIBRATION_SWITCH_PIN (2)

// r g sequential mode
// glitters white when hit

// TODO: srand() vs rand()  
// which pins are pwm?
// 

ISR(PCINT0_vect)
{
}

inline void init_leds() {
  DDRB |= 1 << R_LED_PIN;
  DDRB |= 1 << G_LED_PIN;
  DDRB |= 1 << B_LED_PIN;
}

inline void init_vib_switch() {
  DDRB &= ~(1 << VIBRATION_SWITCH_PIN);
  PORTB |= (1 << VIBRATION_SWITCH_PIN);
}

inline void led_on(uint8_t pin) {
  PORTB |= (1 << pin);  
}

inline void led_off(uint8_t pin) {
  // PORTB &= ~(1 << pin);
  digitalWrite(pin, LOW);
}

void glitter() {

    uint8_t rollValue = rand() & 0xff; // max led value cap
    // PWM on which pin??

    analogWrite(R_LED_PIN, rollValue);
    analogWrite(G_LED_PIN, rollValue);
    analogWrite(B_LED_PIN, rollValue);
    delay(50);
}

void setup()
{
  init_leds();
  init_vib_switch();
  
  // Enable interrupts, enable input state change interrupt on GPIO 2
  GIMSK |= (1 << PCIE);
  PCMSK |= _BV(PCINT2);
  sei();

  ADCSRA &= ~_BV(ADEN); // Turn ADC off, saves ~230uA (flip that bit to turn it back on)

  // sleep setup
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}


void step_christmas_glow()
{
  static uint8_t r = 0;
  static uint8_t g = 254;
  static uint8_t t = 0;
  
  analogWrite(R_LED_PIN, r);
  analogWrite(G_LED_PIN, g);
  led_off(B_LED_PIN);

  if (!t) {
    if (!r)
    {
      t = 1;
    } else {
    g+=2;
    r-=2;
    }
  } else {
    r+=2;
    g-=2;
    if (g == 0) {
      t = 0;
    }
  }
}

uint32_t steps = 0; // steps 10ms until 4000 seconds (>1h)
uint8_t state = 0;

void loop()
{
  delay(10);
  steps++;
  
  if (digitalRead(VIBRATION_SWITCH_PIN) == LOW)
  {
    state = 1;
  }
  

  if (steps >= 400000) {
    state = 2;
  }

  switch (state)
  {
  case 0:
    step_christmas_glow();
    break;

  case 1:
    glitter();
    if (steps >= 30)
      {
        state = 0;
        steps = 0;
      }
    break;

  case 2:
    led_off(R_LED_PIN);
    led_off(G_LED_PIN);
    led_off(B_LED_PIN);
    sleep_enable();
    sleep_cpu();
    state = 1;
    steps = 0;
    break;

  default:
    break;
  }
}
