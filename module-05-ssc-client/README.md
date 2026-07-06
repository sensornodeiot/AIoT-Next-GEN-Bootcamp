# Module 5 — SSC-Client Library

**Goal:** Understand what the SSC-Client library does, install it, and establish a working connection to the SSC platform — WiFi, MQTT, birth message, heartbeat, and LWT are all handled by the library so students can focus on using it correctly.

## Topics

- What SSC-Client is and why it exists: a ready-to-use wrapper that handles WiFi connection, MQTT broker connection, birth/death messages, heartbeat, LWT, and topic routing — all in one library
- Library architecture overview: `SSCClient`, `SSCConfig`, callback registration, and the `loop()` call
- How to download and install: adding via PlatformIO `lib_deps` in `platformio.ini`
- `SSCConfig` — what each field means: broker URL, port, device ID, token, heartbeat interval
- `SSCClient.begin()` — what happens internally: WiFi connect → MQTT connect → publish birth message
- `SSCClient.loop()` — why it must be called every iteration of `loop()` and what it does (keep-alive, reconnect, inbound message dispatch)
- Credential hygiene: store device ID and token as `platformio.ini` build flags, never hardcode in source
- Reading the Serial output to confirm connection state: WiFi connected → MQTT connected → birth published

## Lab

1. Add `SSCClient` to `platformio.ini` via `lib_deps`
2. Create an `SSCConfig` populated from build flags
3. Call `sscClient.begin()` in `setup()` and `sscClient.loop()` in `loop()`
4. Open the Serial monitor — confirm the connection sequence logs appear in order
5. Open the SSC dashboard — confirm the device shows **online** and the birth message `{ "state": "online" }` appears in the event log

## Deliverable

Device connects to SSC automatically on power-up with no manual WiFi or MQTT code written by the student. Student can explain what each stage of the connection sequence does and where it is handled inside the library.

## Setup

`platformio.ini` ships with placeholder build flags (`WIFI_SSID`, `WIFI_PASSWORD`, `SSC_DEVICE_ID`, `SSC_TOKEN`). Replace them with your own values locally before building — never commit real credentials back to this repo.
