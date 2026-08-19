Title: PredictiveMaintenance — An ESP32-Based Vibration and Power Monitoring Platform

Abstract
This project implements a compact, field-ready firmware for the ESP32 that reads vibration data from a WTVB02-485 sensor (Modbus/RS485), optionally ingests power-meter readings, computes simple health scores, and publishes telemetry to MQTT for remote monitoring and analysis. It targets predictive maintenance workflows where low-cost, continuous condition monitoring is required.

Introduction
Rotating and reciprocating machinery degradation often manifests as changes in vibration and power consumption. This firmware integrates a robust Modbus RS485 stack, sensor drivers, and an MQTT publishing pipeline to provide near real-time telemetry and health indicators suitable for cloud dashboards or local logging.

System Design
- Hardware: ESP32 using UART2 for RS485, DE/RE GPIO for half-duplex control. Pins and tunables are centralized in the project configuration.
- Sensors: WTVB02-485 (3-axis accel, velocity, displacement, frequency, temperature). Optional RS485 power meters for voltage/current/power/energy.
- Software: Modular C++ drivers in `lib/` (ModbusRS485, WTVB02Sensor, PowerMeter, HealthScorer). Application services in `src/` orchestrate reads, compute metrics, and manage connectivity.
- Connectivity: WiFi + Async MQTT client for non-blocking publishes, with NVS-backed configuration persistence and OTA support.

Implementation
The `ModbusRS485` layer handles CRC, request/response timing and retries. Sensor drivers convert register payloads into physical units (IEEE-754 float decoding, scale factors, word-swap handling). `sensor_manager` schedules periodic reads (configurable intervals), aggregates results into compact payloads, and forwards them to `mqtt_handler`. `HealthScorer` applies ISO-like RMS thresholds to produce human-readable severity levels and recommendations.

Data & Communication
Telemetry topics follow a tenant/app/device namespace and include separate topics for vibration JSON, binary power payload, and health assessments. Vibration payloads contain timestamped multi-axis metrics and computed RMS; power payloads use a fixed-length scaled integer blob for bandwidth efficiency. QoS=1 ensures delivery with acknowledgement tracking.

Evaluation & Use Cases
- Laboratory validation: 1 Hz reads with serial debug allow rapid verification of sensor conversions.
- Edge deployment: WiFi + MQTT enables integration with cloud dashboards (Sensorclouds or other brokers) and long-term trend analysis.
- Predictive maintenance: Health scoring and thresholds support automated alerts and scheduled inspections.

Limitations & Future Work
- The present health scoring is rule‑based; additional statistical learning on historical data would improve anomaly detection.
- SD/flash logging and time-series buffering for intermittent connectivity are not implemented but can be added.
- Support for more power-meter models and configurable register maps would increase interoperability.

Conclusion
PredictiveMaintenance provides a practical, modular foundation for condition monitoring using low-cost hardware and open protocols. It combines reliable Modbus handling, sensor-specific conversions, non-blocking connectivity, and compact MQTT packaging to enable rapid deployment for maintenance teams and integrators.