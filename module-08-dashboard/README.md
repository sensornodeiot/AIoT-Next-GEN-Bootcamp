# Module 8 — SSC Dashboard Walkthrough & Widget Creation

**Goal:** Familiarise interns with the SSC platform UI, understand every widget type available, and build a complete dashboard for the device they have been working with throughout the course.

## Topics

- SSC platform overview: tenants, applications, devices, and the event log
- Dashboard anatomy: layouts, panels, and widget grid
- Widget catalogue walkthrough:
  - **Gauge** — single live value with min/max range
  - **Line chart** — time-series telemetry history
  - **Bar chart** — comparison across multiple data points or devices
  - **Value card** — latest value with unit label and status colour
  - **Status indicator** — online/offline device state
  - **Command button** — trigger a config command directly from the dashboard
  - **Map** — geolocation pin for field-deployed devices
- Creating and naming a dashboard; adding, resizing, and rearranging panels
- Binding a widget to a device channel (data key mapping)
- Setting refresh rate and historical time range per widget
- Sharing a dashboard view with a team or client

## Lab

No firmware — this module is entirely on the SSC platform UI, using the device from Modules 6–7.

1. Add a **line chart** showing the last 24 hours of DS18B20 temperature readings
2. Add a **value card** showing the latest temperature with °C unit
3. Add a **status indicator** showing device online/offline state
4. Add a **command button** that sends `sensorReadInterval = 10000` in one click
5. Rearrange panels into a logical layout and name the dashboard `"[YourName] — IoT Module"`

## Deliverable

A fully configured SSC dashboard. Student can explain what each widget type is for and create new panels independently. Dashboard is shared with the instructor for review.
