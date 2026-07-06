#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <Arduino.h>

class LEDIndicator {
 public:
  LEDIndicator(uint8_t pin, unsigned long intervalMs);
  void begin();
  // Call every loop() iteration; non-blocking.
  void update();

 private:
  uint8_t pin_;
  unsigned long intervalMs_;
  unsigned long lastToggleMs_ = 0;
  bool state_ = false;
};

#endif
