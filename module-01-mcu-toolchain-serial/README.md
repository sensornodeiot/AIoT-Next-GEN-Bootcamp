# Module 1 — MCU, Toolchain & Serial

**Goal:** Understand what a microcontroller is, get the toolchain working, and establish Serial as the primary debug tool used in every module.

## Topics

- MCU vs PC vs single-board computer (Raspberry Pi)
- SSN32 overview: pins, 3.3 V logic, USB, onboard flash
- PlatformIO: install VS Code extension, create a new project, understand `platformio.ini`
- `setup()` and `loop()` — the Arduino execution model
- `Serial.begin(115200)`, `Serial.print()`, `Serial.printf()`
- Structured debug output: timestamp, module name, value
- Debug discipline: print state transitions, not just values

## Lab

Upload a sketch that prints a formatted uptime message to Serial every second:

```
[1000ms] Hello from SSN32 — uptime: 1s
[2000ms] Hello from SSN32 — uptime: 2s
```

TODO in `src/main.cpp`: implement the uptime print using `millis()` — no `delay()`.

## Deliverable

Serial monitor shows repeating formatted output. Student can upload new firmware independently and read Serial output.
