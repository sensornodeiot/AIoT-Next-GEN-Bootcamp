# Module 3 — Sensors: Analog & Digital

**Goal:** Read both analog and digital sensors, understand the difference, and wire up the DS18B20 used in all connectivity modules.

## Topics

- ADC basics: how analog-to-digital conversion works; 12-bit, 0–3.3 V on SSN32
- `analogRead()`, mapping raw value to a meaningful unit
- Noise and averaging: why raw ADC readings fluctuate
- Why digital sensors exist: accuracy, noise immunity, addressability
- 1-Wire protocol basics: single data line, pull-up resistor, device ROM address
- DS18B20 wiring (VCC, GND, DATA + 4.7 kΩ to VCC)
- Using `OneWire` + `DallasTemperature` libraries in PlatformIO
- Configuring resolution (9–12 bit) and conversion time tradeoff

## Lab

- **Part A** — Wire a potentiometer to GPIO 34. Print a rolling average of 10 samples mapped to 0–100%.
- **Part B** — Wire DS18B20 to GPIO 32. Print temperature in °C every 2 seconds using a `millis()` timer.

## Deliverable

Smooth analog percentage printed to Serial. Live DS18B20 temperature readings printed independently. Both running in the same `loop()` without blocking each other.
