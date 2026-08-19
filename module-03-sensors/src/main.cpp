#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define POT_PIN    34
#define DS18B20_PIN 32

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Rolling average for ADC
const int NUM_SAMPLES = 10;
int adcBuf[NUM_SAMPLES] = {};
int adcIdx = 0;

// Timers and DS18B20 state
unsigned long lastPotRead     = 0;
unsigned long lastTempCycle   = 0;
unsigned long tempRequestedAt = 0;
bool          tempPending     = false;

float smoothedPercent = 0.0f;

void setup() {
    Serial.begin(115200);
    sensors.begin();
    sensors.setResolution(12);
    sensors.setWaitForConversion(false);  // makes requestTemperatures() return immediately
    Serial.println("Module 3 ready.");
}

void loop() {
    unsigned long now = millis();

    // --- Part A: potentiometer (every 100 ms) ---
    if (now - lastPotRead >= 100) {
        lastPotRead = now;
        adcBuf[adcIdx] = analogRead(POT_PIN);
        adcIdx = (adcIdx + 1) % NUM_SAMPLES;   // next slot; wraps 9 → 0

        long sum = 0;
        for (int i = 0; i < NUM_SAMPLES; i++) sum += adcBuf[i];
        smoothedPercent = (sum / (float)NUM_SAMPLES / 4095.0f) * 100.0f;

        Serial.printf("[%lums] pot: %.1f%%\n", now, smoothedPercent);
    }

    // --- Part B: DS18B20 (every 2 s) ---
    if (now - lastTempCycle >= 2000) {
        lastTempCycle = now;
        sensors.requestTemperatures();  // returns immediately (setWaitForConversion false)
        tempPending     = true;
        tempRequestedAt = now;
    }

    if (tempPending && now - tempRequestedAt >= 800) {
        tempPending = false;
        float tempC = sensors.getTempCByIndex(0);
        if (tempC == DEVICE_DISCONNECTED_C) {
            Serial.printf("[%lums] DS18B20: not found\n", now);
        } else {
            Serial.printf("[%lums] temp: %.2f C\n", now, tempC);
        }
    }
}