# Appendix — Low-Level SSCClient Step-by-Step

> **Optional, for after the course.** The main course does Modules 5–10 with the ready-made **`sensorclouds-template`** (configured in a browser, no credentials in code): follow [module-05-ssc-client](module-05-ssc-client.md) onward. This guide is the hands-on companion to [appendix-under-the-hood-ssc-client](appendix-under-the-hood-ssc-client.md): it rebuilds the same features **by hand** with the low-level `SSCClient`, using the projects in the repo's **`appendix-low-level/`** folder. Here credentials *do* go in `platformio.ini` build flags, because there is no setup portal.

**What this is.** The appendix explains *why*. This guide is the **do-exactly-this** companion for when you're actually at the keyboard — every file to open, every value to type, every command to run, and exactly what you should see. It matches the real `AIoT-Next-GEN-Bootcamp` repo (which builds clean), so there is nothing to guess.

**Conventions used below**
- `▶ DO:` an action to perform. `👁 SEE:` what should appear. `⚠ IF:` a fix for a common failure.
- The PlatformIO environment in this repo is **`ssn32`** (not `esp32dev`). All build commands use `-e ssn32`.
- Each `appendix-low-level/module-XX-…/` folder is its **own** PlatformIO project — open that folder directly.
- The `ssc_client` library is **already inside** each module's `lib/ssc_client/`. You never download or copy it.

---

## Before you start (once)

1. **Modules 1–4 done** — you can build, upload, and read Serial at 115200.
2. **A Sensorclouds account** with **one registered device**. From that device you need **7 values** (Step 5.2 shows exactly where each goes). If you don't have a device yet, that's set up in the Sensorclouds platform course / by your instructor.
3. **A 2.4 GHz WiFi network** (the ESP32 cannot use 5 GHz).

---

# Module 5 — Get the device ONLINE

**End state:** you flash the board, and within ~10 seconds it shows **online** in Sensorclouds. No MQTT code written by you.

## 5.1 — Open the project

▶ DO: In VS Code → **File → Open Folder…** → select **`AIoT-Next-GEN-Bootcamp/appendix-low-level/module-05-ssc-client`** (the module folder itself, *not* the parent repo).

👁 SEE: the bottom blue PlatformIO bar (✓ → 🔌 …) after a few seconds. In the Explorer you should see:
```
module-05-ssc-client/
├── platformio.ini          ← config + your credentials
├── src/main.cpp            ← all your code
└── lib/ssc_client/         ← the library (already here — don't touch)
    ├── ssc_client.h
    └── ssc_client.cpp
```
> If `lib/ssc_client/` only has a `NOTE.md` and no `.h/.cpp`, the library wasn't provided — tell your instructor. In this repo it's already populated.

## 5.2 — Get your 7 credentials and put them in `platformio.ini`

▶ DO: Open **`platformio.ini`**. Fill the build flags with your real values. Here is the **complete file** — replace only the `YOUR_...` parts:

```ini
[env:ssn32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient
build_flags =
    '-DWIFI_SSID="YourNetwork"'
    '-DWIFI_PASSWORD="YourWiFiPassword"'
    -DSSC_BROKER=\"YOUR_SSC_BROKER_HOST\"
    -DSSC_PORT=1883
    -DDEVICE_ID=\"YOUR_DEVICE_ID\"
    -DDEVICE_TOKEN=\"YOUR_DEVICE_TOKEN\"
    -DDEVICE_PASSWORD=\"YOUR_DEVICE_PASSWORD\"
    -DTENANT_ID=\"YOUR_TENANT_ID\"
    -DAPP_ID=\"YOUR_APP_ID\"
    -DFIRMWARE_VERSION=\"v1.0.0\"
```

Where each value comes from:

| Build flag | What it is | Where to find it |
|-----------|-----------|------------------|
| `WIFI_SSID` | your 2.4 GHz WiFi name | your router |
| `WIFI_PASSWORD` | your WiFi password | your router |
| `SSC_BROKER` | the MQTT broker host | your device's **MQTT** page in Sensorclouds (e.g. `mqtt.sensorclouds.app`) |
| `SSC_PORT` | MQTT port | `1883` (plain MQTT) — leave as-is |
| `DEVICE_ID` | the MQTT client ID | device **Settings** page |
| `DEVICE_TOKEN` | the MQTT **username** | device **Settings** page |
| `DEVICE_PASSWORD` | the MQTT **password** | device **Settings** page (generated) |
| `TENANT_ID` | your tenant | your account / tenant |
| `APP_ID` | the application this device is in | the application page |
| `FIRMWARE_VERSION` | version you report | you choose — `v1.0.0` is fine |

> **Quoting matters.** Text values use one of two forms already shown: `'-DNAME="value"'` (single-quoted whole flag) **or** `-DNAME=\"value\"` (escaped quotes). Either works; keep the quotes. A missing quote is the #1 build error here.

> **Never commit real credentials.** Fill them locally; don't push real values back to the shared repo.

## 5.3 — Read `src/main.cpp` (this is the whole program)

▶ DO: Open **`src/main.cpp`**. It should already look like this (the complete, working file):

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include "ssc_client.h"

WiFiClient wifiClient;              // (1) the network "pipe"
SSCClient  sscClient(wifiClient);   // (2) hand the pipe to the library

void setup() {
    Serial.begin(115200);

    // (3) Connect WiFi yourself — the library does NOT do this
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[wifi] connecting");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());

    // (4) Fill the 7-field config from your build flags
    SSCConfig config = {
        .host          = SSC_BROKER,
        .port          = 1883,
        .clientId      = DEVICE_ID,
        .username      = DEVICE_TOKEN,
        .password      = DEVICE_PASSWORD,
        .tenantId      = TENANT_ID,
        .applicationId = APP_ID
    };
    sscClient.begin(config);          // (5) go online: connect + birth + subscribe

    Serial.println("Module 5 ready.");
}

void loop() {
    sscClient.update();   // (6) MUST run every loop — this alone keeps you online
}
```

What each numbered part does:
1. **`WiFiClient wifiClient;`** — Arduino's TCP "pipe". Global, so it lives for the whole program.
2. **`SSCClient sscClient(wifiClient);`** — the library, handed the pipe. Global.
3. **WiFi connect** — *your* job, in `setup()`. Blocking-until-connected is fine here (nothing else to do yet).
4. **`SSCConfig`** — the labelled box of settings, filled from the build flags.
5. **`begin(config)`** — the library builds the topics, arms the LWT, connects to the broker, publishes the **birth** message (turns you green), and subscribes to the command topic.
6. **`update()`** — services MQTT and sends the automatic keep-alive. **There is no `publishHeartbeat()`** — calling `update()` every loop is the entire "stay online" story.

## 5.4 — Build

▶ DO: Terminal in the module folder, or the PlatformIO ✓ button:
```
pio run -e ssn32
```
👁 SEE: it ends with
```
========================= [SUCCESS] Took NN seconds =========================
```
⚠ IF: `error: '...' was not declared` on a `DEVICE_*`/`SSC_*` name → a build flag is misspelled or missing a quote in `platformio.ini`.

## 5.5 — Upload

▶ DO: Plug the SSN32 in via USB, then:
```
pio run -e ssn32 -t upload
```
(or press **Ctrl+Alt+U**).

⚠ IF: `could not open port` → close any other Serial monitor / Arduino IDE holding the port; check the board is detected (Device Manager → Ports).

## 5.6 — Watch it connect

▶ DO: Open the Serial Monitor:
```
pio device monitor -e ssn32
```
(or **Ctrl+Alt+S**), 115200 baud. Press the board's **RST** button.

👁 SEE: roughly this sequence (topics abbreviated):
```
[wifi] connecting....
[wifi] connected — IP: 192.168.1.42
[SSC] Broker: mqtt.sensorclouds.app:1883
[SSC] Telemetry: tenant/.../app/.../device/.../data
[SSC] Status:    tenant/.../device/.../status
[SSC] Cmd:       tenant/.../device/.../cmd
[SSC] Ack:       tenant/.../device/.../ack
[SSC] Connecting...
[SSC] Connected
[SSC] Subscribed: tenant/.../device/.../cmd
[SSC] Birth message sent
Module 5 ready.
```

## 5.7 — Confirm online

▶ DO: Open Sensorclouds → your device.
👁 SEE: the device flips to **ONLINE** within ~10 s, and a birth event `{"state":"online",...}` appears in its event log.

## 5.8 — Troubleshooting (read the Serial line where it stops)

| Serial stops at… | Meaning | Fix |
|---|---|---|
| `[wifi] connecting....` forever | WiFi not joining | Check `WIFI_SSID`/`WIFI_PASSWORD` (case-sensitive); confirm the network is **2.4 GHz** |
| `[SSC] Connecting...` then `Connection timeout` | can't reach broker | Check `SSC_BROKER` host and `SSC_PORT` 1883 |
| Connects, then `Connection lost` / retries | credentials rejected | Re-copy `DEVICE_TOKEN`, `DEVICE_PASSWORD`, `DEVICE_ID`, `TENANT_ID`, `APP_ID` from the platform |
| Online, then offline after ~1 min | `update()` not running enough | Make sure `sscClient.update()` is in `loop()` with no long `delay()` |

✅ **Module 5 done when:** Serial shows the full sequence and the dashboard shows **online**.

---

# Module 6 — Understand what just happened (no new code)

**Goal:** prove and understand the messaging model. You **reuse the Module 5 firmware** — same build.

## Steps
1. ▶ DO: Keep the Module 5 board running and the dashboard event log open.
2. **Find the birth message.** 👁 SEE: `{"state":"online",...}` in the event log — note its timestamp.
3. **The LWT test (the memorable one).** ▶ DO: pull the USB cable **without** pressing reset. Wait up to ~90 s. 👁 SEE: the device flips to **offline** automatically — that's the broker publishing your Last-Will message. Plug back in → it returns online with a fresh birth.
4. **Keep-alive check.** ▶ DO: leave it connected 10 min with no telemetry. 👁 SEE: it stays **online** — the automatic MQTT keep-alive (serviced by `update()`) is doing its job. *(There is no heartbeat to send.)*
5. **Break a credential on purpose.** ▶ DO: change `DEVICE_TOKEN` to a wrong value, rebuild/upload. 👁 SEE: `[SSC] Connection timeout` / retry loop. Restore the correct token.

✅ **Module 6 done when:** you can narrate power-on → online (boot → WiFi → MQTT connect → LWT armed → birth → subscribed → `update()` keeping the keep-alive), and you've seen the LWT flip to offline.

---

# Module 7 — Publish real temperature

**Goal:** send the DS18B20 reading every 30 s; see it arrive.

## 7.1 — Open `appendix-low-level/module-07-publish-telemetry` and set `platformio.ini`

▶ DO: Add the sensor libraries and the sensor pin to the flags. Complete file:
```ini
[env:ssn32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient
    paulstoffregen/OneWire
    milesburton/DallasTemperature
build_flags =
    '-DWIFI_SSID="YourNetwork"'
    '-DWIFI_PASSWORD="YourWiFiPassword"'
    -DSSC_BROKER=\"YOUR_SSC_BROKER_HOST\"
    -DSSC_PORT=1883
    -DDEVICE_ID=\"YOUR_DEVICE_ID\"
    -DDEVICE_TOKEN=\"YOUR_DEVICE_TOKEN\"
    -DDEVICE_PASSWORD=\"YOUR_DEVICE_PASSWORD\"
    -DTENANT_ID=\"YOUR_TENANT_ID\"
    -DAPP_ID=\"YOUR_APP_ID\"
    -DFIRMWARE_VERSION=\"v1.0.0\"
    -DONE_WIRE_PIN=32
```

## 7.2 — Wire the sensor
▶ DO: DS18B20 → **VCC=3.3V**, **GND=GND**, **DATA=GPIO 32** with a **4.7 kΩ pull-up** between DATA and 3.3V. *(Without the pull-up you get `-127` / not found.)*

## 7.3 — The complete `src/main.cpp`
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ssc_client.h"

WiFiClient wifiClient;
SSCClient  sscClient(wifiClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

const unsigned long PUBLISH_INTERVAL = 30000;   // 30 s
unsigned long lastPublish = 0;

// int16 (x100), big-endian, two hex bytes.  28.43 C -> 2843 -> 0x0B1B -> "0B1B"
String encodeTemperature(float tempC) {
    int16_t scaled = (int16_t)(tempC * 100.0f);
    char buf[8];
    snprintf(buf, sizeof(buf), "%02X%02X", (scaled >> 8) & 0xFF, scaled & 0xFF);
    return String(buf);
}

void setup() {
    Serial.begin(115200);
    sensors.begin();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[wifi] connecting");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());

    SSCConfig config = {
        .host = SSC_BROKER, .port = 1883,
        .clientId = DEVICE_ID, .username = DEVICE_TOKEN, .password = DEVICE_PASSWORD,
        .tenantId = TENANT_ID, .applicationId = APP_ID
    };
    sscClient.begin(config);
    Serial.println("Module 7 ready.");
}

void loop() {
    sscClient.update();                 // keep online
    sensors.requestTemperatures();      // (kept simple here; Module 4 shows the non-blocking version)

    unsigned long now = millis();
    if (now - lastPublish >= PUBLISH_INTERVAL) {
        lastPublish = now;
        float t = sensors.getTempCByIndex(0);
        if (t == DEVICE_DISCONNECTED_C) {
            Serial.println("[warn] DS18B20 not found — skipping publish");
        } else {
            String payload = encodeTemperature(t);
            if (sscClient.publishTelemetry(payload.c_str()))
                Serial.printf("[%lums] TX %s (%.2f C)\n", now, payload.c_str(), t);
            else
                Serial.printf("[%lums] publish failed — not connected\n", now);
        }
    }
}
```

## 7.4 — Tell the platform how to decode the hex
The device sends `0B1B`; the platform must be told that means "int16 ÷ 100". ▶ DO: set up the **payload decoding** for this device in Sensorclouds (a codec / decode rule). *(This is platform-side — covered in the Sensorclouds platform course. Ask your instructor which decode is configured for your device.)*

## 7.5 — Build, upload, verify
```
pio run -e ssn32 -t upload
pio device monitor -e ssn32
```
👁 SEE (every 30 s): `[12345ms] TX 0B1B (28.43 C)`. In the dashboard **Live Data**, the temperature value updates. **Hold the sensor** → the next value rises.

⚠ IF: `TX` in Serial but nothing in dashboard → the decode/codec isn't set (7.4). `-127` → check the 4.7 kΩ pull-up.

✅ **Module 7 done when:** `TX <hex>` prints every 30 s and the value shows in Live Data.

---

# Module 8 — Dashboards & widgets (platform-side, no firmware)

There is **no firmware** for this module. Building dashboards, gauges, and charts is done entirely in the Sensorclouds platform course. On the firmware side you've already confirmed your data arrives (Module 7's Live Data) — that's all the device needs to do.

✅ **Module 8 done when:** (in the platform course) you've bound a chart/value widget to your device's temperature field.

---

# Module 9 — Receive a command from the cloud

**Goal:** the dashboard sends a value *down*; the device applies it live and ACKs.

## 9.1 — `platformio.ini`: add ArduinoJson
▶ DO: add to `lib_deps` (keep everything from Module 7):
```
    bblanchon/ArduinoJson
```

## 9.2 — The complete `src/main.cpp`
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ssc_client.h"

WiFiClient wifiClient;
SSCClient  sscClient(wifiClient);

// runtime-changeable settings (plain variables — that's what makes "no reboot" work)
unsigned long sensorInterval = 30000;
float         tempOffset     = 0.0f;
unsigned long lastPublish = 0;

// The command arrives as RAW BYTES. Register this with setCommandCallback BEFORE begin().
void onCommand(const char* topic, uint8_t* payload, unsigned int length) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, length)) { Serial.println("[cmd] bad JSON"); return; }

    if (doc["sensorReadInterval"].is<int>()) {
        sensorInterval = constrain(doc["sensorReadInterval"].as<unsigned long>(), 100UL, 60000UL);
        Serial.printf("[cmd] interval -> %lu\n", sensorInterval);
    }
    if (doc["tempOffset"].is<float>()) {
        tempOffset = doc["tempOffset"].as<float>();
        Serial.printf("[cmd] offset -> %.2f\n", tempOffset);
    }

    // Build your OWN ack and publish it (the library does not invent one)
    const char* cmdId = doc["commandId"];
    if (cmdId) {
        JsonDocument ack; ack["commandId"] = cmdId; ack["status"] = "ok";
        char buf[128]; serializeJson(ack, buf);
        sscClient.publishAck(buf);
        Serial.printf("[cmd] ACK %s\n", buf);
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[wifi] connecting");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());

    sscClient.setCommandCallback(onCommand);     // BEFORE begin()
    SSCConfig config = {
        .host = SSC_BROKER, .port = 1883,
        .clientId = DEVICE_ID, .username = DEVICE_TOKEN, .password = DEVICE_PASSWORD,
        .tenantId = TENANT_ID, .applicationId = APP_ID
    };
    sscClient.begin(config);
    Serial.println("Module 9 ready.");
}

void loop() {
    sscClient.update();
    unsigned long now = millis();
    if (now - lastPublish >= sensorInterval) {   // uses the CHANGEABLE variable
        lastPublish = now;
        // (publish telemetry here as in Module 7, applying + tempOffset)
    }
}
```

## 9.3 — Test it
▶ DO: build/upload/monitor. From the dashboard, send `{ "sensorReadInterval": 10000 }` (or the command button). 👁 SEE Serial: `[cmd] interval -> 10000` then `[cmd] ACK {...}`, and the ACK appears in the platform event log. Send `{ "tempOffset": 5.0 }` → values shift +5. Send junk `{ "commandId":"t1","foo":9 }` → no crash, ACK still sent.

✅ **Module 9 done when:** a dashboard command changes behaviour live, the ACK shows, and the device never reboots.

---

# Module 10 — OTA firmware update (bonus)

**Goal:** push new firmware over the air; the device installs it and reboots into it.

## 10.1 — `platformio.ini`: partitions + version
▶ DO: to the Module 9 config, add the dual-slot partition table (keep all flags):
```ini
board_build.partitions = min_spiffs.csv
```
`FIRMWARE_VERSION` is already a build flag — this is the version the device reports.

## 10.2 — Wire the OTA handler into `main.cpp`
The `ota_handler` library is already in `lib/ota_handler/`. Three hook-ups (added to your Module 9 code):
```cpp
#include "ota_handler.h"
OtaHandler otaHandler;

// in onCommand(): route OTA commands to the handler
//   char buf[256]; unsigned int n = length<255?length:255; memcpy(buf,payload,n); buf[n]='\0';
//   if (strstr(buf, "\"ota:") != nullptr) { otaHandler.handleMessage(topic, payload, length); return; }

// in setup(), AFTER sscClient.begin(config):
otaHandler.begin(&sscClient.getMqttClient(), sscClient.getAckTopic());
otaHandler.checkPendingAck();    // confirms a prior OTA succeeded after reboot

// in loop():
otaHandler.update();             // drives download/verify/install, non-blocking
```

## 10.3 — Do an update
1. ▶ DO: set `FIRMWARE_VERSION="v1.0.0"`, upload over USB. 👁 SEE the portal shows `v1.0.0`.
2. ▶ DO: change to `FIRMWARE_VERSION="v1.0.1"`, then **build only**: `pio run -e ssn32`. The image is at `.pio/build/ssn32/firmware.bin`.
3. ▶ DO: in the Sensorclouds portal → your device → OTA → upload `firmware.bin`, target `v1.0.1`, deploy.
4. 👁 SEE: Serial shows download → verify → reboot; the birth message then reports `v1.0.1`.
5. **Safety test:** repeat but pull USB power mid-download → the device boots the **old** `v1.0.0`, unharmed (the running slot was never touched).

✅ **Module 10 done when:** the portal shows the device updated to `v1.0.1` after an OTA, and the rollback test leaves it working.

---

## Quick command reference

| Action | Command |
|--------|---------|
| Build | `pio run -e ssn32` |
| Upload | `pio run -e ssn32 -t upload` |
| Serial monitor | `pio device monitor -e ssn32` (115200) |
| Build + upload + monitor | `pio run -e ssn32 -t upload -t monitor` |
| Clean | `pio run -e ssn32 -t clean` |

## Related

- Concept lessons: [module-05-ssc-client](module-05-ssc-client.md) · [module-06-mqtt-ssc-registration](module-06-mqtt-ssc-registration.md) · [module-07-publish-telemetry](module-07-publish-telemetry.md) · [module-09-receive-commands](module-09-receive-commands.md) · [module-10-ota](module-10-ota.md)
- Project: iot-learning-module
