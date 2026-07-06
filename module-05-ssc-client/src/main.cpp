#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ssc_client.h"  // see lib/ssc_client/NOTE.md

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);

// TODO:
// - WiFi.begin(WIFI_SSID, WIFI_PASSWORD), wait for WL_CONNECTED
// - Build an SSCConfig from SSC_HOST/SSC_PORT/SSC_DEVICE_ID/SSC_TOKEN build flags
// - sscClient.begin(config) in setup()
// - sscClient.loop() every iteration of loop()
// - Confirm Serial shows: WiFi connected -> MQTT connected -> birth published

void setup() {
  Serial.begin(115200);
}

void loop() {
}
