#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ssc_client.h"  // see lib/ssc_client/NOTE.md

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);

// TODO: connect WiFi, configure SSCConfig from build flags, sscClient.begin()
// Verify device shows "online" in the SSC dashboard event log.

void setup() {
  Serial.begin(115200);
}

void loop() {
}
