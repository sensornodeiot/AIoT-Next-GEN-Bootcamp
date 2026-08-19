# Module 4 — Firmware Patterns & Code Structure

**Goal:** Write firmware that handles multiple tasks simultaneously and organize code into tidy header/source files with plain functions.

## Topics

- The single-threaded loop model: everything shares one CPU
- Non-blocking timer pattern: `if (millis() - lastMs >= interval)`
- What is a state machine: states, transitions, events (the "ask now, read later" sensor pattern)
- Splitting code into files: header (`.h`) as the menu, source (`.cpp`) as the kitchen, `#pragma once`
- Organising *your own* code with plain functions + `static` file-scope state
  (you *use* library classes like `DallasTemperature`, but you don't need to *write* one — that's an optional "go further" step)
- `platformio.ini` build flags (`-D`) for configuration constants (pins)
- Multiple build environments in one `platformio.ini`

## Lab

Refactor the Module 3 sketch:

- Move DS18B20 logic into `src/sensors.h` + `src/sensors.cpp` (plain functions: `sensorsBegin()`, `sensorsUpdate()`, `getTemperature()`)
- Move LED blink logic into `src/led.h` + `src/led.cpp`
- Use the `SENSOR_PIN` and `LED_PIN` build flags in `platformio.ini` instead of hardcoded numbers
- Run three independent timed tasks in `loop()`: LED blink (300 ms), sensor read (2 s), uptime print (5 s) — none blocking the others

## Deliverable

Clean `main.cpp` that reads like a description of the system. Two file pairs of plain-function code, no pin numbers in source. No `delay()` anywhere.
