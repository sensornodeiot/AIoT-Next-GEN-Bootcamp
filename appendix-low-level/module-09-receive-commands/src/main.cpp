#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "ssc_client.h"  // provided in lib/ssc_client/

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);

// Runtime-changeable settings (plain variables, NOT const — that's what makes "no reboot" work)
unsigned long sensorInterval = 30000;  // ms
float         tempOffset     = 0.0f;

// The command callback. The library hands you RAW BYTES (not a String), so parse
// straight from (payload, length) with ArduinoJson. Register it with
// sscClient.setCommandCallback(onCommand) BEFORE sscClient.begin().
void onCommand(const char* topic, uint8_t* payload, unsigned int length) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, length)) {   // parse bytes directly
    Serial.println("[cmd] not valid JSON - ignoring");
    return;
  }

  // TODO (see the Module 9 lesson):
  // - if doc["sensorReadInterval"].is<int>(): sensorInterval = constrain(v, 100UL, 60000UL);
  // - if doc["tempOffset"].is<float>():       tempOffset = doc["tempOffset"].as<float>();
  // - Build your OWN ACK and publish it (the library does NOT auto-format an ACK):
  //       const char* cmdId = doc["commandId"];
  //       JsonDocument ack; ack["commandId"] = cmdId; ack["status"] = "ok";
  //       char buf[128]; serializeJson(ack, buf);
  //       sscClient.publishAck(buf);            // <-- takes the full JSON payload, not just the id
}

// TODO setup(): connect WiFi, sscClient.setCommandCallback(onCommand),
//               fill the 7-field SSCConfig, sscClient.begin(config).
// TODO loop():  sscClient.update() every iteration; publish telemetry using sensorInterval
//               and getTemperature()+tempOffset; heartbeat on its own timer.

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sscClient.update();
}
