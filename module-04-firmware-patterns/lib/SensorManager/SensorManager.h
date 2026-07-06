#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
 public:
  explicit SensorManager(uint8_t oneWirePin);
  void begin();
  // TODO: call sensors.requestTemperatures() / getTempCByIndex(0) internally
  float readTemperatureC();
};

#endif
