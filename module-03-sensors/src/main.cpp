#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// TODO Part A: read POT_PIN, keep a rolling average of 10 samples, print mapped 0-100%
// TODO Part B: read DS18B20 every 2s (non-blocking) and print °C

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

void loop() {
}
