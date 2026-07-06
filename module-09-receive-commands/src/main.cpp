#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "ssc_client.h"  // see lib/ssc_client/NOTE.md

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);

unsigned long sensorReadIntervalMs = 2000;
float tempOffset = 0.0f;

// TODO: implement onConfig(payload) callback:
// - parse JSON { field, value, commandId }
// - apply sensorReadInterval / tempOffset at runtime
// - sscClient.publishAck(commandId)

void setup() {
  Serial.begin(115200);
}

void loop() {
}
