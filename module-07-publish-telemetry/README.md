# Module 7 — Publish Telemetry to SSC

**Goal:** Send real sensor data to the SSC platform and see it on a live dashboard chart.

## Topics

- SSC telemetry payload format: hex-encoded binary
- Encoding a temperature reading: `int16_t` × 100, big-endian bytes, hex string
- `sscClient.publishTelemetry(hexString)` — publish cadence and interval
- Heartbeat: why the device sends a keep-alive every 5 minutes to maintain online status
- Viewing data in the SSC dashboard: chart, latest value, device status

## Lab

Encode the DS18B20 reading into an SSC-compatible payload and publish every 30 seconds. Add a heartbeat every 5 minutes. Verify the temperature chart updates live in SSC.

## Deliverable

Live temperature chart in SSC dashboard. Device stays online between publishes (heartbeat working).

## Setup

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/). Copy `ssc_client.h`/`ssc_client.cpp` into `lib/ssc_client/`.
