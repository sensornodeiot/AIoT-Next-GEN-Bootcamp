# Module 10 — OTA Firmware Update *(Bonus — Advanced)*

**Goal:** Update the firmware on a deployed device **remotely** — it downloads a new version, verifies it, installs it, and reboots into it. No USB cable. Like commands, this is **built into the template**; you trigger it from the platform.

**Audience:** Completed [module-09-receive-commands](module-09-receive-commands.md). Optional/advanced — fine to stop after Module 9 and return later.

**Estimated time:** 2–3 hours.

**Deliverable:** A firmware update pushed from the SSC portal; the device reboots and reports the new version.

---

## 1. Why OTA — and why it can't brick the device

Once a device is installed (in a wall, a machine, a field), plugging in a USB cable to update it is expensive or impossible. **OTA (Over-The-Air)** pushes new firmware from the cloud instead.

The obvious fear — *what if power dies mid-update?* — is handled by the ESP32's **two app slots**:

```
bootloader → ota_data (which slot is active)
app0 (running)  ← never touched during an update
app1 (inactive) ← the new image is written here
```

The update writes to the *inactive* slot, verifies a **SHA-256 checksum**, and only then flips the boot pointer. Power lost mid-download? The running slot is untouched — it just boots the old firmware. Bad download? Checksum fails, the new slot is never marked bootable. New firmware crashes on boot? The bootloader **rolls back**. This is why the template's partition table is `min_spiffs.csv` (dual slots) — already set in its `platformio.ini`.

---

## 2. The template already does OTA

`SSCApp` includes the whole OTA lifecycle. When the platform sends an `ota:update` command, the template ACKs progress, downloads over HTTPS, verifies, installs, reboots, and — on the new firmware's first boot — sends a final `ok`. You write **no firmware**. Your only input is the **version number**.

### Firmware version — one build flag
In the template's `platformio.ini`, inside `build_flags` (change only this line, leave the others):
```ini
build_flags =
    -I include
    -D FIRMWARE_VERSION='"1.0.0"'
    -D ARDUINO_LOOP_STACK_SIZE=16384
```
The device reports this in its birth message; the platform compares it to decide whether an update is needed. **Bump it on every release.**

---

## 3. Lab — do an update

### Step 1 — flash v1.0.0
▶ The template ships at `1.0.0`, so just upload over USB the normal way (→ Upload). 👁 SSC shows the device at `1.0.0`.

### Step 2 — build (don't upload) v1.0.1
▶ Change to `FIRMWARE_VERSION='"1.0.1"'`, then click **✓ Build** only (not Upload).
The image is at `sensorclouds-template/.pio/build/esp32dev/firmware.bin`.
*(Mac: Finder hides folders starting with a dot. Press `Cmd + Shift + .` to show `.pio`.)*

### Step 3 — push it from the portal
▶ SSC portal → your device → **OTA** → upload `firmware.bin`, set target `1.0.1`, **Deploy**.

### Step 4 — watch it happen
👁 Keep Serial open. You'll see the handler ACK progress (0/25/50/75 %), download, verify, install, and reboot. After reboot the birth message reports `1.0.1` and the portal marks the update **complete**.

### Step 5 — prove it's safe (the important test)
▶ Repeat, but **pull the USB power partway through the download**. Plug back in. 👁 The device boots the **old** `1.0.0`, unharmed — the running slot was never touched.

> **Rollback command:** the platform can also send `ota:rollback` to switch back to the previous slot and reboot.

---

## 4. Deliverable Checklist

- [ ] Device reported `1.0.0`, then a v1.0.1 image was pushed from the portal
- [ ] Serial showed download → verify → install → reboot; birth then reported `1.0.1`
- [ ] Rollback/safety test: power loss mid-download left the device running `1.0.0`
- [ ] You can explain why two slots + checksum + rollback make bricking practically impossible

---

## Course Complete

Your device now, using the SSC template:
- Connects to Wi-Fi and the SSC cloud from a browser-based config (no credentials in code)
- Publishes your real sensor and **stays online** via periodic telemetry
- Shows data on a dashboard you built
- Accepts commands from the cloud and applies them live, saved across reboots
- Updates its own firmware over the air, safely

### The ideas you'll reuse everywhere
- Stand on a tested library/template; configure rather than re-implement
- A device stays "online" by *publishing regularly*, not just connecting
- MQTT: broker, topics, birth/LWT
- Commands with ACKs; safe remote updates

*(Want to see how the template does all this from the building blocks? [appendix-under-the-hood-ssc-client](appendix-under-the-hood-ssc-client.md).)*

