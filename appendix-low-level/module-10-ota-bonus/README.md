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

Same credential pattern as [module-05-ssc-client](../module-05-ssc-client/). The `ssc_client` and `ota_handler` libraries are already provided in `lib/ssc_client/` and `lib/ota_handler/`. The dual-OTA partition table is selected by `board_build.partitions = min_spiffs.csv` in `platformio.ini` (a built-in PlatformIO table — nothing to copy).
