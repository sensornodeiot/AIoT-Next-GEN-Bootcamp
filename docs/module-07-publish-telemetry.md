
# Module 7 — Publish Your Real Sensor

**Goal:** Replace the template's placeholder value with a **real DS18B20 temperature**, by editing the one file the template leaves for you: `lib/sensor/sensor.cpp`. Then watch real temperature arrive in SSC.

**Audience:** Completed [module-05-ssc-client](module-05-ssc-client.md) (device online via the template) and [module-06-mqtt-ssc-registration](module-06-mqtt-ssc-registration.md).

**Estimated time:** 1.5–2 hours.

**Deliverable:** Your actual room temperature updating in the SSC dashboard every publish interval; holding the sensor makes the number rise.

---

## 1. Where your sensor code goes

The template already does *everything* about publishing — timing, encoding, sending, staying online. The only thing it can't know is **your** hardware. So it calls two functions that live in **`lib/sensor/sensor.cpp`**, and those are the **only** functions you edit:

| Function | When it runs | Your job |
|----------|-------------|----------|
| `sensorBegin()` | once, at startup | initialise your sensor hardware |
| `doRead()` | every publish interval | read the sensor, `r.add("name", value)` |

Everything below the marked line in that file (the hex encoding) you **do not touch** — the template turns your named values into the format SSC expects.

> You never call `publishTelemetry` yourself here. `SSCApp` calls `sensorRead()` on its timer and publishes whatever `doRead()` returns (when `r.valid` is true). That timed publish is what keeps the device online.

---

## 2. Add the sensor libraries

▶ Open the template's `platformio.ini` and add the two DS18B20 libraries to `lib_deps` (keep what's already there):

```ini
lib_deps =
    knolleary/PubSubClient
    bblanchon/ArduinoJson
    paulstoffregen/OneWire
    milesburton/DallasTemperature
```

---

## 3. Wire the DS18B20

| DS18B20 | SSN32 |
|---------|-------|
| VCC (red) | 3.3 V |
| GND (black) | GND |
| DATA (yellow) | GPIO 32 **+ a 4.7 kΩ resistor from DATA to 3.3 V** |

The 4.7 kΩ pull-up is **required** — without it the sensor reads `-127` / not-found.

---

## 4. Edit `lib/sensor/sensor.cpp`

Change only the two marked functions. Here's the DS18B20 version:

```cpp
#include "sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_random.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <Arduino.h>

#define DS18B20_PIN 32
static OneWire oneWire(DS18B20_PIN);
static DallasTemperature sensors(&oneWire);

// ── EDIT THIS (1/2): initialise your sensor ──
bool sensorBegin() {
    sensors.begin();
    sensors.setResolution(12);
    return true;
}

// ── EDIT THIS (2/2): read your sensor, add named values ──
static SensorReading doRead() {
    SensorReading r;
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    if (t == DEVICE_DISCONNECTED_C) {
        Serial.println("[Sensor] DS18B20 not found");
        return r;                     // r.valid stays false -> nothing is published this cycle
    }
    r.add("temperature", t);          // one named parameter
    r.valid = true;
    return r;
}

// ===== DO NOT EDIT BELOW — the template's hex encoding =====
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
bool powerRead(char* /*hexOut*/, size_t /*hexLen*/) { return false; }
```

> **You can publish more than one value.** Add more `r.add("name", value)` lines — humidity, pressure, a battery level — up to 16. The platform decodes each named float in order.

---

## 5. What the payload looks like (you don't build it)

The template sends a compact hex string: a **4-character random nonce**, then **8 hex characters (one IEEE-754 float) per value**. One temperature looks like:
```
a3f2  41c9999a          nonce="a3f2", temperature=25.2 °C
```
You never assemble this — `r.add("temperature", 25.2)` is all you write. The platform is configured to decode it (a **codec**, set up on the platform side — covered in the Sensorclouds course).

---

## 6. Build, upload, verify

```
pio run -e esp32dev -t upload
pio device monitor -e esp32dev
```

👁 Serial: `[Sensor] ...` then a `TX <hex>` line every publish interval. In SSC → your device → **Live Data**, the `temperature` value now reflects the **real** room temperature. **Hold the sensor** in your hand → the next value rises.

⚠ IF real temperature isn't appearing:
- `-127` in a decoded reading → check the 4.7 kΩ pull-up and wiring.
- `TX` prints but SSC shows nothing → the platform's decoder for this device isn't set (ask your instructor / see the Sensorclouds course).

---

## 7. Deliverable Checklist

- [ ] `lib/sensor/sensor.cpp` reads the DS18B20 in `doRead()` (`r.add("temperature", t)`)
- [ ] OneWire + DallasTemperature added to `lib_deps`, project builds
- [ ] Serial shows a `TX` line every publish interval
- [ ] SSC Live Data shows real temperature; holding the sensor changes it
- [ ] You can explain: you only edit `doRead()`; the template handles timing, encoding, and publishing

---

## 8. What's Next

**[module-08-ssc-dashboard](module-08-ssc-dashboard.md)** — build charts and gauges from your temperature (platform side). Then **[module-09-receive-commands](module-09-receive-commands.md)** — change the device's behaviour from the cloud.
