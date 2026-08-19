
# Project Setup Guide

**What this is:** exactly how a student gets the code and opens the right project for every module — including how to get and use the **SensorClouds template** for Modules 5–10. Read this once before Module 1.

> **The one thing to understand up front:** the course has **two kinds of project**.
> - **Modules 1–4 (fundamentals)** — each module is its **own** small starter project (`module-0X/`), env **`ssn32`**. You fill in the code.
> - **Modules 5–10 (connectivity)** — you use **one shared project**, the **`sensorclouds-template/`**, env **`esp32dev`**. You flash it and *configure* it; you don't rebuild it per module.

---

## 1. One-time tool install

1. **VS Code** — `code.visualstudio.com`
2. **PlatformIO IDE** extension — VS Code → Extensions (`Ctrl+Shift+X`) → search "PlatformIO IDE" → Install → restart. Wait for the 🏠 icon.
3. **Git** — `git-scm.com` (to clone the course code)
4. **USB driver** (Windows) — CP210x or CH340, so the SSN32 shows up as a COM port. (Details in [module-01-mcu-toolchain-serial](module-01-mcu-toolchain-serial.md).)

---

## 2. Get the course code (once)

Clone the course repository:

```
git clone https://github.com/OceanWave17/ssn32-iot-course.git
cd ssn32-iot-course
```

Inside you'll find one folder per module, plus the template:

```
ssn32-iot-course/
├── module-01-mcu-toolchain-serial/   ← Modules 1–4: one starter project each
├── module-02-gpio/
├── module-03-sensors/
├── module-04-firmware-patterns/
├── sensorclouds-template/            ← Modules 5–10: the SSCApp template (use this)
└── appendix-low-level/               ← optional "under the hood" low-level versions of 5–10
```

*(Modules 0 and 8 have no firmware — Module 0 is the tool install above; Module 8 is dashboard work on the platform.)*

---

## 3. Setting up a fundamentals module (1–4)

Each is a self-contained PlatformIO project.

1. **VS Code → File → Open Folder →** the module folder itself, e.g. `module-02-gpio`
   *(Open the module folder, **not** the parent `ssn32-iot-course` — PlatformIO needs `platformio.ini` at the root of the open window.)*
2. Wait for the PlatformIO bar (✓ 🔌 …) at the bottom.
3. Follow the module's lesson to fill in `src/main.cpp`.
4. Build / upload / monitor (env is **`ssn32`**):
   ```
   pio run -e ssn32                 # build
   pio run -e ssn32 -t upload       # flash
   pio device monitor -e ssn32      # serial @115200
   ```
   (Or the PlatformIO toolbar buttons / `Ctrl+Alt+U`, `Ctrl+Alt+S`.)

---

## 4. Setting up the connectivity half (Modules 5–10)

**All of Modules 5–10 use the same project: `sensorclouds-template/`.** You don't switch folders between these modules — you keep working in the template, adding to it as you go:

- **Module 5** — flash it and configure it in the browser (captive portal). *(No code.)*
- **Module 7** — edit `lib/sensor/sensor.cpp` (your DS18B20).
- **Module 9** — add a command in `lib/config_cmd_handler/`.
- **Module 10** — bump `FIRMWARE_VERSION`, do an OTA from the portal.

Open it and build (env is **`esp32dev`** here, not `ssn32`):

```
# from inside the course repo
# VS Code → Open Folder → sensorclouds-template
pio run -e esp32dev                 # build
pio run -e esp32dev -t upload       # flash
pio device monitor -e esp32dev      # serial @115200
```

On first flash the device has no config, so it starts a Wi-Fi hotspot — see [module-05-ssc-client](module-05-ssc-client.md) for the portal walkthrough.

---

## 5. How to get / pull the SensorClouds template

**Option A — it's already in the course repo (recommended).** When you cloned `ssn32-iot-course` (Section 2), you got `sensorclouds-template/`. Just open that folder. This copy is pre-tailored for the workshop (beginner `sensor.cpp`, DS18B20-ready).

**Option B — pull the official upstream template.** The template comes from the public repo `github.com/sensornodeiot/ssc-client`:
```
git clone https://github.com/sensornodeiot/ssc-client.git
```
Open that folder (env `esp32dev`). ⚠ The upstream default `lib/sensor/sensor.cpp` reads a **WTVB02-485 vibration sensor over RS485**, not a DS18B20 — so replace it with the workshop `sensor.cpp` (see [module-07-publish-telemetry](module-07-publish-telemetry.md)) before it will publish temperature.

Use **Option A** for the workshop; Option B is for anyone who wants the latest official version.

---

## 6. Module → project → command reference

| Module | Open this folder | Env | You do |
|--------|------------------|-----|--------|
| 0 | *(none)* | — | install tools (Section 1) |
| 1 | `module-01-mcu-toolchain-serial` | `ssn32` | fill in `main.cpp` |
| 2 | `module-02-gpio` | `ssn32` | fill in `main.cpp` |
| 3 | `module-03-sensors` | `ssn32` | fill in `main.cpp` |
| 4 | `module-04-firmware-patterns` | `ssn32` | split into `src/` files |
| **5–10** | **`sensorclouds-template`** | **`esp32dev`** | flash + configure + edit `sensor.cpp` / commands |
| 8 | *(none)* | — | dashboard work on the platform |
| Appendix | `appendix-low-level/module-0X` | `ssn32` | optional hand-coded low-level versions |

Build/flash/monitor is always the same three commands, swapping the env:
```
pio run -e <env>            /  -t upload            /  pio device monitor -e <env>
```

---

## 7. For the instructor

- **Push the template first.** For students to `git clone` the course repo and receive `sensorclouds-template/`, the commit that added it must be **pushed to GitHub** (`git push`). Until then it exists only locally.
- Have students verify their toolchain on **Module 1** before the workshop day — a clean `pio run -e ssn32` on `module-01` proves VS Code + PlatformIO + drivers all work.
- For Modules 5–10, pre-register each student's SSC device so the portal step (Module 5) isn't a bottleneck.

---

## Related

- [module-01-mcu-toolchain-serial](module-01-mcu-toolchain-serial.md) · [module-05-ssc-client](module-05-ssc-client.md) · [module-07-publish-telemetry](module-07-publish-telemetry.md)
- Project: iot-learning-module
