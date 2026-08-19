
# Module 3 — Sensors: Analog & Digital

**Goal:** Read both analog and digital sensors, understand the difference between them, and wire up the DS18B20 temperature sensor used in all subsequent modules.

**Audience:** Completed [module-02-gpio](module-02-gpio.md).

**Estimated time:** 2–3 hours (reading + lab)

**Deliverable:** Smooth analog percentage from a potentiometer and live DS18B20 temperature readings — both running in one `loop()` with no blocking.

---

## 1. Analog vs Digital Sensors

| Property   | Analog                          | Digital                                            |
| ---------- | ------------------------------- | -------------------------------------------------- |
| Output     | Continuous voltage (0–3.3 V)    | Discrete values over a protocol (1-Wire, I2C, SPI) |
| Resolution | Limited by ADC                  | Set in sensor firmware (9–12 bit for DS18B20)      |
| Noise      | Susceptible to electrical noise | Immune — data is encoded                           |
| Wiring     | One signal wire                 | One or two signal wires; more complex protocol     |
| Examples   | LDR, potentiometer, thermistor  | DS18B20, BME280, DHT22                             |

Analog sensors are simple to wire but produce noisy readings. Digital sensors require a library but give clean, calibrated data. The DS18B20 is the right choice for temperature in IoT applications — it is accurate to ±0.5 °C out of the box.

---

## 2. ADC: Reading Analog Signals

The **ADC (Analog-to-Digital Converter)** on the ESP32 converts a voltage (0–3.3 V) to a 12-bit integer (0–4095).

```cpp
int raw = analogRead(34);   // returns 0–4095
```

### Mapping raw values

To convert a raw ADC reading to a percentage (0–100 %):

```cpp
float percent = (analogRead(34) / 4095.0f) * 100.0f;
```

Use `4095.0f` (not `4095`) to force floating-point division.

### ESP32 ADC caveats

- Only certain pins support ADC: GPIO 32–39 on the SSN32 (use GPIO 34 for input-only analog reads)
- **ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) are unavailable when Wi-Fi is active** — use ADC1 pins (32–39) for anything that runs alongside Wi-Fi
- The ESP32 ADC has non-linearity at the edges (near 0 V and near 3.3 V). Do not rely on readings below ~100 mV or above ~3.1 V for precise measurements

---

## 3. Noise and Averaging

ADC readings fluctuate due to electrical noise. A single raw reading may jump ±20–50 counts. Use a **rolling average** to smooth the signal:

```cpp
const int NUM_SAMPLES = 10;
int samples[NUM_SAMPLES] = {};
int sampleIndex = 0;

void readAnalog() {
    samples[sampleIndex] = analogRead(34);
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;   // next slot; wraps 9 → 0

    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) sum += samples[i];
    float avg = sum / (float)NUM_SAMPLES;

    float percent = (avg / 4095.0f) * 100.0f;
    Serial.printf("[%lums] pot: %.1f%%\n", millis(), percent);
}
```

> **What is `% NUM_SAMPLES`?** `%` gives the **remainder** of a division. Here it keeps `sampleIndex` cycling `0, 1, 2, … 9, 0, 1, …` so we always overwrite the *oldest* of the 10 stored readings — a "rolling" window of the last 10 samples. Without the wrap, the index would keep growing and run off the end of the array.

Calling `readAnalog()` every 50–100 ms fills the buffer quickly and produces a stable reading within the first second.

---

## 4. Digital Sensors and 1-Wire

The **DS18B20** communicates over the **1-Wire** protocol — a single data line that carries both power and data. Up to 64 sensors can share one wire, each identified by a unique 64-bit ROM address.

### Why 1-Wire for temperature?

- One GPIO pin reads an arbitrary number of sensors
- Data is noise-immune (digital encoding)
- Factory-calibrated: no user calibration required
- Wide range: −55 °C to +125 °C; ±0.5 °C accuracy from −10 to +85 °C

### Wiring

```
SSN32 3.3V ──────────────────── DS18B20 VCC (red)
                │
               [4.7kΩ]          ← pull-up resistor (required)
                │
SSN32 GPIO 32 ──┤─────────────── DS18B20 DATA (yellow)

SSN32 GND ──────────────────── DS18B20 GND (black)
```

The 4.7 kΩ pull-up between VCC and DATA is **required** — without it, the bus has no defined idle state and communication will fail.

---

## 5. OneWire and DallasTemperature Libraries

Add both libraries to `platformio.ini`:

```ini
lib_deps =
    paulstoffregen/OneWire @ ^2.3.8
    milesburton/DallasTemperature @ ^3.11.0
```

### Usage

```cpp
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_PIN 32

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(115200);
    sensors.begin();
    sensors.setResolution(12);  // 12-bit: 0.0625 °C steps, 750 ms conversion
}

void loop() {
    sensors.requestTemperatures();          // trigger conversion
    float tempC = sensors.getTempCByIndex(0);  // read first sensor

    if (tempC == DEVICE_DISCONNECTED_C) {
        Serial.println("[error] DS18B20 not found");
    } else {
        Serial.printf("[%lums] temp: %.2f C\n", millis(), tempC);
    }

    delay(2000);  // will be replaced with millis() in the lab
}
```

### Resolution vs conversion time

| Resolution | Step size | Conversion time |
|------------|-----------|----------------|
| 9-bit | 0.5 °C | 94 ms |
| 10-bit | 0.25 °C | 188 ms |
| 11-bit | 0.125 °C | 375 ms |
| 12-bit | 0.0625 °C | 750 ms |

`requestTemperatures()` is a **blocking** call for the duration of the conversion. In the lab you will make it non-blocking using millis().

---

## 6. Non-Blocking Sensor Reads

`requestTemperatures()` normally **blocks** for the full conversion time — 750 ms at 12-bit. That freezes the entire loop. The fix is one setup call:

```cpp
sensors.setWaitForConversion(false);  // call once in setup()
```

With this set, `requestTemperatures()` returns immediately after triggering the conversion. The loop keeps running. You check back 800 ms later to collect the result.

Use a **bool flag** to track whether a conversion is in progress:

```cpp
bool          tempPending     = false;
unsigned long tempRequestedAt = 0;
unsigned long lastTempCycle   = 0;

// Trigger a new conversion every 2 s
if (now - lastTempCycle >= 2000) {
    lastTempCycle = now;
    sensors.requestTemperatures();
    tempPending     = true;
    tempRequestedAt = now;
}

// Read result 800 ms after triggering (12-bit conversion takes at most 750 ms)
if (tempPending && now - tempRequestedAt >= 800) {
    tempPending = false;
    float tempC = sensors.getTempCByIndex(0);
    // use tempC
}
```

Two events per 2-second cycle: trigger at the top, read 800 ms later. The loop is free the entire time.

---

## 7. Lab — Potentiometer + DS18B20

### Hardware

| Component | Connection |
|-----------|-----------|
| Potentiometer wiper | GPIO 34 |
| Potentiometer ends | 3.3 V and GND |
| DS18B20 VCC | 3.3 V |
| DS18B20 DATA | GPIO 32 (with 4.7 kΩ to 3.3 V) |
| DS18B20 GND | GND |

### platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    paulstoffregen/OneWire @ ^2.3.8
    milesburton/DallasTemperature @ ^3.11.0
```

### Complete code

```cpp
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
```

### Expected Serial output

```
Module 3 ready.
[100ms]  pot: 47.3%
[200ms]  pot: 47.5%
...
[2000ms] temp: 28.43 C
[2100ms] pot: 52.1%
...
[4000ms] temp: 28.44 C
```

Both sensors report independently. Neither blocks the other.

---

## 8. Deliverable Checklist

- [x] Potentiometer reading prints a rolling average as a percentage every 100 ms
- [x] DS18B20 temperature prints every 2 seconds
- [x] Both run in the same `loop()` with no `delay()`
- [x] No `DEVICE_DISCONNECTED_C` errors (sensor is wired correctly with pull-up)
- [x] You can explain why the 4.7 kΩ pull-up is required

---

## 9. What's Next

In **[module-04-firmware-patterns](module-04-firmware-patterns.md)** you will:
- Split this project into tidy files (`sensors.h/.cpp`, `led.h/.cpp`) using plain functions — so `main.cpp` reads like a summary
- Replace hardcoded pin numbers with `platformio.ini` build flags
- Run three independent timed tasks in a clean, readable `loop()`
