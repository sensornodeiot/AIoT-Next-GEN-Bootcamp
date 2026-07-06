# Module 9 — Receive Commands from Cloud

**Goal:** Control the device remotely from the SSC platform dashboard — completing the bidirectional IoT loop.

## Topics

- MQTT subscribe path: how `SSCClient` delivers inbound messages via callback
- Parsing a JSON config command: field name, value, `commandId`
- Applying changes at runtime: no reboot needed
- Sending an ACK back to the platform: `publishAck(commandId)`
- Command routing pattern: one callback, multiple command types

## Lab

Implement a command handler that accepts:

- `sensorReadInterval` — change how often the sensor reads (100–60000 ms)
- `tempOffset` — add a calibration offset to the reported temperature (float)

Send each command from the SSC dashboard. Verify the device responds and ACKs.

## Deliverable

Student changes sensor interval and temperature offset live from SSC dashboard. ACK visible in platform event log. No device reboot required.

## Setup

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/). Copy `ssc_client.h`/`ssc_client.cpp` into `lib/ssc_client/`. Also add `ArduinoJson` (bblanchon/ArduinoJson) via `lib_deps` to parse inbound commands.
