# Appendix — Under the Hood: the low-level `SSCClient`

> **Optional / advanced.** The main course ([module-05-ssc-client](module-05-ssc-client.md) onward) uses the ready-made **template** (`SSCApp`) — you flash it and configure it in a browser, writing almost no code. This appendix shows what's happening *inside*: how to drive the low-level `SSCClient` class by hand — connect WiFi yourself, build `SSCConfig`, call `begin()`/`update()`. Read it only if you're curious how the template works, or you want to build a device from the building blocks instead of the orchestrator. For the full hand-written code of every connectivity module in this style, see [modules-5-10-step-by-step](modules-5-10-step-by-step.md).
>
> **Note:** driving `SSCClient` directly means *you* are responsible for publishing regularly — a device that only connects and never publishes is marked **offline** by the platform after 90 seconds without a message. The template solves this with its built-in periodic telemetry scheduler.

**Goal:** Get your SSN32 onto the internet and connected to the SSC platform — so it shows up **online** in the dashboard. You will connect to WiFi yourself, then let the SSC-Client library handle the messaging part for you.

**Audience:** Completed [module-04-firmware-patterns](module-04-firmware-patterns.md). This is your first "online" module — everything before this ran offline on the bench.

**Estimated time:** 2–3 hours (reading + lab)

**Deliverable:** Your device connects on power-up and appears **online** in the SSC dashboard, with no MQTT code written by you.

> **Instructor note:** This is the module where "IoT" finally becomes real for the class — the moment their board shows up green in the dashboard is the payoff of Modules 0–4. Have the SSC dashboard projected so everyone sees devices appear as students flash. Expect WiFi typos (SSID/password) to be the #1 problem — Section 8 is your triage guide.

---

## 1. The Big Picture — What Are We Actually Connecting To?

Up to now your board has been talking only to *you*, over the USB cable, through the Serial Monitor. In this module it starts talking to a **server on the internet**.

Three things sit between your sensor and the dashboard:

```
  [ SSN32 ] --WiFi--> [ your router / internet ] --MQTT--> [ SSC platform ] --> [ dashboard ]
   your code            the network path            a message broker         what you see
```

- **WiFi** gets your board onto the network — same as your phone joining WiFi.
- **MQTT** is the *language* devices use to send small messages to a server. (You'll learn MQTT properly in [module-06-mqtt-ssc-registration](module-06-mqtt-ssc-registration.md). For today, think of it as "texting short messages to a server.")
- **SSC platform** is that server — it stores your data and draws the dashboard.

> **Why this matters:** Beginners often think "the library does the internet." It doesn't. Knowing *which* piece does *which* job is what lets you fix things when they break — a WiFi problem and an MQTT problem look different and are fixed in different places.

---

## 2. What SSC-Client Is — and What It Is *Not*

**SSC-Client** is a small C++ library that does the fiddly MQTT messaging work for you: connecting to the broker, announcing your device, sending data, and reconnecting if the connection drops.

What it does **not** do, and this is the important part:

| Job | Who does it |
|-----|-------------|
| Join WiFi | **You** (a few lines of code — Section 3) |
| Connect to the MQTT broker | SSC-Client |
| Announce "I'm online" (birth message) | SSC-Client |
| Send sensor data | SSC-Client (Module 7) |
| Reconnect after a drop | SSC-Client |
| Keep the "online" status alive | SSC-Client — as long as **you call `update()` every loop** (Section 7) |

> **Why split WiFi out of the library?** WiFi is a *general* skill your board needs for lots of things (time sync, web requests, OTA updates later). The SSC-Client stays focused on one job — messaging — and works with *whatever* network connection you give it. That separation is why the same library works on WiFi today and could work over a cellular modem tomorrow. See ssc-client for the full architecture.

### Where everything goes (read this before the code)

You edit **two files**. You do **not** create the library.

| Where | What goes there |
|-------|-----------------|
| `platformio.ini` | Your credentials (as build flags) **and the MQTT library dependency** (Sections 8–9). |
| `src/main.cpp` | All the code in this module. The WiFi connect + `SSCConfig` + `begin()` go in **`setup()`**; `update()` goes in **`loop()`**. |
| the SSC-Client **library** | **You don't write it.** Its two files (`ssc_client.h` / `.cpp`) are already in your starter project's `lib/ssc_client/` folder — PlatformIO compiles anything in `lib/` automatically. You just `#include "ssc_client.h"` and use it. |

> **Don't try to assemble the snippets in your head.** The next sections explain each piece one at a time — but the complete, copy-paste-ready `src/main.cpp` is in **the Lab (Section 9)**. Read the parts to understand what each does, then take the whole file.

---

## 3. Step One — Connect to WiFi (You Do This)

**→ This goes in `setup()`** (it runs once, at startup).

The library needs a working internet connection *before* it can do anything. So the very first thing in `setup()` is joining WiFi:

```cpp
#include <WiFi.h>

WiFi.mode(WIFI_STA);              // "station" mode = join an existing network (like a phone)
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

Serial.print("[wifi] connecting");
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());
```

What each line does:
- `WiFi.mode(WIFI_STA)` — tells the ESP32 to *join* a network, not *create* one.
- `WiFi.begin(ssid, password)` — starts connecting in the background.
- The `while` loop waits until `WiFi.status()` reports `WL_CONNECTED`. Getting an **IP address** printed is your proof it worked.

> **Why the `while` loop here, when Module 4 said "never block"?** This runs once in `setup()`, before your real program starts — there's nothing else to do until we're online, so waiting here is fine. The no-blocking rule is about `loop()`, where many tasks share the CPU.

> **Gotcha — 2.4 GHz only.** The ESP32 cannot join 5 GHz WiFi. If your phone connects but the board never does, check that the network is 2.4 GHz.

---

## 4. Step Two — Create the Client and Hand It a Network Connection

**→ These two lines go at the top of the file** (global — above `setup()`, next to `#include "ssc_client.h"`).

The SSC-Client can't reach the network by itself — you give it a `WiFiClient`, which is Arduino's "pipe" for sending bytes over WiFi:

```cpp
#include <WiFi.h>
#include "ssc_client.h"

WiFiClient wifiClient;          // the network "pipe"
SSCClient  sscClient(wifiClient); // hand the pipe to the library
```

> **Why hand it a `WiFiClient` instead of the library making its own?** Because *you* own the network connection. Giving it to the library means the library never has to care *how* you got online — it just uses the pipe you gave it. (In programming this is called "dependency injection"; you don't need the term, just the idea: *give it what it needs, don't let it go find its own*.)

---

## 5. Step Three — Fill In `SSCConfig`

**→ This goes in `setup()`** (just before you call `begin()`).

`SSCConfig` is a plain struct — a labelled box of settings the library needs. Every field comes from your SSC dashboard and is stored as a build flag (Section 8):

```cpp
SSCConfig config = {
    .host          = SSC_BROKER,      // MQTT broker address, e.g. "mqtt.sensorclouds.app"
    .port          = 1883,            // standard MQTT port
    .clientId      = DEVICE_ID,       // your device's ID from the dashboard
    .username      = DEVICE_TOKEN,    // device token — also used to build the message topics
    .password      = DEVICE_PASSWORD, // device password
    .tenantId      = TENANT_ID,       // your organisation on SSC
    .applicationId = APP_ID           // the application this device belongs to
};
```

> **What are those ALL-CAPS names?** `SSC_BROKER`, `DEVICE_ID`, `DEVICE_TOKEN`, and the rest are **build flags** — your real credentials, kept out of the source code. You paste them into `platformio.ini` in **Section 8**. For now, just know each one stands for a value you get from the platform.

| Field                        | What it is                      | Why the library needs it                                                    |
| ---------------------------- | ------------------------------- | --------------------------------------------------------------------------- |
| `host` / `port`              | Where the SSC broker lives      | So it knows what server to dial                                             |
| `clientId`                   | Unique name for this connection | The broker rejects two clients with the same ID                             |
| `username`                   | The device token                | Identifies the device **and** becomes part of the message address (topic)   |
| `password`                   | Device password                 | Proves the device is allowed to connect                                     |
| `tenantId` / `applicationId` | Your org + app on SSC           | Together with the token, these build the exact "channel" your data flows on |

> **Why so many IDs?** SSC organises everything as **tenant → application → device**. Those three values are stitched into the address every message travels on (e.g. `tenant/…/app/…/device/…/data`). Get one wrong and your device connects but its data lands in the wrong place — or nowhere. You'll see these exact topics printed in Serial when `begin()` runs.

---

## 6. Step Four — `begin()`: Go Online

**→ This goes in `setup()`, right after WiFi connects.**

Call `begin()` once, after WiFi is up:

```cpp
sscClient.begin(config);
```

Here's what the library does for you inside `begin()` — and *why* each step exists:

1. **Builds the topic addresses** from your tenant / app / token (`.../data`, `.../status`, `.../cmd`, `.../ack`).
2. **Sets up the connection** — server address, a 60-second keep-alive, and a bigger message buffer.
3. **Arms the "Last Will" (LWT).** *Why:* if your board loses power or crashes, it can't say goodbye. So it tells the broker in advance: "if I vanish, publish `offline` for me." That's why a dead device correctly shows offline instead of a stale "online".
4. **Connects to the broker.**
5. **Publishes the birth message** `{"state":"online","firmwareVersion":"..."}`. *Why:* this is the device announcing itself — it's what turns the dashboard indicator green, and it reports which firmware is running.
6. **Subscribes to the command topic.** *Why:* so the platform can send commands *down* to the device later ([module-09-receive-commands](module-09-receive-commands.md)).

> **Retained messages:** the birth and offline messages are sent "retained", meaning the broker remembers the last one. So even if you open the dashboard *after* the device connected, it still shows the correct status. Without this, status would only appear at the exact moment it changed.

---

## 7. Step Five — `update()` Every Loop (this is what keeps you online)

**→ This goes in `loop()`.**

There is exactly **one** thing the library needs from your `loop()`: call `update()` on every iteration.

```cpp
void loop() {
    sscClient.update();   // <-- must run constantly
    // ... your other tasks ...
}
```

`update()` is the library's housekeeping: it services the MQTT connection, sends the automatic **keep-alive ping** under the hood, notices if the connection dropped, and **reconnects automatically** (waiting a bit longer between each failed try, so it doesn't hammer the server).

> **This is the whole "stay online" story — there is no manual heartbeat.** The broker keeps you marked online as long as the MQTT keep-alive keeps arriving (every ~60 s), and `update()` sends it for you. The birth message marked you online; the LWT marks you offline if you vanish. So you do **not** call any `publishHeartbeat()` — you just make sure `update()` runs every loop.

> **Why "every loop", then?** If you block for tens of seconds without calling `update()`, the keep-alive stops, the broker assumes the device is dead, and cuts you off (that 60-second keep-alive set up in `begin()`). This is the real reason Module 4 drilled the no-`delay()` habit.

---

## 8. Credential Hygiene — Never Hardcode Secrets

**Never type WiFi passwords or device tokens directly into a `.cpp` file.** If that file is ever shared or pushed to GitHub, your secrets go with it.

Instead, put them in `platformio.ini` as build flags. The compiler substitutes them at build time:

```ini
build_flags =
    '-DWIFI_SSID="YourNetwork"'
    '-DWIFI_PASSWORD="YourPassword"'
    '-DSSC_BROKER="mqtt.sensorclouds.app"'
    '-DDEVICE_ID="your-device-id"'
    '-DDEVICE_TOKEN="your-device-token"'
    '-DDEVICE_PASSWORD="your-device-password"'
    '-DTENANT_ID="your-tenant"'
    '-DAPP_ID="your-app"'
```

> **Format note:** for text values, wrap the whole flag in single quotes and the value in double quotes: `'-DWIFI_SSID="MyNetwork"'`. This is fiddly and a common source of build errors — check it carefully.

> **Why this way?** Your code refers to `WIFI_SSID`; the actual secret lives only in `platformio.ini`, which you keep off GitHub. Share your code freely, keep your secrets local.

---

## 9. Lab — Get Your Device Online

### Step 1 — Register a device on SSC
1. Log into the SSC dashboard → open your application
2. Click **Add Device**
3. Copy the **Device ID**, **Token**, **Password**, and note your **Tenant** and **Application** names

### Step 2 — Fill in `platformio.ini`

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
build_flags =
    '-DWIFI_SSID="YourNetwork"'
    '-DWIFI_PASSWORD="YourPassword"'
    '-DSSC_BROKER="mqtt.sensorclouds.app"'
    '-DDEVICE_ID="your-device-id"'
    '-DDEVICE_TOKEN="your-device-token"'
    '-DDEVICE_PASSWORD="your-device-password"'
    '-DTENANT_ID="your-tenant"'
    '-DAPP_ID="your-app"'
lib_deps =
    knolleary/PubSubClient @ ^2.8
```

> **Where's the SSC-Client library?** It's already in your starter project under **`lib/ssc_client/`** (`ssc_client.h` + `ssc_client.cpp`) — PlatformIO compiles everything in `lib/` automatically, so `#include "ssc_client.h"` just works with no extra setup. The only external dependency you add is **PubSubClient** (the MQTT library it's built on). The one value the library reads from your side is `FIRMWARE_VERSION`, which is already in your build flags.

### Step 3 — `src/main.cpp`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include "ssc_client.h"

WiFiClient wifiClient;
SSCClient  sscClient(wifiClient);

void setup() {
    Serial.begin(115200);

    // 1) WiFi first
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[wifi] connecting");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[wifi] connected — IP: %s\n", WiFi.localIP().toString().c_str());

    // 2) Configure and start SSC-Client
    SSCConfig config = {
        .host          = SSC_BROKER,
        .port          = 1883,
        .clientId      = DEVICE_ID,
        .username      = DEVICE_TOKEN,
        .password      = DEVICE_PASSWORD,
        .tenantId      = TENANT_ID,
        .applicationId = APP_ID
    };
    sscClient.begin(config);

    Serial.println("Module 5 ready.");
}

void loop() {
    sscClient.update();   // service MQTT every loop — this alone keeps you online
}
```

### Step 4 — Verify
1. Upload (`Ctrl+Alt+U`) and open the Serial Monitor (`Ctrl+Alt+S`)
2. Watch the sequence appear in order (see below)
3. Open the SSC dashboard — your device should turn **online** within ~10 seconds

Expected Serial output:
```
[wifi] connecting....
[wifi] connected — IP: 192.168.1.42
[SSC] Broker: mqtt.sensorclouds.app:1883
[SSC] Telemetry: tenant/.../app/.../device/.../data
[SSC] Connecting...
[SSC] Connected
[SSC] Birth message sent
[SSC] Subscribed: tenant/.../device/.../cmd
Module 5 ready.
```

---

## 10. Troubleshooting

| What you see | Likely cause | Fix |
|--------------|--------------|-----|
| Stuck on `[wifi] connecting....` | Wrong SSID/password, or 5 GHz network | Re-check build flags (case-sensitive); use a 2.4 GHz network |
| WiFi connects, then `[SSC] Connection timeout` | Wrong broker host/port | Check `SSC_BROKER` and port 1883 |
| Connects then immediately `Connection lost`, retries | Wrong token / password / IDs | Re-copy Device ID, token, password, tenant, app from dashboard |
| Device online then drops after ~1 min | `update()` not called often enough | Make sure `sscClient.update()` runs every loop, no long `delay()` |
| Build error on the `-D...` lines | Quote formatting | Use `'-DNAME="value"'` exactly |

> **Reading the topics:** the `[SSC]` lines print the exact addresses your device is using. If data later goes missing (Module 7), come back and check these match what the dashboard expects.

---

## 11. Deliverable Checklist

- [x] WiFi connects and prints an IP address
- [x] `WiFiClient` is created and passed into `SSCClient`
- [x] All credentials are build flags — no secrets in `.cpp`/`.h`
- [x] Serial shows the full sequence: WiFi → broker → connected → birth sent
- [x] SSC dashboard shows the device **online**
- [ ] You can explain, in one sentence each: who does WiFi, what `begin()` does, and why `update()` running every loop is what keeps the device online (the MQTT keep-alive — no manual heartbeat)

---

## 12. What's Next

In **[module-06-mqtt-ssc-registration](module-06-mqtt-ssc-registration.md)** you will:
- Learn what MQTT actually is — broker, topic, publish/subscribe — the messaging model under everything you just did
- Understand how SSC turns your tenant/app/device IDs into topics
- See why MQTT (not a normal web request) is the right tool for IoT

