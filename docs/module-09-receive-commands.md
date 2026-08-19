
# Module 9 — Control the Device from the Cloud

**Goal:** Send a command *down* from the SSC dashboard and have the device change its behaviour live (no reboot), then confirm it. The template already handles this — you'll drive it from the platform and understand the flow.

**Audience:** Completed [module-08-ssc-dashboard](module-08-ssc-dashboard.md). Your device is online and publishing.

**Estimated time:** 1–1.5 hours.

**Deliverable:** Changing the publish interval from the SSC dashboard changes how often the device sends; the ACK appears in the event log; the device never reboots and remembers the new value.

---

## 1. The full two-way loop

Until now, data only went **up**. Commands complete the picture:

```
Device ──[ telemetry ]──> SSC          (Module 7)
Device <──[ command  ]── SSC dashboard (this module — an operator clicks a button)
Device ──[   ACK     ]──> SSC          (device confirms it did the thing)
```

This is what makes IoT *controllable*: retune a device without touching the hardware.

---

## 2. The template already receives commands

When the template connects, it subscribes to your device's **command** topic. When the platform sends a command there, `SSCApp` routes it to a handler, applies it, saves it to flash (so it survives a reboot), and replies with an **ACK**. **You write no firmware for this** — it's built in.

The template ships one ready command:

### `config:set_interval` — change how often the device publishes

| Field | Value |
|-------|-------|
| Command | `config:set_interval` |
| Parameters | `{ "publish_interval_ms": 30000 }` |

- **Bounds:** 1000 ms – 3600000 ms (1 s to 1 hour). Out-of-range values are rejected with a `failed` ACK.
- The new interval takes effect on the next loop tick — **no reboot** — and is saved to NVS, so it persists.

---

## 3. Lab — change the interval from the cloud

1. ▶ With the device online (from Module 7), open SSC → your device → **Send Command**.
2. ▶ Send command **`config:set_interval`** with parameters **`{ "publish_interval_ms": 30000 }`**.
3. 👁 On Serial you'll see the device accept it; the `TX` lines now appear every **30 s** instead of every 10 s.
4. 👁 In the event log, an ACK appears:
   ```json
   { "commandId": "...", "command": "config:set_interval", "status": "ok" }
   ```
5. ▶ Send an out-of-range value, e.g. `{ "publish_interval_ms": 50 }`. 👁 The device replies `status: "failed"` and keeps its previous interval.
6. ▶ Reboot the board (press RST). 👁 It comes back publishing at **30 s** — the change was saved.

---

## 4. How the ACK lifecycle looks on the platform

Every command moves through: **PENDING → SENT → ACKNOWLEDGED** (or **FAILED / TIMEOUT**). You'll see this in the command history — it's how an operator knows the device actually applied the change, not just that the message was sent.

---

## 5. Optional — adding your own command

The command handling lives in the template's `lib/config_cmd_handler/`. A new command is a new `command:` name plus a small handler that reads its parameters and applies them (the same pattern as `set_interval`). This is a "go further" exercise — the built-in `config:set_interval` is all Module 9 requires.

> **Under the hood:** if you want to see how a command is parsed from raw MQTT bytes by hand (`setCommandCallback`, `deserializeJson`, a self-built ACK), that's in [appendix-under-the-hood-ssc-client](appendix-under-the-hood-ssc-client.md) and the low-level code in [modules-5-10-step-by-step](modules-5-10-step-by-step.md).

---

## 6. Deliverable Checklist

- [ ] Sent `config:set_interval` from SSC; the device's publish rate changed live
- [ ] The ACK (`status: ok`) appeared in the event log
- [ ] An out-of-range value was rejected with `failed`
- [ ] The new interval survived a reboot (saved to flash)
- [ ] The device never rebooted to apply the change
- [ ] You can describe the PENDING → SENT → ACKNOWLEDGED lifecycle

---

## 7. What's Next

**[module-10-ota](module-10-ota.md)** (bonus) — update the device's firmware over the air, also built into the template.
