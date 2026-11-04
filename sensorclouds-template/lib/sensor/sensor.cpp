#include "sensor.h"
#include <esp_random.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ============================================================================
//  WORKSHOP HARDWARE
//  Everything the course wires to the SSN32 lives in this file:
//
//    DS18B20 temperature   GPIO 32  (+ 4.7 kΩ or 5.1 kΩ pull-up to 3.3 V)
//    Potentiometer / LDR   GPIO 34  (analog, 0–100 %)
//    Push button           GPIO 4   (plain button to GND, or the kit's
//                                    3-pin module: OUT → GPIO 4)
//    LED                   GPIO 2   (the SSN32's on-board LED; an external
//                                    LED + 220 Ω on GPIO 2 lights with it)
//
//  SSCApp calls sensorRead() every publish_interval_ms (10 s by default) and
//  publishes the result. It ALWAYS publishes all four values, even if nothing
//  is wired yet, because a steady publish is what keeps the device ONLINE.
//  A missing DS18B20 is sent as "no value" (NaN), never as a fake number.
//
//  Payload order (the platform decoder depends on it — keep it the same):
//    temperature, pot, button, led
// ============================================================================

#define DS18B20_PIN 32
#define POT_PIN     34
#define BUTTON_PIN  4
#define LED_PIN     2

static OneWire oneWire(DS18B20_PIN);
static DallasTemperature sensors(&oneWire);

static bool ledOn = false;

// A plain button wired to GND reads LOW when pressed. The kit's 3-pin button
// module (VCC / GND / OUT) reads HIGH when pressed. sensorBegin() checks which
// way the pin rests at power-up and treats the opposite level as "pressed",
// so both work. (Don't hold the button while the board starts.)
static int buttonPressedLevel = LOW;

// Set by the button interrupt, cleared after each publish. This way a quick
// press between two publishes still shows up as "pressed" on the dashboard.
static volatile bool buttonPressedSincePublish = false;

static void IRAM_ATTR onButtonPress() {
    buttonPressedSincePublish = true;
}

void ledSet(bool on) {
    ledOn = on;
    digitalWrite(LED_PIN, on ? HIGH : LOW);
}

bool ledIsOn() {
    return ledOn;
}

bool sensorBegin() {
    // DS18B20: "ask now, read later". A 12-bit reading takes ~750 ms, so we
    // don't wait for it: we start a conversion now and read the finished
    // result at the next publish.
    sensors.begin();
    sensors.setResolution(12);
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    Serial.printf("[Sensor] DS18B20 found: %d\n", sensors.getDeviceCount());

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(5);
    buttonPressedLevel = (digitalRead(BUTTON_PIN) == HIGH) ? LOW : HIGH;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonPress,
                    buttonPressedLevel == LOW ? FALLING : RISING);
    Serial.printf("[Sensor] Button on GPIO %d reads %s when pressed\n",
                  BUTTON_PIN, buttonPressedLevel == LOW ? "LOW" : "HIGH");

    pinMode(LED_PIN, OUTPUT);
    ledSet(false);

    return true;
}

// Average several ADC samples so the potentiometer value doesn't jump around.
static float readPotPercent() {
    const int SAMPLES = 16;
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
        sum += analogRead(POT_PIN);
    }
    return (sum / (float)SAMPLES) / 4095.0f * 100.0f;
}

static SensorReading doRead() {
    SensorReading r;

    // 1. Temperature (°C) — the result of the conversion started last time
    float t = sensors.getTempCByIndex(0);
    if (t == DEVICE_DISCONNECTED_C) {
        Serial.println("[Sensor] DS18B20 not found - temperature sent as no value");
        t = NAN;
        sensors.begin();   // look again, in case it was just plugged in
    }
    sensors.requestTemperatures();   // start the next conversion
    r.add("temperature", t);

    // 2. Potentiometer (or LDR) as 0–100 %
    r.add("pot", readPotPercent());

    // 3. Button: 1 = held now or pressed since the last publish, 0 = not pressed
    bool pressed = digitalRead(BUTTON_PIN) == buttonPressedLevel || buttonPressedSincePublish;
    buttonPressedSincePublish = false;
    r.add("button", pressed ? 1.0f : 0.0f);

    // 4. LED: 1 = on, 0 = off (switched from Sensorclouds with config:set_led)
    r.add("led", ledIsOn() ? 1.0f : 0.0f);

    Serial.printf("[Sensor] temperature=%.2f pot=%.1f%% button=%d led=%d\n",
                  t, r.params[1].value, pressed ? 1 : 0, ledIsOn() ? 1 : 0);

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
