#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ssc_client.h"    // see lib/ssc_client/NOTE.md
#include "ota_handler.h"   // see lib/ota_handler/NOTE.md

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);
OtaHandler otaHandler;

// TODO:
// - Report FIRMWARE_VERSION in the birth message
// - checkPendingAck() on boot to confirm a prior OTA succeeded
// - Trigger OTA update from the SSC portal, verify new FIRMWARE_VERSION after reboot

void setup() {
  Serial.begin(115200);
}

void loop() {
}
