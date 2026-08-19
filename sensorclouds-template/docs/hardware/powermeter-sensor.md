# Power Meter (RS485)

This document describes the RS485 power meter used with the PredictiveMaintenance firmware: wiring, Modbus register map, driver location, configuration notes, and verification steps.

## Overview

- Interface: RS485 (Modbus RTU)
- Driver location: `lib/PowerMeter/`
- Typical use: read voltage, current, power, energy and publish to MQTT as a binary scaled payload

## Hardware Wiring

- Connect RS485 A/B to the meter's A/B lines (observe polarity).
- Use a common ground when needed (some meters isolate ground).
- The project uses UART2 on the ESP32 for RS485 (see `include/config.h` for pin constants).
- DE/RE control: connect to `RS485_DE_PIN` and drive high for TX, low for RX (non-blocking driver handles toggling).

## Typical Pinout (ESP32)

- RX2: GPIO16 (RS485 RX)
- TX2: GPIO17 (RS485 TX)
- DE/RE: GPIO4 (RS485 direction control)
- Status LED: GPIO2

Confirm these values in [include/config.h](include/config.h#L1).

## Modbus Register Map (common meter ranges)

The exact register addresses depend on meter make/model. Below are the ranges used by the project's `PowerMeter` driver by default; adapt `word-swap` and offsets if your meter differs.

- Voltage (V): holding registers `0x2000`..`0x2002` (phase A/B/C)
- Current (A): holding registers `0x2003`..`0x2005` (phase A/B/C)
- Active Power (W): `0x2006`..`0x2008`
- Power Factor: `0x2009`..`0x200B`
- Frequency (Hz): `0x200C`
- Energy (Wh): `0x4000`..`0x4001` (64-bit / 32+32 depending on meter)

Scale factors and register widths vary. The driver decodes IEEE-754 floats and scaled integers; enable `WORD_SWAP` in `config.h` only if your meter stores words in reverse order.

### Example Modbus Read

To read three voltage floats at device ID `0x01` perform a Read Holding Registers (FC03) for the appropriate address and length. The `ModbusRS485` library handles CRC16 and timeouts.

## Driver & Code

- Driver: `lib/PowerMeter/power_meter.h` and `power_meter.cpp`
  - Constructor accepts a `ModbusRS485 *` instance and the Modbus slave ID.
  - API: `initialize()` and `readSensorData(PowerMeterData &out)` returning `bool`.
- Sensor manager integration: `src/app/sensor_manager.*` schedules periodic reads and passes data to `mqtt_handler` for publishing.

## Configuration

Edit `include/config.h` for these constants:

- `POWER_METER_ENABLED` — enable/disable driver
- `POWER_METER_MODBUS_ID` — meter slave ID
- `POWER_METER_READ_INTERVAL` — read period in ms
- `POWER_METER_WORD_SWAP` — toggle if float words are swapped

See [CLAUDE.md](CLAUDE.md#L1) for repository conventions and `include/config.h` for current values.

## MQTT Payloads

The firmware publishes power meter data as a compact binary payload to the topic built from stored NVS credentials:

```
tenant/{tenantId}/app/{appName}/device/{deviceToken}/power
```

The payload format is a fixed-length scaled integer blob produced by `mqtt_handler` — consult `src/mqtt/mqtt_handler.cpp` for exact packing.

## Verification & Testing

1. Enable the power meter in `include/config.h`.
2. Build and verify:

```bash
pio run -e esp32dev
```

3. Connect the meter and power the system.
4. Use the serial monitor to check sensor initialization messages:

```bash
pio device monitor -e esp32dev -b 115200
```

5. Confirm MQTT publishes on the `/power` topic (use `mosquitto_sub` or your broker UI).

## Troubleshooting

- No response from meter: verify wiring, baud rate, and slave ID. Use a USB-RS485 adapter and a PC Modbus tool to confirm meter responds.
- Incorrect values / garbled floats: try toggling `POWER_METER_WORD_SWAP` or adjust register offsets.
- CRC/timeouts: ensure DE/RE direction toggling delay is sufficient and there are no bus collisions.

## References

- Driver: `lib/PowerMeter/` (implementation and register constants)
- Modbus helper: `lib/ModbusRS485/`
- Project README: [README.md](README.md#L1)

---

Document created by the repository assistant. If you want additional meter-specific register maps (e.g., for models X, Y, Z), tell me which model and I'll add them.
