# Module 4 — Firmware Patterns & Code Structure

**Goal:** Learn two things that separate a toy sketch from real firmware: (1) how to run several tasks at once without one freezing the others, and (2) how to split a growing program into tidy files so `main.cpp` stays readable.

**Audience:** Completed [module-03-sensors](module-03-sensors.md).

**Estimated time:** 2–3 hours (reading + lab)

**Deliverable:** The Module 3 code, reorganized into separate files with plain functions, all pins moved to build flags, and three timed tasks running independently with no `delay()`.

> **Instructor note:** This is the "tidy up" module before the cloud work begins. The single most valuable habit to instil here is the non-blocking mindset — from Module 5 onward, a single stray `delay()` will silently knock a device offline. We deliberately do **not** teach students to write their own C++ classes here; they *use* library objects (like the sensor) but organize *their own* code with plain functions. Writing classes is an optional box at the end for anyone racing ahead.

---

## 1. Everything Shares One CPU

The ESP32 runs your program on a single core (by default), with **no operating system** taking turns between tasks. Your `loop()` runs top to bottom, over and over, millions of times a second.

That means: **if one line waits, everything waits.** A `delay(1000)` doesn't pause "just the blink" — it freezes the whole board for a full second: no sensor reads, no button checks, and (soon) no keeping the WiFi/cloud connection alive.

> **Why this matters now:** From Module 5 on, your board must service its internet connection constantly. A blocking call is the #1 cause of a device mysteriously dropping offline. The habit you build here is what prevents that.

The fix is the pattern you already met in Modules 2 and 3: break work into **short steps** and use timestamps to decide when to run each one.

---

## 2. The Non-Blocking Timer Pattern (the one pattern to remember)

```cpp
unsigned long lastRun = 0;
const unsigned long INTERVAL_MS = 1000;

void loop() {
    unsigned long now = millis();               // one time snapshot per loop

    if (now - lastRun >= INTERVAL_MS) {         // has enough time passed?
        lastRun = now;                          // remember when we ran
        doWork();                               // do a quick job and return
    }
}
```

Three rules:
- Read `millis()` **once** at the top of `loop()` and reuse that snapshot.
- Update `lastRun` **before** doing the work.
- Keep `doWork()` short — it should finish fast and hand control back.

Running several tasks is just several of these, each with its **own** timer variable:

```cpp
if (now - lastSensor >= 2000) { lastSensor = now; readSensor(); }
if (now - lastBlink  >= 500)  { lastBlink  = now; toggleLed(); }
if (now - lastUptime >= 5000) { lastUptime = now; printUptime(); }
```

They don't interfere — each simply runs when its own interval elapses.

---

## 3. State Machines — for Jobs That Take Time

Some jobs can't finish in one step. The DS18B20 (Module 3) needs ~750 ms *after* you ask before the reading is ready. You can't `delay(750)` — that would freeze everything. So you track **what stage the job is in** and move through the stages across many loop iterations.

That "what stage am I in" variable is a **state machine**:

```cpp
enum SensorStage { IDLE, WAITING };   // the possible stages
SensorStage stage = IDLE;
unsigned long startedAt = 0;

void updateSensor() {
    unsigned long now = millis();
    switch (stage) {
        case IDLE:
            sensors.requestTemperatures();   // ask for a reading
            startedAt = now;
            stage = WAITING;                 // move to the next stage
            break;
        case WAITING:
            if (now - startedAt >= 800) {    // reading is ready now
                lastTemp = sensors.getTempCByIndex(0);
                stage = IDLE;                // back to the start
            }
            break;
    }
}
```

Call `updateSensor()` every loop. Each call does a tiny bit and returns immediately — the board stays free the whole time.

> **Why bother with stages instead of just waiting?** Because "ask, then come back later for the answer" is how you do *anything* slow (a sensor conversion, a network reply, a motor move) without freezing the rest of the program. It's the non-blocking timer pattern applied to a multi-step job.

---

## 4. Why Split Code Into Files?

So far everything lived in `main.cpp`. That's fine for 40 lines. But once you add WiFi, the cloud client, sensors, and command handling, `main.cpp` becomes hundreds of lines and impossible to scan.

The fix: put **related code in its own file**, so `main.cpp` reads like a summary of the system, not the whole story. All the sensor details live in `sensors.cpp`; `main.cpp` just calls `getTemperature()`.

Each "module of code" is a **pair of files**:

| File                       | Its job                                                                           |
| -------------------------- | --------------------------------------------------------------------------------- |
| `sensors.h` (**header**)   | The short list of what this file offers — the function names other files can call |
| `sensors.cpp` (**source**) | The actual code behind those functions                                            |
|                            |                                                                                   |

> **The mental model:** the `.h` is the *menu* (what you can order), the `.cpp` is the *kitchen* (how it's made). Other files read the menu; they never need to see inside the kitchen.

For this course, put these files right in the `src/` folder next to `main.cpp` — PlatformIO compiles every `.cpp` in `src/` automatically. (There's also a `lib/` folder for code you want to reuse across *different* projects — you'll see that when we pull in the SSC client library.)

---

## 5. Writing a Header + Source Pair (with plain functions)

You will **use** library classes in this course (the sensor object, the cloud client). But you don't need to *write* classes to organise your own code — plain functions are simpler and enough. Here's the sensor code from Module 3, moved into its own pair of files:

```cpp
// src/sensors.h
#pragma once            // "only include this file once" — prevents duplicate-definition errors

void  sensorsBegin();   // set up the sensor — call once in setup()
void  sensorsUpdate();  // call every loop — reads the sensor without blocking
float getTemperature(); // returns the most recent temperature in °C
```

```cpp
// src/sensors.cpp
#include "sensors.h"    // "..." means one of OUR files;  <...> means an installed library
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(SENSOR_PIN);            // SENSOR_PIN comes from a build flag (Section 6)
static DallasTemperature sensors(&oneWire);

static float lastTemp = 0.0f;                  // "static" = private to this file only
static bool  pending  = false;
static unsigned long cycleStart = 0, requestedAt = 0;

void sensorsBegin() {
    sensors.begin();
    sensors.setResolution(12);
    sensors.setWaitForConversion(false);       // makes requestTemperatures() return instantly
}

void sensorsUpdate() {
    unsigned long now = millis();
    if (!pending && now - cycleStart >= 2000) {     // every 2 s, ask for a new reading
        cycleStart = now;
        sensors.requestTemperatures();
        pending = true; requestedAt = now;
    }
    if (pending && now - requestedAt >= 800) {      // 800 ms later, collect it
        pending = false;
        lastTemp = sensors.getTempCByIndex(0);
    }
}

float getTemperature() { return lastTemp; }
```

> **Why `#pragma once`?** A header can end up included twice (directly and through another file). Without a guard, the compiler sees the same declarations twice and errors. `#pragma once` tells it "include me only once" — one line, problem solved.

> **Why `static` on the file-scope variables?** It keeps them private to `sensors.cpp`. Nothing in `main.cpp` can accidentally reach in and change `lastTemp`. Other files can only go through `getTemperature()` — a clean, controlled door. That's encapsulation, achieved without writing a class.

---

## 6. Build Flags — Stop Hardcoding Pin Numbers

A pin number buried in code (`OneWire oneWire(32);`) is easy to lose track of. Put it in `platformio.ini` as a **build flag** instead, and refer to it by name:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
build_flags =
    -DSENSOR_PIN=32
    -DLED_PIN=2
lib_deps =
    paulstoffregen/OneWire @ ^2.3.8
    milesburton/DallasTemperature @ ^3.11.0
```

Now `SENSOR_PIN` works anywhere in your code as if you'd typed `32`. To move the sensor to another pin, you edit **one line** in `platformio.ini` — no hunting through source files.

> **Why do it this way?** All your hardware choices end up in one place (`platformio.ini`), and the same code can be built for different boards just by changing flags. Note: build flags are for things that are the same for every board (pins, version numbers). **Passwords and Wi-Fi details never go in build flags or code:** from Module 5 you type them into the device's own setup web page, and the board saves them in its memory.

### Multiple environments (optional)

You can define more than one build target — handy if you have two boards wired differently:

```ini
[env:bench]
extends = env:esp32dev
build_flags = -DSENSOR_PIN=15 -DLED_PIN=4
```

Build a specific one with `pio run -e bench`.

---

## 7. Lab — Reorganise Module 3

### Objective

Take your working Module 3 sketch and split it into clean files:
- `src/sensors.h` + `src/sensors.cpp` — the DS18B20 code (from Section 5)
- `src/led.h` + `src/led.cpp` — the LED blink code
- `platformio.ini` build flags for both pins
- A short `main.cpp` running three independent tasks

### `src/led.h` and `src/led.cpp`

```cpp
// src/led.h
#pragma once
void ledBegin();
void ledSetInterval(unsigned long ms);
void ledUpdate();   // call every loop
```

```cpp
// src/led.cpp
#include "led.h"
#include <Arduino.h>

static unsigned long interval = 500;
static unsigned long last = 0;
static bool state = false;

void ledBegin() { pinMode(LED_PIN, OUTPUT); }
void ledSetInterval(unsigned long ms) { interval = ms; }

void ledUpdate() {
    unsigned long now = millis();
    if (now - last >= interval) {
        last = now;
        state = !state;
        digitalWrite(LED_PIN, state);   // true = HIGH, false = LOW (same trick as Module 2)
    }
}
```

### `src/main.cpp`

```cpp
#include <Arduino.h>
#include "sensors.h"
#include "led.h"

unsigned long lastTempPrint = 0;
unsigned long lastUptime    = 0;

void setup() {
    Serial.begin(115200);
    sensorsBegin();
    ledBegin();
    ledSetInterval(300);
    Serial.println("Module 4 ready.");
}

void loop() {
    unsigned long now = millis();

    ledUpdate();       // blinks at 300 ms internally
    sensorsUpdate();   // reads temperature every 2 s internally

    if (now - lastTempPrint >= 2000) {          // print temp every 2 s
        lastTempPrint = now;
        Serial.printf("[%lums] temp: %.2f C\n", now, getTemperature());
    }

    if (now - lastUptime >= 5000) {             // print uptime every 5 s
        lastUptime = now;
        Serial.printf("[%lums] uptime: %lus\n", now, now / 1000);
    }
}
```

*(Reminder from Module 1: `%lums` is not one code — it's `%lu` (an unsigned long, which is what `millis()` returns) followed by the plain letters `ms`.)*

### Verify your work
- `main.cpp` contains **no** raw pin numbers and **no** sensor/LED details — just calls
- No `delay()` anywhere in the project
- LED blinks at 300 ms; temperature prints every 2 s; uptime every 5 s — all independent
- Changing `SENSOR_PIN` in `platformio.ini` is the only edit needed to move the sensor

---

## 8. Go Further (optional) — Wrapping Code in a Class

Once you're comfortable with function-based files, the next step professionals take is bundling a module's data and functions into a **class**. You've already been *using* classes — `DallasTemperature sensors(&oneWire)` is one. Writing your own looks like this:

```cpp
class Led {
public:
    Led(uint8_t pin, unsigned long interval) : _pin(pin), _interval(interval) {}
    void begin() { pinMode(_pin, OUTPUT); }
    void update() { /* same millis() logic, using _pin and _interval */ }
private:
    uint8_t _pin;
    unsigned long _interval;
    unsigned long _last = 0;
    bool _state = false;
};
```

The payoff is you can have *two* LEDs (`Led red(2,300); Led green(4,1000);`) from one piece of code. **This is optional** — plain functions are perfectly fine for everything in this course. Come back to classes when you find yourself copy-pasting a file to make a second copy of something.

---

## 9. Deliverable Checklist

- [x] `sensors.h/.cpp` and `led.h/.cpp` exist in `src/`
- [x] `platformio.ini` defines `SENSOR_PIN` and `LED_PIN` as build flags
- [x] `main.cpp` is short and reads like a description — no pin numbers, no sensor internals
- [x] Three timed tasks run independently, no `delay()` anywhere
- [x] You can explain what a state machine is and why "ask now, read later" beats waiting
- [x] You can explain what a `.h` file is for versus a `.cpp` file

---

## 10. What's Next

In **[module-05-ssc-client](module-05-ssc-client.md)** you will:
- Upload the ready-made Sensorclouds template and set it up from your phone or laptop browser
- Go **online** for the first time
- See your device appear in the Sensorclouds dashboard

