# SSN32 IoT Course

Hands-on curriculum taking a complete beginner from zero to a cloud-connected [SSN32](https://github.com/OceanWave17/ssn32-iot-course) ESP32 device publishing sensor data to, and receiving commands from, the SSC (Sensorclouds) platform.

No prior embedded or IoT experience required. Basic programming concepts (variables, loops, functions) are helpful but not assumed.

## Hardware required

- SSN32 Dev Module
- DS18B20 temperature sensor
- 4.7 kΩ resistor (1-Wire pull-up)
- LED + 220 Ω resistor
- Push button
- Potentiometer or LDR (light-dependent resistor)
- Breadboard and jumper wires
- USB cable
- SSC platform account and registered device

## Software required

- VS Code + PlatformIO extension
- Serial monitor (built into PlatformIO)
- SSC platform dashboard (web browser)

## Modules

| Module | Title | Code |
|---|---|---|
| 0 | Introduction to IoT | [module-00-intro](module-00-intro/) |
| 1 | MCU, Toolchain & Serial | [module-01-mcu-toolchain-serial](module-01-mcu-toolchain-serial/) |
| 2 | GPIO: Output & Input | [module-02-gpio](module-02-gpio/) |
| 3 | Sensors: Analog & Digital | [module-03-sensors](module-03-sensors/) |
| 4 | Firmware Patterns & Code Structure | [module-04-firmware-patterns](module-04-firmware-patterns/) |
| 5 | SSC-Client Library | [module-05-ssc-client](module-05-ssc-client/) |
| 6 | MQTT & SSC Device Registration | [module-06-mqtt-registration](module-06-mqtt-registration/) |
| 7 | Publish Telemetry to SSC | [module-07-publish-telemetry](module-07-publish-telemetry/) |
| 8 | SSC Dashboard Walkthrough & Widget Creation | [module-08-dashboard](module-08-dashboard/) |
| 9 | Receive Commands from Cloud | [module-09-receive-commands](module-09-receive-commands/) |
| 10 | OTA Firmware Update *(bonus)* | [module-10-ota-bonus](module-10-ota-bonus/) |

Each module folder (except 0 and 8, which have no firmware lab) is a self-contained PlatformIO project — open it directly in VS Code with the PlatformIO extension, or `cd` into it and run `pio run`.

## Progression

```
Module 0    Introduction to IoT
──────────────────────────────────────────────
Module 1    MCU, Toolchain & Serial
Module 2    GPIO: Output & Input
Module 3    Sensors: Analog & Digital
Module 4    Firmware Patterns & Code Structure
──────────────────────────────────────────────
Module 5    SSC-Client Library
Module 6    MQTT + SSC Device Registration
Module 7    Publish Telemetry → SSC Dashboard
Module 8    SSC Dashboard Walkthrough & Widget Creation
Module 9    Receive Commands ← SSC Dashboard
──────────────────────────────────────────────
Module 10   OTA Firmware Update (bonus)
```

Later modules build on earlier ones — Module 4 refactors Module 3's code into libraries, Module 6 reuses Module 5's SSC-Client setup, and so on. Work through them in order.
