# Module 8 — SSC Dashboard Walkthrough & Widget Creation

**Goal:** Learn every widget the SSC dashboard offers and build a complete, shared dashboard for the device used throughout Modules 6–7.

**Audience:** Completed [module-07-publish-telemetry](module-07-publish-telemetry.md). Device must be publishing telemetry.

**Estimated time:** 2–3 hours (dashboard configuration + lab)

**Deliverable:** A fully configured SSC dashboard — line chart, value card, status indicator, command button — shared with the instructor.

---

## 1. SSC Platform Overview

The SSC (Sensorclouds) platform is organised into:

```
Tenant → Application → Device → Channels / Events
                              └── Dashboards
```

**Dashboards** are separate from devices. One dashboard can contain widgets bound to multiple devices. One device's data can appear on multiple dashboards.

### Navigation

| Section | What you find there |
|---------|-------------------|
| **Applications** | List of all your device groups |
| **Devices** | Individual device pages, event logs, live data |
| **Dashboards** | Visual panels you build and share |
| **Settings** | Tenant-level config, user management |

---

## 2. Dashboard Anatomy

A dashboard is a grid of **panels**. Each panel contains one widget. You can:
- Add, remove, resize, and reposition panels freely
- Bind each widget to any device channel in your tenant
- Set per-widget refresh rate and historical time range

To create a dashboard: **Dashboards → New Dashboard → Give it a name**.

---

## 3. Widget Catalogue

### Gauge

Displays a single live value as a needle on a dial with a min/max arc.

**Best for:** Instantaneous values with a defined safe range — battery voltage, temperature alarm threshold, tank fill level.

**Configuration:**
- Channel: select device + channel
- Min / Max: define the scale
- Thresholds: colour bands (green/yellow/red)

### Line Chart

Plots channel values over time as a continuous line.

**Best for:** Trending data — temperature over 24 hours, power consumption over a week, vibration over a shift.

**Configuration:**
- Channel(s): one or more channels on the same chart
- Time range: last 1 hour / 24 hours / 7 days / custom
- Refresh rate: how often the chart fetches new data

### Bar Chart

Compares values across multiple data points or devices as vertical bars.

**Best for:** Comparing multiple devices side by side, or comparing channels at different time buckets (daily kWh by day of week).

### Value Card

Shows the latest value for one channel with a unit label and optional status colour.

**Best for:** "What is the temperature right now" — a single prominent number with unit.

**Configuration:**
- Channel: device + channel
- Unit: °C, mm/s, kWh, etc.
- Colour thresholds: change card background when value exceeds a limit

### Status Indicator

Shows a device's online/offline state as a coloured dot or badge.

**Best for:** At-a-glance health check — is this device connected right now?

**Configuration:**
- Device: select a registered device
- The indicator automatically follows the birth/LWT state

### Command Button

A button in the dashboard that sends a pre-configured config command to a device when clicked.

**Best for:** Operations staff who need to change a setting without writing code — "Reset interval to 10 s", "Enable debug mode".

**Configuration:**
- Device: target device
- Command: the command name the device understands — the template's built-in `config:set_interval`
- Parameters: the JSON payload sent with it, e.g. `{ "publish_interval_ms": 10000 }`

### Map

Pins a device at a geographic location on a world map.

**Best for:** Field-deployed assets — agricultural sensors, fleet tracking, remote meters.

**Configuration:**
- Device: select device
- Location: set latitude/longitude manually, or bind to a GPS channel if the device reports location

---

## 4. Creating and Configuring Panels

### Add a panel

1. Open your dashboard → click **Add Panel** (or the `+` button)
2. Select widget type
3. Configure the data binding (device + channel)
4. Set the title, unit, time range, refresh rate
5. Click **Save**

### Resize and reposition

Click and drag the panel's bottom-right corner to resize. Click and drag the panel header to move it. Panels snap to a grid.

### Edit or delete

Hover a panel → click the **⋮** menu → **Edit** or **Delete**.

---

## 5. Binding a Widget to a Channel

Every widget that shows data needs to know:
1. Which **device** to read from
2. Which **channel** (data key) within that device

The channels come from the device's **decoder** (Module 7). The template's decoder gives every device four channels: `temperature` (°C), `pot` (0–100 %), `button` (1/0) and `led` (1/0). If a channel is missing from the list, the decoder isn't set on that device yet.

For the command button, you also specify:
- **Command** — the command name the device handles (the template has two built in: `config:set_interval` and `config:set_led`)
- **Parameters** — the JSON payload sent with it (e.g. `{ "publish_interval_ms": 10000 }`)

---

## 6. Sharing a Dashboard

To share your dashboard with a team member or instructor:

1. Open the dashboard → **Share** (top-right)
2. Select **Read-only link** — generates a URL that shows the dashboard without login
3. Or add a team member by email under **Settings → Users** — they get full dashboard access

---

## 7. Lab — Build the Module Dashboard

Build a dashboard named `[YourName] — IoT Module` with seven panels, one for every part on your breadboard:

### Panel 1 — Line Chart (Temperature History)

| Setting | Value |
|---------|-------|
| Widget | Line Chart |
| Title | Temperature (24 h) |
| Channel | Device → channel `temperature` |
| Time range | Last 24 hours |
| Refresh | Every 30 seconds |

### Panel 2 — Value Card (Live Temperature)

| Setting | Value |
|---------|-------|
| Widget | Value Card |
| Title | Live Temperature |
| Channel | Device → channel `temperature` |
| Unit | °C |
| Threshold | Yellow > 35 °C, Red > 40 °C |

### Panel 3 — Status Indicator

| Setting | Value |
|---------|-------|
| Widget | Status Indicator |
| Title | SSN32 Status |
| Device | Your registered device |

### Panel 4 — Command Button (Reset Interval)

| Setting | Value |
|---------|-------|
| Widget | Command Button |
| Title | Set 10 s interval |
| Device | Your registered device |
| Command | `config:set_interval` |
| Parameters | `{ "publish_interval_ms": 10000 }` |

*(This command already works — the template you flashed in Module 5 handles `config:set_interval` out of the box, so clicking the button changes the publish rate right away. In Module 9 you'll drive it deliberately and watch the full ACK lifecycle.)*

### Panel 5 — Gauge (analog input)

| Setting | Value |
|---------|-------|
| Widget | Gauge |
| Title | Knob position *(or whatever is on GPIO 34: "Light level" for an LDR)* |
| Channel | Device → channel `pot` |
| Unit | % |
| Min | 0 |
| Max | 100 |
| Thresholds | Green 0–70, Yellow 70–90, Red 90–100 |

Turn the knob (or cover the LDR) and watch the needle follow at the next publish. The channel is still called `pot` whatever part is on GPIO 34; the widget title is where you say what it really is.

### Panel 6 — Value Card (Button)

| Setting | Value |
|---------|-------|
| Widget | Value Card (or Status Indicator, if it can bind to a channel) |
| Title | Button pressed? |
| Channel | Device → channel `button` |
| Threshold | Green = 0, Orange = 1 |

### Panel 7 — Command Buttons (LED On / LED Off)

Add **two** command buttons side by side:

| Setting | LED On | LED Off |
|---------|--------|---------|
| Widget | Command Button | Command Button |
| Title | LED on | LED off |
| Device | Your registered device | Your registered device |
| Command | `config:set_led` | `config:set_led` |
| Parameters | `{ "state": 1 }` | `{ "state": 0 }` |

Click **LED on**: the SSN32's on-board LED (and your external LED on GPIO 2, if wired) lights straight away, and the `led` channel shows `1` at the next publish.

### Layout

Arrange panels so the status indicator is in the top-left corner (most important), the line chart spans the top row, the temperature value card, analog gauge and button card sit side by side in the middle row, and the command buttons are at the bottom.

---

## 8. Deliverable Checklist

- [ ] Dashboard named `[YourName] — IoT Module` created
- [ ] Line chart shows at least 30 minutes of temperature history with data points visible
- [ ] Value card shows the current temperature with °C unit
- [ ] Status indicator shows **online** when device is connected
- [ ] Gauge follows your analog part (knob, trimmer or LDR); button card shows `1` after a press
- [ ] Interval command button works — clicking it changes the publish interval (the template handles it)
- [ ] LED on / LED off buttons switch the LED on the board
- [ ] Dashboard shared with instructor (read-only link or tenant access)
- [ ] You can explain what each widget type is best suited for

---

## 9. What's Next

In **[module-09-receive-commands](module-09-receive-commands.md)** you will:
- Send the built-in `config:set_interval` command from SSC and watch the publish rate change live
- Follow the command lifecycle on the platform: PENDING → SENT → ACKNOWLEDGED
- See the device's `ok` ACK — and a `failed` ACK when you send an out-of-range value
- Confirm the change survives a reboot (the interval is saved to NVS)
- All of this **without writing firmware** — the template handles the `cmd` topic for you

