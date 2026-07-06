#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ssc_client.h"  // see lib/ssc_client/NOTE.md

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// TODO:
// - Encode DS18B20 reading as int16_t x100, big-endian hex string
// - sscClient.publishTelemetry(hexString) every 30s
// - Send a heartbeat every 5 minutes

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

void loop() {
}
