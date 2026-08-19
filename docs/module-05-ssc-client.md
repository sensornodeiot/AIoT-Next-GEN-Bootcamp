
# Module 5 — Connect with the SSC Template

**Goal:** Get your SSN32 **online** in the SSC (Sensorclouds) platform and keep it there — by flashing a ready-made **template** and configuring it in your browser. You write almost no code.

**Audience:** Completed [module-04-firmware-patterns](module-04-firmware-patterns.md). This is your first "online" module.

**Estimated time:** 1.5–2 hours.

**Deliverable:** Your device connects on power-up, publishes a reading every few seconds, and **stays online** in the SSC dashboard — all configured through a web wizard, no credentials in code.

> **Instructor note:** the payoff here is a board that shows up green *and stays green*. That "stays" matters: a device that only connects but never publishes gets marked offline after a few minutes — which is exactly why we use the template (it publishes on a timer). Have the SSC dashboard projected. The #1 snag is joining the config Wi-Fi hotspot; Section 6 is the triage guide.

---

## 1. The idea: configure, don't code

Up to now you wrote every line. Connecting a device to a cloud platform properly means a *lot* of plumbing: Wi-Fi with auto-reconnect, an MQTT client, a birth/offline announcement, publishing telemetry on a schedule, receiving commands, and over-the-air updates. Writing all of that by hand is error-prone.

So instead you use the **SSC template** — a ready-made firmware (called `SSCApp`) that already contains all of it. Your job is not to *write* it, but to:

1. **Flash** the template to your board.
2. **Configure** it (Wi-Fi + your SSC credentials) in a web wizard.
3. (Later, Module 7) tell it **which sensor** you have.

That's the real way devices are built — you stand on a tested library, not a blank file.

> **Curious what's inside?** The appendix [appendix-under-the-hood-ssc-client](appendix-under-the-hood-ssc-client.md) shows how to drive the low-level `SSCClient` by hand. Optional — come back to it after the course.

---

## 2. What the whole program looks like

Open the template's `src/main.cpp`. This is the **entire** sketch:

```cpp
#include <Arduino.h>
#include "ssc_app.h"

SSCApp app;

void setup() {
    Serial.begin(115200);
    app.begin();     // load saved config; if none, start the config portal
}

void loop() {
    app.update();    // run Wi-Fi + MQTT + publish telemetry + handle commands + OTA
    delay(10);
}
```

Everything — Wi-Fi, MQTT, the periodic telemetry that keeps you online, remote commands, OTA — lives inside `app`. You don't edit this file.

---

## 3. Where things live

| Where | What it's for | Do you touch it? |
|-------|---------------|------------------|
| `src/main.cpp` | The 12-line sketch above (`app.begin()` / `app.update()`) | **No** |
| the config **portal** (a web page on the device) | Where you enter Wi-Fi + SSC credentials — saved to the board's flash | **Yes — this is your "config"** |
| `lib/sensor/sensor.cpp` | Which sensor to read and what to publish | Not yet — you edit this in **Module 7** |
| `platformio.ini` `FIRMWARE_VERSION` | the version the device reports | only when you do OTA (Module 10) |

**No credentials go in any file.** They're typed into the portal and stored on the device — much safer than build flags.

---

## 4. Flash the template

▶ In VS Code, **File → Open Folder →** the **`sensorclouds-template`** project folder. Plug in the SSN32, then:

```
pio run -e esp32dev -t upload
```

👁 Ends with `[SUCCESS]` and uploads. Then open the Serial Monitor:
```
pio device monitor -e esp32dev
```
(115200 baud). Press **RST**.

---

## 5. Configure it in the browser (the first-boot portal)

The very first boot has **no saved config**, so the device starts a **configuration portal**:

👁 Serial shows something like:
```
[Portal] Starting configuration portal
[Portal] AP: SSN32-Config  IP: 192.168.4.1
[Portal] Ready at http://192.168.4.1
```

▶ **Step 5a — Join the device's Wi-Fi.** On your phone or laptop, open Wi-Fi settings and connect to:
- **Network:** `SSN32-Config`
- **Password:** `ssn32config`

▶ **Step 5b — Open the portal.** In a browser go to **http://192.168.4.1**. You'll see a 4-step wizard: **Status → Wi-Fi → MQTT → OTA**.

▶ **Step 5c — Wi-Fi step.** Enter your **2.4 GHz** network name and password. *(The ESP32 can't use 5 GHz.)*

▶ **Step 5d — MQTT step.** Enter the values from your SSC device page:

| Portal field | What it is | From SSC |
|--------------|-----------|----------|
| Broker host | the MQTT broker | your device's **MQTT** page (e.g. `mqtt.sensorclouds.app`) |
| Port | `1883` | fixed |
| Client ID | must match your **Device ID** | device **Settings** |
| Username | your **Device Token** | device **Settings** |
| Password | the generated **password** | device **Settings** |
| Tenant ID | your tenant | account/tenant |
| Application ID | the app this device is in | application page |
| Publish interval | how often it sends (ms) | e.g. `10000` for 10 s |

▶ **Step 5e — Save.** The device stores everything to its flash and reboots.

> **To change any of this later:** hold the **BOOT** button (GPIO0) for **2 seconds** at power-on — the portal starts again.

---

## 6. Watch it connect and stay online

After it reboots with valid config, 👁 Serial shows roughly:
```
[wifi] connecting... connected — IP: 192.168.1.42
[SSC] Broker: mqtt.sensorclouds.app:1883
[SSC] Connecting...
[SSC] Connected
[SSC] Birth message sent
[Sensor] ...
[12000ms] TX <hex>        <- a reading every ~10 s
[22000ms] TX <hex>
```

▶ Open SSC → your device. 👁 It shows **ONLINE**, and a `temperature` value updates every ~10 seconds. Leave it 10 minutes — it **stays online**, because it keeps publishing.

> **Why it stays online now** (when Module-4-style connect-only code did not): SSC marks a device offline after a few minutes of *no publishes*. The template publishes every interval, so its "last seen" never goes stale. *(For now the value is a placeholder ~25 °C — Module 7 makes it your real DS18B20.)*

---

## 7. Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| No `SSN32-Config` hotspot appears | device already has a saved config | hold **BOOT** 2 s at power-on to force the portal |
| Portal page won't load | wrong address | it's exactly `http://192.168.4.1`, and you must be joined to `SSN32-Config` |
| Saved config but `[wifi] connecting…` forever | wrong Wi-Fi or 5 GHz | re-enter Wi-Fi in the portal; use a 2.4 GHz network |
| Wi-Fi OK, `[SSC] Connection timeout` | wrong broker host/port | re-check the MQTT step (host, port 1883) |
| Connects then drops repeatedly | wrong token/password/IDs | re-copy Client ID, Token, password, tenant, app in the portal |
| Online then offline after minutes | it isn't publishing | check Serial for `TX` lines; if the sensor read fails nothing is sent (Module 7 fixes the real sensor) |

---

## 8. Deliverable Checklist

- [ ] Template flashed to the SSN32
- [ ] Joined `SSN32-Config` and completed the Wi-Fi + MQTT wizard
- [ ] Serial shows `[SSC] Connected` → `Birth message sent` → periodic `TX`
- [ ] SSC dashboard shows the device **online** with a value updating
- [ ] It's still online after 10 minutes
- [ ] You can explain: why configuring beats hand-coding here, and why periodic publishing (not just connecting) is what keeps a device online

---

## 9. What's Next

In **[module-06-mqtt-ssc-registration](module-06-mqtt-ssc-registration.md)** you'll understand the messaging model underneath (broker, topics, birth, LWT) so you can read what the template is doing and debug it. Then **[module-07-publish-telemetry](module-07-publish-telemetry.md)** replaces the placeholder value with your real DS18B20.
