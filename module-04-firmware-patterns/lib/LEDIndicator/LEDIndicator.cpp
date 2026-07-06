#include "LEDIndicator.h"

LEDIndicator::LEDIndicator(uint8_t pin, unsigned long intervalMs)
    : pin_(pin), intervalMs_(intervalMs) {}

void LEDIndicator::begin() {
  pinMode(pin_, OUTPUT);
}

void LEDIndicator::update() {
  // TODO: toggle state_ every intervalMs_ using millis(), write to pin_
}
