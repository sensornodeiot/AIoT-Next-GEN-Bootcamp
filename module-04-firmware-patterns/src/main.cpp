#include <Arduino.h>
#include "SensorManager.h"
#include "LEDIndicator.h"

SensorManager sensorManager(SENSOR_PIN);
LEDIndicator led(LED_PIN, 300);

// TODO: run three independent timed tasks in loop(), none blocking the others:
// - led.update() every iteration (LED_PIN blink, 300ms)
// - sensor read every 2s via sensorManager.readTemperatureC()
// - uptime print every 5s

void setup() {
  Serial.begin(115200);
  sensorManager.begin();
  led.begin();
}

void loop() {
  led.update();
}
