#include "led.h"
#include <Arduino.h>

static unsigned long interval = 500;
static unsigned long last = 0;
static bool state = false;

void ledBegin() { pinMode(LED_PIN, OUTPUT); }          // LED_PIN comes from a build flag
void ledSetInterval(unsigned long ms) { interval = ms; }

void ledUpdate() {
  unsigned long now = millis();
  if (now - last >= interval) {
    last = now;
    state = !state;
    digitalWrite(LED_PIN, state);   // true = HIGH, false = LOW
  }
}
