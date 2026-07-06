#include "SensorManager.h"

SensorManager::SensorManager(uint8_t oneWirePin) {
  // TODO: construct OneWire + DallasTemperature with oneWirePin
}

void SensorManager::begin() {
  // TODO: sensors.begin()
}

float SensorManager::readTemperatureC() {
  // TODO: return sensors.getTempCByIndex(0) after requestTemperatures()
  return NAN;
}
