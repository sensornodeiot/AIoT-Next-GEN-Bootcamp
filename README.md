# SSN32 IoT Course

Hands-on curriculum taking a complete beginner from zero to a cloud-connected SSN32 (ESP32) device publishing sensor data to, and receiving commands from, the SSC (Sensorclouds) platform.

No prior embedded or IoT experience required. Basic programming concepts (variables, loops, functions) are helpful but not assumed.

## Hardware required

- SSN32 Dev Module
- DS18B20 temperature sensor + 4.7 kΩ resistor (1-Wire pull-up)
- LED + 220 Ω resistor
- Push button
- Potentiometer or LDR
- Breadboard, jumper wires, USB cable
- SSC platform account and a registered device

## Software required

- VS Code + PlatformIO extension
- Git (to clone this repo)
- SSC platform dashboard (web browser)

## Getting started

```
git clone https://github.com/sensornodeiot/AIoT-Next-GEN-Bootcamp.git
```

Then **open a single project folder** in VS Code (File → Open Folder) — never the parent repo, or PlatformIO gets confused by the multiple `platformio.ini` files. Which folder to open depends on the module (below).

## Lessons

The full written lessons live in **[`docs/`](docs/)** — start with **[docs/README.md](docs/README.md)**. Each lesson explains the *why* and walks the lab step by step.

## The two kinds of project

| Modules | What you open | Env | You… |
|---------|---------------|-----|------|
| **1–4** (fundamentals) | one starter folder **per module** — `module-01-…`, `module-02-gpio`, `module-03-sensors`, `module-04-firmware-patterns` | `ssn32` | write the firmware, following each lesson |
| **5–10** (connectivity) | **one shared project — `sensorclouds-template/`** | `esp32dev` | flash it, **configure** it in a browser, and edit only `lib/sensor/sensor.cpp` (+ add commands) |

Build / flash / monitor is always the same, swapping the env:
```
pio run -e <env>              # build     (ssn32  for modules 1–4;  esp32dev for the template)
pio run -e <env> -t upload    # flash
pio device monitor -e <env>   # serial @115200
```

*(Modules 0 and 8 have no firmware: Module 0 is tool install, Module 8 is dashboard work on the platform.)*

## Repo layout

```
AIoT-Next-GEN-Bootcamp/
├── docs/                             # the written lessons — start with docs/README.md
├── module-00-intro/                  # Module 0 — intro (no firmware)
├── module-01-mcu-toolchain-serial/   # Modules 1–4 — fundamentals (env: ssn32)
├── module-02-gpio/
├── module-03-sensors/
├── module-04-firmware-patterns/
├── module-08-dashboard/              # Module 8 — dashboard notes (no firmware)
├── sensorclouds-template/            # Modules 5–10 — the SSCApp template (env: esp32dev)  ← use this
└── appendix-low-level/               # OPTIONAL: hand-coded low-level versions of Modules 5–10
```

## Modules 5–10 use the SSCApp template

The `sensorclouds-template/` is a ready-made firmware (`SSCApp`) that already does WiFi + auto-reconnect, MQTT, periodic telemetry, remote commands, and OTA. You don't rebuild it per module — you use the same project throughout:

- **Module 5** — flash it; configure WiFi + SSC credentials in the captive web portal (device becomes AP `SSN32-Config` → `http://192.168.4.1`). No code.
- **Module 7** — edit `lib/sensor/sensor.cpp` (`sensorBegin` / `doRead`) for your DS18B20.
- **Module 9** — add a command in `lib/config_cmd_handler/` (the built-in `config:set_interval` shows the pattern).
- **Module 10** — bump `FIRMWARE_VERSION` and run an OTA from the portal.

Credentials are entered in the portal and saved to the device's flash — **nothing sensitive in the code**.

> **Staying online:** the template publishes telemetry on a timer; that periodic publish (not just connecting) is what keeps the device shown as online.

## Appendix — low-level (optional)

`appendix-low-level/` holds the hand-coded versions of Modules 5–10 that drive the low-level `SSCClient` directly (WiFi + `SSCConfig` + `begin()`/`update()` by hand), env `ssn32`. It's for the curious who want to see what the template does inside — **not** part of the main course path.

## Progression

```
Module 0    Introduction to IoT
──────────────────────────────────────────────
Module 1    MCU, Toolchain & Serial
Module 2    GPIO: Output & Input
Module 3    Sensors: Analog & Digital
Module 4    Firmware Patterns & Code Structure
──────────────────────────────────────────────  ← from here, use sensorclouds-template/
Module 5    Connect with the SSC Template
Module 6    MQTT & the Messaging Model
Module 7    Publish Your Real Sensor
Module 8    SSC Dashboard Walkthrough & Widget Creation
Module 9    Control the Device from the Cloud
──────────────────────────────────────────────
Module 10   OTA Firmware Update (bonus)
```
