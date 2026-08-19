#include <Arduino.h>
#include <WiFi.h>
#include "ssc_client.h"

WiFiClient wifiClient;
SSCClient  sscClient(wifiClient);

void setup() {
    Serial.begin(115200);

    // 1) WiFi first
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[wifi] connecting");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());

    // 2) Configure and start SSC-Client
    SSCConfig config = {
        .host          = SSC_BROKER,
        .port          = 1883,
        .clientId      = DEVICE_ID,
        .username      = DEVICE_TOKEN,
        .password      = DEVICE_PASSWORD,
        .tenantId      = TENANT_ID,
        .applicationId = APP_ID
    };
    sscClient.begin(config);

    Serial.println("Module 5 ready.");
}

void loop() {
    sscClient.update();   // service MQTT every loop — this alone keeps you online (MQTT keep-alive)
}