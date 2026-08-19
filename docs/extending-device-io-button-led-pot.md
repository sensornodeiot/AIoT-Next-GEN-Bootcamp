
# Extending Your Device — Potentiometer, Button & LED

**Goal:** Add three more components to your template device and interact with them from the SSC dashboard: a **potentiometer** and a **push button** you can *watch* (they send data up), and an **LED** you can *control* (the dashboard sends a command down).

**Audience:** Completed [module-07-publish-telemetry](module-07-publish-telemetry.md) — you have the `SSCApp` template flashed, configured, and publishing your DS18B20 temperature.

---

## 1. Two kinds of I/O (this is the key idea)

Everything you add is one of two types, and they use two different paths in the template:

| Type | Direction | Where you add it | Example |
|------|-----------|------------------|---------|
| **Input (sensor)** | device → cloud (**telemetry**) | `lib/sensor/sensor.cpp` → `doRead()` with `r.add(...)` | potentiometer, button |
| **Output (actuator)** | cloud → device (**command**) | `lib/config_cmd_handler/` → a new command | LED |

So the **potentiometer and button** are just more `r.add()` lines (like your temperature). The **LED** is different — the dashboard has to send it a command, so you add a small command handler.

---

## 2. Pin plan

| Component | Pin | Notes |
|-----------|-----|-------|
| DS18B20 (already there) | GPIO 32 | + 4.7 kΩ pull-up |
| **Potentiometer** wiper | GPIO 34 | ADC input-only pin — perfect for analog in; ends to 3.3 V and GND |
| **Push button** | GPIO 4 | `INPUT_PULLUP`, other leg to GND. **Do not use GPIO 0** — that's the BOOT button the template uses to open the config portal |
| **LED** (+ 220 Ω) | GPIO 2 | anode → GPIO 2 via 220 Ω → LED → GND |

> **Why GPIO 34 for the pot:** GPIO 32–39 are ADC1 pins and work while Wi-Fi is on. GPIO 34 is input-only (no pull-ups, no output) — ideal for a potentiometer.

---

## 3. Part A — Potentiometer (input → dashboard value)

**Wire:** wiper → GPIO 34; the two outer legs → 3.3 V and GND.

**Edit `doRead()`** in `lib/sensor/sensor.cpp` — add one line:

```cpp
    // potentiometer as a 0–100 % value
    r.add("pot", analogRead(34) / 4095.0f * 100.0f);
```

That's it. The template publishes it alongside temperature every interval. On the dashboard, bind a **gauge** or **value card** to the `pot` field.

---

## 4. Part B — Push button (input → dashboard indicator)

**Wire:** one leg → GPIO 4, the other leg → GND.

**In `sensorBegin()`** add the pin mode (once):
```cpp
    pinMode(4, INPUT_PULLUP);   // button: pressed = LOW
```

**In `doRead()`** add the reading:
```cpp
    // button state: 1 = pressed, 0 = released
    r.add("button", digitalRead(4) == LOW ? 1.0f : 0.0f);
```

On the dashboard, bind a **status indicator** or value card to the `button` field.

> **Note — timing.** Telemetry is sent every publish interval (e.g. 10 s). So the button reports its state *at each publish*, not the instant you press it. A quick tap between publishes may be missed. That's fine for "is it held down?" monitoring; instant reaction would need event-based publishing (advanced).

---

## 5. Part C — LED (output ← dashboard command)

The LED is controlled **from** the dashboard, so it's a **command**, not telemetry. You add one command, `config:set_led`, to the template's command handler — the same place `config:set_interval` lives.

**Wire:** GPIO 2 → 220 Ω → LED anode; LED cathode → GND.

### 5.1 — Declare the handler
In **`lib/config_cmd_handler/config_cmd_handler.h`**, add one line in the `private:` section (next to `handleSetInterval`):
```cpp
    void handleSetLed(JsonDocument& doc);
```

### 5.2 — Set the pin up
In **`lib/config_cmd_handler/config_cmd_handler.cpp`**, at the top add:
```cpp
#define LED_CMD_PIN 2
```
and in `begin()` add:
```cpp
    pinMode(LED_CMD_PIN, OUTPUT);
    digitalWrite(LED_CMD_PIN, LOW);   // start off
```

### 5.3 — Route the command
In `handleMessage()`, add another `else if` next to the existing ones:
```cpp
    } else if (strcmp(command, "config:set_led") == 0) {
        handleSetLed(doc);
```

### 5.4 — Handle it
Add this new function (mirrors `handleSetInterval`):
```cpp
void ConfigCmdHandler::handleSetLed(JsonDocument& doc) {
    const char* cmdId = doc["commandId"] | "unknown";
    JsonVariant param = doc["parameters"]["state"];

    if (param.isNull() || !param.is<int>()) {
        sendAck(cmdId, "config:set_led", "failed", "Missing or invalid state (0 or 1)");
        return;
    }

    int state = param.as<int>();
    digitalWrite(LED_CMD_PIN, state ? HIGH : LOW);
    Serial.printf("[CfgCmd] LED -> %d\n", state);
    sendAck(cmdId, "config:set_led", "ok");
}
```

### 5.5 — Control it from SSC
In the dashboard's **Send Command** dialog (or a command button widget):

| Field | Value |
|-------|-------|
| Command | `config:set_led` |
| Parameters | `{ "state": 1 }` to turn on, `{ "state": 0 }` to turn off |

👁 The LED changes, and an ACK `{ "command": "config:set_led", "status": "ok" }` appears in the event log.

> **Why `config:` and not `led:`?** Commands route by prefix — `config:` already goes to this handler, so reusing it means zero changes to the orchestrator (`ssc_app.cpp`). If you'd rather have a clean `led:` prefix, add an `else if (strncmp(command, "led:", 4) == 0)` branch in `SSCApp::dispatchCommand()` — but `config:set_led` is the least-effort path.

---

## 6. Your `doRead()` with everything

For reference, `doRead()` now looks like this (temperature + pot + button):

```cpp
static SensorReading doRead() {
    SensorReading r;

    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    if (t != DEVICE_DISCONNECTED_C) r.add("temperature", t);   // note: spelled correctly!

    r.add("pot",    analogRead(34) / 4095.0f * 100.0f);
    r.add("button", digitalRead(4) == LOW ? 1.0f : 0.0f);

    r.valid = (r.count > 0);
    return r;
}
```

*(Earlier your file had `r.add("temperautre", ...)` — a typo. The field name becomes the dashboard field **and must match your decoder**, so use `"temperature"`.)*

---

## 7. ⚠ Update the platform decoder to match

Every `r.add()` adds another value to the payload, in order. The template sends: **2-byte nonce, then one IEEE-754 float per value.** Your platform decoder must list the **same names in the same order**. After adding pot and button, the `names` list becomes:

```javascript
const names = ["temperature", "pot", "button"];   // exact order of your r.add() calls
```

*(This is the template-format decoder from the previous discussion. If your device still uses the old temp/humidity `int16 ÷ 10` decoder, it won't understand this payload — reconcile the format first.)*

The LED command needs **no** decoder change — commands aren't decoded telemetry.

---

## 8. Build, upload, verify

```
pio run -e esp32dev -t upload
pio device monitor -e esp32dev
```

- 👁 Turn the pot → the `pot` value on the dashboard sweeps 0–100 %.
- 👁 Hold the button → `button` shows `1` at the next publish; release → `0`.
- 👁 Send `config:set_led` `{ "state": 1 }` → the LED lights and you get an `ok` ACK.

---

## 9. Checklist

- [ ] Potentiometer on GPIO 34; `pot` value updates on the dashboard
- [ ] Button on GPIO 4 (`INPUT_PULLUP`); `button` reports 1/0
- [ ] LED on GPIO 2; `config:set_led` turns it on/off from the dashboard with an ACK
- [ ] Decoder `names` list updated to `["temperature", "pot", "button"]`
- [ ] You can explain the difference between an **input** (telemetry, `doRead`) and an **output** (command, `config_cmd_handler`)
