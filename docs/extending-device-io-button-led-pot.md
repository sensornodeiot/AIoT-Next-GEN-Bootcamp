# Your Device's I/O — Potentiometer, Button & LED

**What this is:** a one-page reference for the inputs and outputs built into the `sensorclouds-template`: where each part is wired, where its code lives, and how you see or control it from SSC. All of it is **already in the template**. You don't add any code to use it.

**Used in:** [module-07-publish-telemetry](module-07-publish-telemetry.md) (wiring + telemetry), [module-08-ssc-dashboard](module-08-ssc-dashboard.md) (widgets), [module-09-receive-commands](module-09-receive-commands.md) (LED command).

---

## 1. Two kinds of I/O (the key idea)

Every part is one of two types, and they travel in opposite directions:

| Type | Direction | How it travels | Where the code lives | Parts |
|------|-----------|----------------|----------------------|-------|
| **Input** (sensor) | device → cloud | **telemetry**, every publish interval | `lib/sensor/sensor.cpp` → `doRead()` with `r.add(...)` | DS18B20, potentiometer, button |
| **Output** (actuator) | cloud → device | **command**, the moment it's sent | `lib/config_cmd_handler/` → `config:set_led` | LED |

The LED is both: you *switch* it with a command, and the template *reports* its state (`led`) in telemetry so the dashboard can show it.

---

## 2. Pin plan

| Part | Pin | Wiring |
|------|-----|--------|
| DS18B20 | GPIO 32 | VCC → 3.3 V, GND → GND, DATA → GPIO 32 + 4.7 kΩ (or 5.1 kΩ) from DATA to 3.3 V |
| Potentiometer (or LDR) | GPIO 34 | outer legs → 3.3 V and GND, middle leg → GPIO 34 |
| Push button | GPIO 4 | plain button: GPIO 4 ↔ GND. Kit 3-pin module: VCC → 3.3 V, GND → GND, OUT → GPIO 4 |
| LED | GPIO 2 | on-board LED (always there). Optional external LED: GPIO 2 → 220 Ω → LED → GND |

Why these pins:
- **GPIO 34** is an ADC1 pin, so it keeps working while Wi-Fi is on (ADC2 pins don't). It's input-only, which is all a potentiometer needs.
- **GPIO 4, not GPIO 0, for the button.** GPIO 0 is the BOOT button, which the template uses to open its setup portal.
- **GPIO 2** is wired to the SSN32's on-board LED, so the LED command works even before you wire anything.

---

## 3. What each part sends or does

| Name on SSC | Part | Value | Notes |
|-------------|------|-------|-------|
| `temperature` | DS18B20 | °C, 2 decimals | Left out when no sensor is found (never a fake number) |
| `pot` | potentiometer / LDR | 0–100 % | Average of 16 readings, so it doesn't jitter |
| `button` | push button | `1` / `0` | `1` if held now **or pressed at any moment since the last publish**, so a quick tap is never missed. Works with either button type: the template checks at power-up which way the pin rests |
| `led` | LED | `1` / `0` | The current LED state |

**LED command:** `config:set_led` with `{ "state": 1 }` (on) or `{ "state": 0 }` (off). Any other value gets a `failed` ACK. The state isn't saved: the LED starts off after every restart.

---

## 4. The decoder must match

The template sends the four values in a fixed order: `temperature`, `pot`, `button`, `led`. The Sensorclouds decoder reads them back in the same order. The complete script, with test payloads, is in the course repo at **`sensorclouds-template/docs/platform/decoder.md`**.

If you add a value of your own, add it **at the end** of both lists, the `r.add()` calls in `doRead()` and the decoder's `NAMES` (see [module-07-publish-telemetry](module-07-publish-telemetry.md) Section 6).

---

## 5. Quick check

| Do this | See this |
|---------|----------|
| Turn the potentiometer | `pot` follows at the next publish (0 to 100) |
| Tap the button once | `button` = `1` at the next publish, then `0` |
| Send `config:set_led` `{ "state": 1 }` | LED lights at once; ACK `ok`; `led` = `1` at the next publish |
| Unplug the DS18B20 | `temperature` disappears from the next publish; the device **stays online** |

