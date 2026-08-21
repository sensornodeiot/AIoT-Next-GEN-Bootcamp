#include "sensor.h"
#include <esp_random.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ============================================================================
//  WORKSHOP SENSOR
//  You edit ONLY the two functions marked "EDIT THIS". SSCApp calls sensorRead()
//  every publish_interval_ms and publishes the result — that steady publish is
//  what keeps your device ONLINE.
// ============================================================================

#define DS18B20_PIN 32
static OneWire oneWire(DS18B20_PIN);
static DallasTemperature sensors(&oneWire);

// ── EDIT THIS (1/2): initialize your sensor hardware, once at startup ──
bool sensorBegin() {
    // Module 5: there is no real sensor yet — nothing to initialize.
    // Module 7: for the DS18B20 you'll do the OneWire/Dallas setup here, e.g.
      sensors.begin();
      sensors.setResolution(12);
      pinMode(4, INPUT_PULLUP);   // button: pressed = LOW
    return true;   // return false only if your hardware failed to start
}

// ── EDIT THIS (2/2): read your sensor, add one or more NAMED values ──
static SensorReading doRead() {
    SensorReading r;

    // Module 5 starter: a fake temperature that drifts around 25 °C, so the whole
    // pipeline works with NO extra hardware and the device stays online.
    // In Module 7 you replace this with a real DS18B20 reading:
    //   float t = sensors.getTempCByIndex(0);
    //   if (t == DEVICE_DISCONNECTED_C) return r;   // r.valid stays false -> skip publish
    //   r.add("temperature", t);
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    if (t == DEVICE_DISCONNECTED_C) {
        Serial.println("[Sensor] DS18B20 not found");
        return r;
    }
    r.add("temperautre", t);
    // potentiometer as a 0–100 % value
    r.add("pot", analogRead(34) / 4095.0f * 100.0f);
    // button state: 1 = pressed, 0 = released
    r.add("button", digitalRead(4) == LOW ? 1.0f : 0.0f);

    r.valid = true;   // must be true or nothing is published
    return r;
}

// ============================================================================
//  DO NOT EDIT BELOW — hex encoding (4-char nonce + one 8-char IEEE-754 float
//  per named parameter). This is the format the Sensorclouds platform decodes.
// ============================================================================

static bool encodeHex(const SensorReading& r, char* hexOut, size_t hexLen) {
    if (!r.valid) return false;
    uint8_t offset = 0;
    uint16_t nonce = (uint16_t)(esp_random() & 0xFFFF);
    offset += snprintf(hexOut + offset, hexLen - offset, "%04x", nonce);
    for (uint8_t i = 0; i < r.count; i++) {
        uint32_t raw;
        memcpy(&raw, &r.params[i].value, sizeof(raw));
        offset += snprintf(hexOut + offset, hexLen - offset, "%08" PRIx32, raw);
    }
    return true;
}

bool sensorRead(char* hexOut, size_t hexLen) {
    SensorReading r = doRead();
    return encodeHex(r, hexOut, hexLen);
}

// The workshop template does not use the second (power-meter) publish path.
bool powerRead(char* /*hexOut*/, size_t /*hexLen*/) { return false; }
