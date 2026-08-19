#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "ssc_client.h"    // provided in lib/ssc_client/
#include "ota_handler.h"   // provided in lib/ota_handler/

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);
OtaHandler otaHandler;

// The device's current version — reported automatically in the birth message.
// It comes from the FIRMWARE_VERSION build flag; bump it in platformio.ini per release.

void onCommand(const char* topic, uint8_t* payload, unsigned int length) {
  // OTA commands arrive on the same "cmd" topic. Route the "ota:" ones to the handler;
  // handle everything else (Module 9 config commands) yourself.
  char buf[256];
  unsigned int n = length < 255 ? length : 255;
  memcpy(buf, payload, n);
  buf[n] = '\0';

  if (strstr(buf, "\"ota:") != nullptr) {
    otaHandler.handleMessage(topic, payload, length);
    return;
  }
  // ...otherwise, your Module 9 config handling...
}

// TODO (see the Module 10 lesson):
// setup(), after sscClient.begin(config):
//   - sscClient.setCommandCallback(onCommand);   // before begin()
//   - otaHandler.begin(&sscClient.getMqttClient(), sscClient.getAckTopic());
//   - otaHandler.checkPendingAck();   // once, shortly after first connect: confirms a prior OTA
// loop():
//   - sscClient.update();
//   - otaHandler.update();            // drives download/verify/install, non-blocking
// Then: build (don't upload) a v1.0.1 image, push it from the SSC portal, watch it
//       reboot and report the new FIRMWARE_VERSION in the birth message.

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sscClient.update();
  // otaHandler.update();
}
