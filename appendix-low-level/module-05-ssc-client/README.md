# Module 5 — SSC-Client Library

**Goal:** Get the device onto the internet and connected to the SSC platform so it shows up **online** in the dashboard. You connect WiFi yourself, then let the SSC-Client library handle the MQTT messaging.

## Topics

- The pieces: WiFi (you connect it) → MQTT broker → SSC platform → dashboard, and which piece does which job
- What SSC-Client is (an MQTT wrapper) and what it is **not** — it does **not** do WiFi. You connect WiFi yourself and pass a `WiFiClient` into it
- Where the library lives: the `ssc_client` files are already in `lib/ssc_client/` — PlatformIO compiles `lib/` automatically, so `#include "ssc_client.h"` just works (nothing to install)
- `SSCConfig` — the real fields: `host`, `port`, `clientId`, `username` (token), `password`, `tenantId`, `applicationId`
- `sscClient.begin(config)` — what it does: build topics, arm LWT, connect, publish birth `{"state":"online","firmwareVersion":...}`, subscribe to `cmd`
- `sscClient.update()` — why it must be called every `loop()` iteration (services MQTT, auto-reconnects with backoff). **Note: it is `update()`, not `loop()`**
- Staying online is **automatic**: the MQTT keep-alive is serviced by `sscClient.update()` every loop — there is no manual heartbeat method
- Credential hygiene: store all IDs/secrets as `platformio.ini` build flags, never hardcode in source

## Lab

1. Connect WiFi yourself (`WiFi.begin`) and create a `WiFiClient`
2. Construct `SSCClient` with that `WiFiClient`
3. Fill `SSCConfig` from build flags (all seven fields); call `sscClient.begin(config)` in `setup()`
4. Call `sscClient.update()` every `loop()` iteration, plus a heartbeat on its own timer
5. Open the Serial monitor — confirm the connection sequence logs appear in order
6. Open the SSC dashboard — confirm the device shows **online** and the birth message appears in the event log

## Deliverable

Device connects to SSC automatically on power-up. Student can explain who does WiFi, what `begin()` does, why `update()` runs every loop, and why the heartbeat is manual.

## Setup

`platformio.ini` ships with placeholder build flags: `WIFI_SSID`, `WIFI_PASSWORD`, `SSC_BROKER`, `SSC_PORT`, `DEVICE_ID`, `DEVICE_TOKEN`, `DEVICE_PASSWORD`, `TENANT_ID`, `APP_ID`, `FIRMWARE_VERSION`. Replace them with your own values locally before building — never commit real credentials back to this repo. The SSC-Client library is already provided in `lib/ssc_client/` — nothing to copy.
