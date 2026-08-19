
# Module 1 — MCU, Toolchain & Serial

**Goal:** Understand what a microcontroller is, get your development environment running, and establish Serial as the primary debug tool used throughout every module.

**Audience:** No embedded or electronics experience required. Basic programming (variables, loops, functions) is helpful.

**Estimated time:** 2–3 hours (reading + lab)

**Deliverable:** A sketch running on your SSN32 that prints a formatted uptime message to the Serial monitor every second.

---

## 1. What is a Microcontroller?

You have probably heard of three kinds of computing devices:

| Device                          | Examples                 | What makes it special                                                |
| ------------------------------- | ------------------------ | -------------------------------------------------------------------- |
| **PC / laptop**                 | Windows desktop, MacBook | General-purpose OS, runs many programs simultaneously, GBs of RAM    |
| **Single-board computer (SBC)** | Raspberry Pi             | Full Linux OS, USB ports, HDMI — basically a small PC                |
| **Microcontroller (MCU)**       | ESP32, STM32, Arduino    | No OS, runs one program, KBs of RAM, directly controls hardware pins |
|                                 |                          |                                                                      |

A **microcontroller** is a self-contained chip that integrates a processor, memory, and peripheral interfaces (GPIO, ADC, UART, SPI, I2C…) in one package. It runs a single program that you write, stored in its own flash memory. When you apply power, it boots directly into your code — no operating system, no desktop, no login.

### Why use a microcontroller instead of a Raspberry Pi?

| Property | MCU (ESP32) | SBC (Raspberry Pi) |
|---|---|---|
| Boot time | ~300 ms | 30–60 s (Linux boot) |
| Power consumption | 10 µA deep sleep | ~300 mA idle |
| Real-time response | Deterministic (µs) | Non-deterministic (OS scheduler) |
| Cost | ~$4–10 | ~$35–80 |
| Complexity | You control everything | OS manages resources |

MCUs are the right choice when you need **low power**, **fast deterministic response**, or **direct hardware control** — which covers most IoT sensor and actuator scenarios.

---

## 2. Meet the SSN32

The SSN32 is Sensornode IoT Technology's ESP32 development board. It is the hardware you will use throughout this course.

### Key facts

| Feature   | Value                                            |
| --------- | ------------------------------------------------ |
| Module    | ESP32-WROOM-32E                                  |
| Processor | Dual-core Xtensa LX6 @ up to 240 MHz             |
| Flash     | 4 MB (your board)                                |
| RAM       | 520 KB SRAM                                      |
| GPIO      | 26 pins — all 3.3 V logic                        |
| Wireless  | Wi-Fi 802.11 b/g/n + Bluetooth 4.2 BLE           |
| USB       | USB-to-Serial chip — connect via Micro-USB cable |
| Power     | Powered from USB (5 V → onboard 3.3 V regulator) |

### 3.3 V logic — important!

Every GPIO pin on the SSN32 operates at **3.3 V**, not 5 V. Applying 5 V to a GPIO pin will damage the chip. Keep this in mind when connecting sensors or other modules.

### Physical orientation

When you hold the SSN32 with the USB connector facing down:
- The two rows of pins along the left and right edges are the **GPIO pins**
- Each pin is labelled on the board silk-screen (e.g., `IO2`, `IO4`, `3V3`, `GND`)
- The USB connector is used both for **power** and for **uploading your firmware**

> Refer to the SSN32 V1.1 datasheet pinout diagram for the exact pin numbering.

---

## 3. Setting Up the Development Environment

You will write firmware in C++ using the **Arduino framework** inside **PlatformIO**, a professional-grade build system that runs as a VS Code extension.

### Step 1 — Install VS Code

Download and install **Visual Studio Code** from `code.visualstudio.com`.

### Step 2 — Install the PlatformIO extension

1. Open VS Code
2. Click the **Extensions** icon in the left sidebar (or press `Ctrl+Shift+X`)
3. Search for **PlatformIO IDE**
4. Click **Install**
5. Wait for the installation to complete — PlatformIO downloads its core tools the first time it runs. This can take a few minutes.
6. Restart VS Code when prompted

You will see a small house icon (🏠) in the left sidebar — that is the PlatformIO home screen.

### Step 3 — Install the USB driver (Windows only)

The SSN32 uses a CP210x or CH340 USB-to-Serial chip. Windows may need a driver:

- **CP210x:** search "CP210x USB to UART Bridge Driver" on Silicon Labs' website
- **CH340:** search "CH340 driver" on the manufacturer's website

Connect the SSN32 with a Micro-USB cable. Open Device Manager (`Win + X → Device Manager`). Under **Ports (COM & LPT)** you should see a COM port (e.g., `COM3`). If you see an unknown device instead, install the driver for your chip.

---

## 4. Your First PlatformIO Project

### Create the project

1. Click the **PlatformIO home** icon in the left sidebar
2. Click **New Project**
3. Fill in the form:
   - **Name:** `module01-serial`
   - **Board:** type `esp32dev` and select **Espressif ESP32 Dev Module**
   - **Framework:** Arduino
4. Click **Finish** — PlatformIO creates the project folder and downloads the ESP32 toolchain (first time only, ~500 MB)

### Project structure

After creation you will see:

```
module01-serial/
├── platformio.ini       ← project configuration
├── src/
│   └── main.cpp         ← your firmware code goes here
├── include/             ← your header files
├── lib/                 ← your local libraries
└── .pio/                ← generated build files (never edit or commit this)
```

### Understanding platformio.ini

Open `platformio.ini`. It looks like this:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

| Key                      | Meaning                                                                             |
| ------------------------ | ----------------------------------------------------------------------------------- |
| `[env:esp32dev]`         | Names this build environment — you can have multiple environments in one file       |
| `platform = espressif32` | The hardware platform (Espressif chip family)                                       |
| `board = esp32dev`       | The specific board definition — esp32dev matches the SSN32's ESP32-WROOM-32E layout |
| `framework = arduino`    | Use the Arduino SDK (gives you `setup()`, `loop()`, `Serial`, etc.)                 |

You will add more settings to this file in later modules (libraries, build flags, upload speed).

---

## 5. The Arduino Execution Model

Open `src/main.cpp`. Replace its contents with this minimal sketch:

```cpp
#include <Arduino.h>

void setup() {
    // Runs once when the board powers on or resets
}

void loop() {
    // Runs over and over, forever
}
```

The `#include <Arduino.h>` line is required when using PlatformIO — unlike the Arduino IDE, PlatformIO does not add it automatically.

### setup() and loop()

```
Power on / reset
       │
       ▼
   setup()          ← runs exactly once
       │
       ▼
   loop()  ──────── ← runs repeatedly until power is removed
       │    ↑
       └────┘
```

`setup()` is where you configure hardware: initialize Serial, set pin modes, connect to Wi-Fi, etc.

`loop()` is where your main program runs — reading sensors, checking conditions, sending data.

There is **no operating system** managing this. Your code is the only thing running (unless you use FreeRTOS, which is a later topic). If `loop()` blocks — for example, with a `delay(5000)` — the entire MCU sits idle for those 5 seconds, unable to do anything else.

---

## 6. Serial Communication

**Serial** is the way your MCU talks to your computer through the USB cable. The SSN32 has a dedicated **USB-to-Serial chip** on the board that converts UART signals to USB, so you do not need any extra hardware.

When you open the **Serial Monitor** in PlatformIO, you are seeing the text your MCU prints over this connection.

### Initializing Serial

You must call `Serial.begin()` in `setup()` before using any other Serial functions:

```cpp
void setup() {
    Serial.begin(115200);
}
```

`115200` is the **baud rate** — the speed of the serial connection in bits per second. Both the MCU and your Serial Monitor must use the same rate, or you will see garbage characters. `115200` is the standard for ESP32 development.

### Printing text

| Function                                       | What it does                              |
| ---------------------------------------------- | ----------------------------------------- |
| `Serial.print("hello")`                        | Prints text with no newline at the end    |
| `Serial.println("hello")`                      | Prints text and moves to the next line    |
| `Serial.printf("[%d] value: %.2f\n", ms, val)` | Formatted print — works like C `printf()` |

`Serial.printf()` is the most useful. It lets you embed variable values directly in the output string without multiple `print()` calls.

#### printf format codes

| Code | Type | Example |
|------|------|---------|
| `%d` | integer (decimal) | `42` |
| `%u` | unsigned integer | `3000` |
| `%lu` | unsigned long | `86400000` |
| `%f` | float (6 decimal places) | `3.141593` |
| `%.2f` | float (2 decimal places) | `3.14` |
| `%s` | string | `hello` |

`\n` at the end of a `printf()` string moves to the next line (same as `println`).

> **Reading a format string — `%lums` is NOT one code.** This trips up everyone at first. In `"[%lums] uptime: %lus"`, the compiler only reacts to the `%...` parts; everything else is printed literally. So `%lums` is really **`%lu`** (fill in an unsigned long) **+ the plain letters `ms`**. The output for `now = 2000` is `[2000ms]`. Likewise `%lus` is `%lu` + the letter `s` → `2s`.
>
> **Why `%lu` and not `%d`?** The `%` code must match the *type* of the value. `millis()` returns an **`unsigned long`** (a big, always-positive number), and `%lu` is exactly "unsigned long." Using `%d` (a plain int) with it can print wrong numbers once the value gets large. Rule of thumb for this course: **anything from `millis()` gets `%lu`.**

### Opening the Serial Monitor

In VS Code with PlatformIO:
1. Connect the SSN32 via USB
2. Click the **plug icon** at the bottom status bar (or press `Ctrl+Alt+S`)
3. Set the baud rate to **115200**

> If you see scrambled characters, the baud rate does not match. Check that both `Serial.begin()` and the monitor are set to `115200`.

---

## 7. millis() — Time Without Blocking

To print every second, you need to track time. The function `millis()` returns the number of **milliseconds** since the MCU booted. It is a `unsigned long` (32-bit), which overflows after ~49 days — not a concern for now.

```cpp
unsigned long now = millis();  // e.g., 4321 ms after boot
```

You can use `millis()` to measure elapsed time:

```cpp
if (millis() - lastTime >= 1000) {
    // one second has passed
    lastTime = millis();
}
```

This pattern — check elapsed time in `loop()`, act when the interval has passed — is used **everywhere** in embedded firmware. You will see it in every module. For now, just know what `millis()` returns; you will learn the full non-blocking pattern in Module 2.

---

## 8. Structured Debug Output

A good debug print tells you **what happened, when, and where**. Compare:

```
23.5
23.6
23.4
```

vs.

```
[2000ms] sensor: temp = 23.5 C
[4000ms] sensor: temp = 23.6 C
[6000ms] sensor: temp = 23.4 C
```

The second format tells you the timestamp, which module produced the message, and the unit. When your firmware has 10 different subsystems printing to Serial at the same time, structured output is the only way to make sense of it.

### Recommended format

```
[<millis>ms] <module>: <message>
```

Examples:
```
[1000ms] Hello from SSN32 — uptime: 1s
[500ms] wifi: connecting to MyNetwork...
[2340ms] sensor: temp = 23.5 C, humidity = 61.2 %
[5000ms] mqtt: published payload (42 bytes)
```

### Debug discipline

Print **state transitions** and **meaningful events**, not constant values:

| Bad (too much noise) | Good (meaningful) |
|---|---|
| Print sensor value every 10 ms | Print sensor value every read interval |
| Print "in loop()" every iteration | Print "sensor timeout — retrying" once |
| Print raw ADC value | Print converted value with unit |

If your Serial output scrolls so fast you cannot read it, you are printing too much.

---

## 9. Lab — Uptime Printer

Write a sketch that prints a formatted uptime message to Serial once per second. The expected output:

```
SSN32 ready.
[1000ms] Hello from SSN32 — uptime: 1s
[2000ms] Hello from SSN32 — uptime: 2s
[3000ms] Hello from SSN32 — uptime: 3s
```

### Complete code

Open `src/main.cpp` and replace the entire file with:

```cpp
#include <Arduino.h>

unsigned long lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("SSN32 ready.");
}

void loop() {
    unsigned long now = millis();

    if (now - lastPrint >= 1000) {
        lastPrint = now;
        unsigned long uptimeSec = now / 1000;
        Serial.printf("[%lums] Hello from SSN32 — uptime: %lus\n", now, uptimeSec);
    }
}
```

### Code walkthrough

```cpp
unsigned long lastPrint = 0;  // declared before setup()
```
Variables declared outside `setup()` and `loop()` are **global** — they exist for the lifetime of the program and keep their value between calls to `loop()`. Every timer variable in this course follows this same pattern.

```cpp
unsigned long now = millis();
```
Capture the current time at the top of the loop — use this one snapshot throughout the function rather than calling `millis()` multiple times.

```cpp
if (now - lastPrint >= 1000) {
```
Has at least 1000 ms passed since the last print? If yes, execute the block. If no, skip and return to the top of `loop()` immediately — the MCU is free to do other work.

```cpp
lastPrint = now;
```
Update the timestamp **before** doing the work, so the next interval starts from this moment.

```cpp
Serial.printf("[%lums] Hello from SSN32 — uptime: %lus\n", now, uptimeSec);
```
`%lu` is the format code for `unsigned long`. On ESP32 (32-bit), `unsigned long` is 32 bits; `%lu` matches it correctly.

### Build and upload

1. Connect the SSN32 to your computer via USB
2. In VS Code, press `Ctrl+Alt+U` (or click the **Upload** arrow button at the bottom) — PlatformIO compiles the code and uploads it
3. Wait for the message: `=== [SUCCESS] Took X.XX seconds ===`
4. Open the Serial Monitor (`Ctrl+Alt+S`) at **115200 baud**
5. Press the **RST (Reset)** button on the SSN32 — the board restarts and you will see the output appear

> If upload fails with "Could not open port", check that no other program (e.g., Arduino IDE) has the COM port open, and that the correct COM port is detected in Device Manager.

### Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| Scrambled characters in monitor | Baud rate mismatch | Set monitor to 115200 |
| No output at all | Board not in normal boot mode | Hold RST, release; or hold BOOT then press RST then release BOOT |
| Upload fails — port not found | Driver not installed or wrong COM port | Reinstall USB driver; check Device Manager |
| `[0ms]` repeats immediately | `lastPrint` not updating | Ensure `lastPrint = now;` is inside the `if` block |
| Compile error: `'Serial' not declared` | Missing `#include <Arduino.h>` | Add the include at the top of the file |

---

## 10. Deliverable Checklist

Before moving to Module 2, verify:

- [x] VS Code and PlatformIO extension are installed
- [x] SSN32 is detected as a COM port in Device Manager (Windows) or `/dev/ttyUSB0` (Linux/Mac)
- [x] Project builds without errors (`=== [SUCCESS] ===`)
- [x] Firmware uploads to the SSN32
- [x] Serial Monitor shows `SSN32 ready.` on boot
- [x] Formatted uptime lines appear once per second with correct timestamps
- [x] You can modify the message text, re-upload, and see the new text — without help

---

## 11. What's Next

In **[module-02-gpio](module-02-gpio.md)**, you will:
- Control an LED wired to a GPIO pin
- Read a push button with reliable press detection
- Learn the non-blocking timer pattern (`millis()`) properly — why `delay()` breaks firmware and how to replace it
