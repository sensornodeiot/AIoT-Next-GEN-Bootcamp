# WTVB05 Vibration Sensor — Quick Reference

This document summarizes how to read and interpret data from the WTVB05 vibration sensor (Modbus RTU over RS485 or CAN). Use this as a compact reference when integrating the sensor into the PredictiveMaintenance firmware.

## Overview
- Interfaces: Modbus RTU (RS485) or CAN (active broadcast)
- Default Modbus ID: `0x50`
- Default baud: `9600` bps

---

## 1) Modbus RTU (RS485)
- Protocol: Modbus Function Code `0x03` (Read Holding Registers).
- Data format: registers returned as 16-bit words; multi-word values follow sensor conventions (floats/ints as documented below).

Key registers and conversion formulas:
- 3-axis Acceleration: registers `0x34`–`0x36`.
  - Acceleration (g) = (RegisterValue / 32768) × 16
- 3-axis Vibration Speed (Velocity): registers `0x3A`–`0x3C`.
  - Velocity (mm/s) = RegisterValue / 100  (firmware ≥ v1.1.7)
- 3-axis Displacement: registers `0x41`–`0x43`.
  - Returned value in μm; resolution depends on "Displacement range mode" at register `0x2B` (1 μm or 0.01 μm).
- 3-axis Frequency: registers `0x44`–`0x46`.
  - Frequency (Hz) = RegisterValue / 10
- Temperature: register `0x40`.
  - Temperature (°C) = RegisterValue / 100

Example: to read three-axis acceleration, request `0x34` length `0x03` from the sensor ID.

Notes:
- Word/byte order is sensor-specific; validate with a known vector and toggle any word-swap option in your Modbus layer if values look incorrect.
- Minimum useful vibration frequency is sensor-dependent—consult datasheet for low-frequency behavior.

---

## 2) CAN Protocol (Active Broadcast)
- Packet framing: each packet begins with header `0x55` followed by a `TYPE` byte that indicates content.
- Common TYPE values:
  - `0x51` : Acceleration
  - `0x55` : Temperature
  - `0x5B` : Vibration speed (velocity)
  - `0x5C` : Vibration displacement
  - `0x5D` : Vibration frequency

Parsing rules:
- Values are transmitted as LowByte then HighByte. Combine into a signed 16-bit integer:
  - `Value = (short)((HighByte << 8) | LowByte)`
- Apply the same conversion formulas as Modbus after combining (accel: /32768×16, velocity: /100, frequency: /10, temp: /100).

---

## 3) Spectral Energy & Harmonics
- The sensor exposes spectral-energy registers for detailed frequency analysis.
- X-axis energy registers: `0x6E`–`0x75` (per-point harmonics) and `0x76`–`0x7B` (band energies).
- Y-axis energy registers: `0x7D`–`0x84` and `0x85`–`0x8A`.
- Z-axis energy registers: `0x8C`–`0x93` and `0x94`–`0x99`.
- Total energy per axis: `0x7C` (X), `0x8B` (Y), `0x9A` (Z).

Use these registers to compute harmonic amplitudes, narrowband energy, and to detect resonance or imbalance across specified bands.

---

## 4) Raw FIFO (Original Acceleration Data)
- Register `0x2C` provides access to the Original Acceleration FIFO.
- A successful read returns 1024 packets (6,144 bytes total for X/Y/Z), useful for custom signal processing and FFT analysis.
- Read operation is large; ensure Modbus timeouts and buffer sizes on the master are configured to accept the full transfer.

---

## Integration Notes for PredictiveMaintenance
- The project's `lib/ModbusRS485/` handles CRC16, timeouts, and retries — reuse it for WTVB05 Modbus reads.
- For CAN, implement a small parser that recognizes `0x55` header packets and dispatches by TYPE byte into the same conversion pipeline used for Modbus-derived values.
- Verify `POWER`/`WORD_SWAP` and endianness when first integrating: compare a known reference reading (static tilt or gravity for accel) to detect byte/word order issues.

---

## Quick Troubleshooting
- No response via RS485: check wiring polarity (A/B), terminating resistor, baud, and device address.
- Bad numeric values: try toggling word-swap or interpreting values as signed/unsigned appropriately.
- Partial FIFO reads: increase Modbus timeout and verify the master supports large block transfers.

---

## References
- Sensor datasheet: WTVB05 (manufacturer)
- Project Modbus helper: `lib/ModbusRS485/`
- Example WTVB02 docs: `docs/hardware/vibration-sensor.md`

(Prepared for integration into the PredictiveMaintenance firmware.)
