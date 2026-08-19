# Module 9 — Receive Commands from Cloud

**Goal:** Control the device remotely from the SSC platform dashboard — completing the bidirectional IoT loop.

## Topics

- MQTT subscribe path: `sscClient.setCommandCallback(cb)` — a single callback with the raw-byte signature `(const char* topic, uint8_t* payload, unsigned int length)`
- Parsing a JSON command straight from the bytes: `deserializeJson(doc, payload, length)`; read `commandId` + fields, validate with `constrain()`
- Applying changes at runtime via mutable variables: no reboot needed
- Building and sending your OWN ACK JSON, then `sscClient.publishAck(buffer)` (the library does not auto-format the ACK)
- One callback handling multiple fields; unknown fields ignored safely

## Lab

Implement a command handler that accepts:

- `sensorReadInterval` — change how often the sensor reads (100–60000 ms)
- `tempOffset` — add a calibration offset to the reported temperature (float)

Send each command from the SSC dashboard. Verify the device responds and ACKs.

## Deliverable

Student changes sensor interval and temperature offset live from SSC dashboard. ACK visible in platform event log. No device reboot required.

## Setup

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/). The `ssc_client` library is already provided in `lib/ssc_client/`, and `ArduinoJson` (bblanchon/ArduinoJson) is in `lib_deps` for parsing inbound commands.
