#include <Arduino.h>

// TODO:
// - Blink LED_PIN using millis() at a rate that toggles between 500ms and 100ms
// - Read BUTTON_PIN (INPUT_PULLUP) with software debounce
// - Each confirmed press toggles the blink rate and prints to Serial

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
}
