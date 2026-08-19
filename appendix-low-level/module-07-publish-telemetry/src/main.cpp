#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ssc_client.h"  // provided in lib/ssc_client/

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

const unsigned long PUBLISH_INTERVAL = 30000;   // 30 s
unsigned long lastPublish = 0;

// Encode a temperature as the SSC hex payload: int16 (x100), big-endian, two hex bytes.
// 28.43 C -> 2843 -> 0x0B1B -> "0B1B".
String encodeTemperature(float tempC) {
  int16_t scaled = (int16_t)(tempC * 100.0f);
  char buf[8];
  snprintf(buf, sizeof(buf), "%02X%02X", (scaled >> 8) & 0xFF, scaled & 0xFF);
  return String(buf);
}

// TODO (see the Module 7 lesson):
// - Same connect sequence as Module 5 (WiFi + 7-field SSCConfig + begin + update()).
// - Every PUBLISH_INTERVAL: read the DS18B20, encodeTemperature(), and
//   sscClient.publishTelemetry(payload.c_str());   // note .c_str()  -- check the bool result.
// - Keep calling sscClient.update() every loop — that keeps the device online (no heartbeat).
// - Configure the platform to decode your hex payload so it can chart it.

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

void loop() {
  // sscClient.update();   // service MQTT every iteration
}
