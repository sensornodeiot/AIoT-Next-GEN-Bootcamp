# Course Lessons

The full written lessons for the SSN32 IoT Course. (Generated from the course wiki; the wiki is the editable source of truth.)

**New here? Read [Project Setup Guide](project-setup-guide.md) first** — it explains how to open the right project for each module.

## Modules

| # | Lesson | Project you open |
|---|--------|------------------|
| 0 | [Introduction to IoT](module-00-iot-intro.md) | *(install tools)* |
| 1 | [MCU, Toolchain & Serial](module-01-mcu-toolchain-serial.md) | `module-01-mcu-toolchain-serial` |
| 2 | [GPIO: Output & Input](module-02-gpio.md) | `module-02-gpio` |
| 3 | [Sensors: Analog & Digital](module-03-sensors.md) | `module-03-sensors` |
| 4 | [Firmware Patterns & Code Structure](module-04-firmware-patterns.md) | `module-04-firmware-patterns` |
| 5 | [Connect with the SSC Template](module-05-ssc-client.md) | `sensorclouds-template` |
| 6 | [MQTT & the Messaging Model](module-06-mqtt-ssc-registration.md) | `sensorclouds-template` |
| 7 | [Publish Your Real Sensor](module-07-publish-telemetry.md) | `sensorclouds-template` |
| 8 | [SSC Dashboard Walkthrough & Widget Creation](module-08-ssc-dashboard.md) | *(platform, no firmware)* |
| 9 | [Control the Device from the Cloud](module-09-receive-commands.md) | `sensorclouds-template` |
| 10 | [OTA Firmware Update (bonus)](module-10-ota.md) | `sensorclouds-template` |

## Add-ons & reference

- [Project Setup Guide](project-setup-guide.md) — get the code + per-module setup
- [Extending Your Device — Potentiometer, Button & LED](extending-device-io-button-led-pot.md) — add more I/O to the template
- [Appendix — Under the Hood (low-level SSC-Client)](appendix-under-the-hood-ssc-client.md) — optional, how the template works inside
- [Step-by-Step (low-level reference)](modules-5-10-step-by-step.md) — the hand-coded connectivity path (matches `appendix-low-level/`)
