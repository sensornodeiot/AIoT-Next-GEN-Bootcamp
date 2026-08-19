#include "sensors.h"   // "..." = one of OUR files;  <...> = an installed library
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(SENSOR_PIN);           // SENSOR_PIN comes from a build flag
static DallasTemperature sensors(&oneWire);

static float lastTemp = 0.0f;                 // "static" = private to this file only
static bool  pending  = false;
static unsigned long cycleStart = 0, requestedAt = 0;

void sensorsBegin() {
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(false);        // makes requestTemperatures() return instantly
}

// TODO: implement the non-blocking "ask now, read later" state machine (see the lesson):
//  - every 2 s: sensors.requestTemperatures(); pending = true; requestedAt = now;
//  - 800 ms after the request: lastTemp = sensors.getTempCByIndex(0); pending = false;
void sensorsUpdate() {
  unsigned long now = millis();
  // --- Part B: DS18B20 (every 2 s) ---
    if (!pending && now - cycleStart >= 2000) {
        cycleStart = now;
        requestedAt = now;
        pending     = true;
        sensors.requestTemperatures();// returns immediately (setWaitForConversion false)

    }

    if (pending && now - requestedAt >= 800) {
        pending = false;
        float tempC = sensors.getTempCByIndex(0);
        if (tempC == DEVICE_DISCONNECTED_C) {
            Serial.printf("[%lums] DS18B20: not found\n", now);
        } else {
            lastTemp = tempC;
            Serial.printf("[%lums] temp: %.2f C\n", now, tempC);
        }
    }
}

float getTemperature() { return lastTemp; }
