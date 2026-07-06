# Module 4 — Firmware Patterns & Code Structure

**Goal:** Write firmware that handles multiple tasks simultaneously and organize code into reusable libraries.

## Topics

- The single-threaded loop model: everything shares one CPU
- Non-blocking timer pattern: `if (millis() - lastMs >= interval)`
- What is a state machine: states, transitions, events
- PlatformIO `lib/` folder: building a local library
- Header files (`.h`): declarations, include guards
- Source files (`.cpp`): definitions
- `platformio.ini` build flags (`-D`) for configuration constants
- Multiple build environments in one `platformio.ini`

## Lab

Refactor the Module 3 sketch:

- Move DS18B20 logic into `lib/SensorManager/` (`SensorManager.h` + `SensorManager.cpp`)
- Move LED blink logic into `lib/LEDIndicator/`
- Add a `SENSOR_PIN` build flag in `platformio.ini` instead of a hardcoded number
- Run three independent timed tasks in `loop()`: LED blink (300 ms), sensor read (2 s), uptime print (5 s) — none blocking the others

## Deliverable

Clean `main.cpp` that reads like a description of the system. Two reusable libraries. No `delay()` anywhere.
