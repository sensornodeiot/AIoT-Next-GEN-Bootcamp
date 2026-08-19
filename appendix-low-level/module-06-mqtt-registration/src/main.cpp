#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ssc_client.h"  // provided in lib/ssc_client/

WiFiClient wifiClient;
SSCClient sscClient(wifiClient);

// Module 6 reuses the Module 5 firmware — the focus here is UNDERSTANDING the
// MQTT model (broker / topic / birth / LWT / keep-alive) and proving it, not new code.
// TODO:
// - Same setup as Module 5: connect WiFi, fill the 7-field SSCConfig from build flags,
//   sscClient.begin(config), and call sscClient.update() every loop (that keeps it online — no heartbeat).
// - Verify the device shows "online" in the SSC dashboard event log (birth message).
// - LWT test: pull USB without reset, watch it flip to "offline" within ~90 s.

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sscClient.update();   // service MQTT every iteration
}
