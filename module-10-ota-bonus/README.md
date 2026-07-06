# Bonus Module 10 — OTA Firmware Update *(advanced, optional)*

**Goal:** Update device firmware remotely without physical access.

## Topics

- SSN32 OTA partition scheme: two app slots, `min_spiffs.csv`
- How `OtaHandler` works: download via HTTPS, SHA-256 verify, write inactive partition, reboot
- Firmware versioning: `FIRMWARE_VERSION` build flag in `platformio.ini`
- Birth message reports current version; SSC compares to OTA target
- `checkPendingAck()` on boot: confirms update succeeded

## Lab

Bump `FIRMWARE_VERSION` to `v1.0.1`, build, and trigger an OTA update from the SSC portal. Verify the new version appears in the birth message after reboot.

## Deliverable

Device updates firmware over-the-air. New version confirmed in SSC device info panel.

## Setup

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/). Copy `ssc_client.h`/`ssc_client.cpp` into `lib/ssc_client/` and `ota_handler.h`/`ota_handler.cpp` into `lib/ota_handler/` from the reference implementation ([[projects/ird-temp-compost]] in the wiki). Requires the `min_spiffs.csv` custom partition table (dual OTA app slots) — copy it from the same reference project alongside this module's `platformio.ini`.
