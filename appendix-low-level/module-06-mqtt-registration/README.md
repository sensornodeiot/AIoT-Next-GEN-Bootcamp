# Module 6 — MQTT & SSC Device Registration

**Goal:** Understand the messaging protocol the SSC platform uses and get a device registered and showing online.

## Topics

- What is MQTT: broker, client, topic, publish, subscribe, QoS
- Why MQTT is used in IoT (vs HTTP): persistent connection, low overhead, bidirectional
- SSC platform structure: tenant, application, device, token
- `SSCClient` library overview: what it wraps (PubSubClient), what it adds (birth message, heartbeat, LWT, topic routing)
- Register a device on the SSC dashboard; copy device ID and token

## Lab

Add `SSCClient` to the project (`lib/ssc_client/`). Configure `SSCConfig` with device credentials. Connect to the SSC broker. Verify the device appears **online** in the dashboard and the birth message `{ "state": "online" }` is visible in the platform event log.

## Deliverable

Device shows as online in SSC dashboard. Student understands the broker/topic/client model.

## Setup

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/) — edit the placeholder build flags in `platformio.ini` locally, never commit real values. The `ssc_client` library is already provided in `lib/ssc_client/`.
