
# Module 6 — MQTT & SSC Device Registration

**Goal:** Now that your device is online (Module 5), understand *how* it's talking — the MQTT messaging model underneath — so you can read the topics, register devices confidently, and diagnose connection problems on your own.

**Audience:** Completed [module-05-ssc-client](module-05-ssc-client.md).

**Estimated time:** 2 hours (mostly understanding + inspection — little new code)

**Deliverable:** You can explain the path a message takes from your device to the SSC dashboard, and you can fix the common "won't connect" problems without help.

> **Instructor note:** Module 5 was "make it work"; this module is "understand what just happened." No big new code here — the value is the mental model and the hands-on LWT/offline test in Section 6, which always gets an "oh, cool" from the room. Keep the dashboard event log projected.

---

## 1. What Is MQTT? (in plain terms)

**MQTT** is the small, efficient "texting" system that IoT devices use to talk to a server. Three roles:

```
   Publisher  ──send──>  Broker  ──deliver──>  Subscriber
   (your board)          (the SSC server)      (the dashboard)
```

- **Broker** — the central post office. Everything goes through it. SSC runs the broker.
- **Client** — anything connected to the broker. Your board is a client; so is the dashboard.
- **Topic** — the *address* a message is sent to (a text path like `.../device/.../data`).
- **Publish** — send a message to a topic. **Subscribe** — ask the broker "send me everything posted to this topic."

> **The key idea:** nobody talks to anybody *directly*. Your board publishes to a topic; the dashboard subscribes to that topic; the broker connects the two. That's why your board doesn't need to know the dashboard's address — it only needs the broker's.

### Why MQTT and not a normal web request (HTTP)?

| | HTTP (like loading a webpage) | MQTT |
|--|-------------------------------|------|
| Connection | Opens a new one for every message | Stays connected all the time |
| Message size | Big headers | Tiny header (a couple of bytes) |
| Server → device | Device must keep asking "anything for me?" | Broker pushes instantly |
| Battery / data cost | High | Low |

For a sensor that sends small readings often and needs to receive commands anytime, MQTT is the right tool.

---

## 2. Topics — the Address Your Data Travels On

SSC builds four topics for each device from your **tenant**, **application**, and device **token**. You saw these printed in Serial in Module 5:

```
tenant/<tenantId>/app/<applicationId>/device/<token>/data     ← device → SSC (sensor readings)
tenant/<tenantId>/app/<applicationId>/device/<token>/status   ← device → SSC (online / offline)
tenant/<tenantId>/app/<applicationId>/device/<token>/cmd      ← SSC → device (commands)
tenant/<tenantId>/app/<applicationId>/device/<token>/ack      ← device → SSC (I did it)
```

The library builds these strings for you inside `begin()` and subscribes to the `cmd` one so the platform can reach the device. You never type them by hand.

> **Why the tenant/app/device path?** It keeps every device's messages in their own lane. Two companies (tenants) can use the same broker and never see each other's data, because their topics differ from the very first segment.

---

## 3. Three Features That Keep "Online/Offline" Honest

These are the parts students most often see but don't understand. Here's what each is *for*.

### Birth message — "I'm here"
When the device connects, the library publishes to the **status** topic:
```json
{"state":"online","firmwareVersion":"v1.0.0"}
```
*Why:* this is what turns the dashboard indicator green, and it reports which firmware is running (used later for OTA in Module 10).

### Last Will & Testament (LWT) — "if I vanish, tell them"
A device that loses power can't send a goodbye. So at connect time the device pre-registers a message with the broker: *"if I disappear, publish this offline notice for me."*
```json
{"state":"offline","reason":"lwt"}
```
*Why:* without it, a device that lost power would show a **stale "online"** forever. With it, the broker notices the silence (via keep-alive, below) and posts the offline notice automatically — no code runs on the dead device.

### Keep-alive — "still here" (automatic)
Between readings there can be long gaps. What stops the platform from thinking the device vanished? The **MQTT keep-alive**: a tiny automatic "ping" the library sends (about every 60 s) from inside `update()`. As long as `update()` runs every loop, that ping keeps arriving and the broker keeps you marked **online**.

*You write no code for this — there is no manual heartbeat to send.* Your only job is to keep calling `sscClient.update()`; that's why a long `delay()` would silently knock you offline (the ping stops).

---

## 4. A Word on QoS (Quality of Service)

MQTT lets each message pick how hard the broker tries to deliver it:

| Level | Meaning | Where SSC uses it |
|-------|---------|-------------------|
| **QoS 0** | Send once, no guarantee | Telemetry (a lost reading 30 s ago doesn't matter — a fresh one is coming) |
| **QoS 1** | Keep trying until confirmed | The **LWT** offline notice (you really don't want to miss that) |

You don't set these yourself — the library chose sensible defaults. Just know *why* telemetry is "fire and forget" but the offline notice is "make sure it arrives."

---

## 5. Registering a Device on SSC

```
Tenant  →  Application  →  Device
(your org)  (a group)      (one board)
```

1. **Applications → New Application** — name it (e.g. `IoT Learning Module`).
2. Inside it, **Devices → Add Device** — give it a readable name (e.g. `Aek-SSN32-01`).
3. Copy the values the platform generates — **Device ID, Token, Password** — plus note your **Tenant** and **Application** names. (The token/password may be shown only once.)
4. Paste them into your `platformio.ini` build flags (exactly as in Module 5).
5. Build, upload, and watch the device appear online.

> **Why so many values?** Device ID names the connection, the token identifies the device *and* forms its topic path, the password proves it's allowed in, and tenant/app place it in the right lane. Get one wrong and you'll connect to the wrong lane — or be refused.

---

## 6. Lab — Inspect and Prove It (little new code)

Use the template you flashed in Module 5 (already online). This lab is about *seeing* the concepts work.

1. **Find the birth message.** In the dashboard event log, locate `{"state":"online",...}` and note its timestamp.
2. **The LWT test (the fun one).** Pull the USB cable **without** pressing reset. Wait up to ~90 seconds. Watch the device flip to **offline** in the dashboard — that's the broker publishing your LWT. Plug back in; watch it return online with a fresh birth message.
3. **Keep-alive check.** Leave it connected 10 minutes with no telemetry. It should stay online — the automatic MQTT keep-alive (serviced by `update()`) is doing its job.
4. **Break the credentials on purpose.** Change `DEVICE_TOKEN` to a wrong value, upload, and read the Serial output — see how a rejected connection looks (`[SSC] Connection timeout` / retry). Restore the correct token.
5. **Explain the flow.** In your own words, write the steps from power-on to "online in dashboard": boot → `setup()` → WiFi connect → MQTT connect → LWT registered → birth published → subscribed to `cmd` → `update()` servicing the keep-alive every loop.

---

## 7. Diagnosing Connection Problems

| What you see | Likely cause | Fix |
|--------------|--------------|-----|
| `[wifi] connecting....` forever | Wrong SSID/password (case-sensitive) or a 5 GHz network | Fix build flags; use 2.4 GHz |
| WiFi OK, then `[SSC] Connection timeout` | Wrong broker host/port | Check `SSC_BROKER` and port 1883 |
| Connects then drops repeatedly | Wrong token / password / tenant / app | Re-copy all IDs from the dashboard |
| Online, then offline after ~1 min | `update()` not called often (a `delay()` somewhere) | Keep `loop()` non-blocking; call `sscClient.update()` every iteration |
| No birth message in log | `begin()` didn't finish, or dashboard opened before connect | Check Serial; refresh the dashboard |

> **The habit that prevents most of these:** keep `loop()` fast and always call `sscClient.update()`. This is exactly why Module 4 banned `delay()`.

---

## 8. Deliverable Checklist

- [ ] Device registered (ID, token, password, tenant, app all in `platformio.ini`)
- [ ] Device shows **online** after upload
- [ ] You found the birth message in the event log
- [ ] LWT test done — device went offline within ~90 s of unplugging, then back online
- [ ] You can explain, in one sentence each: topic, broker, birth message, LWT, and keep-alive
- [ ] You can describe the full power-on → online sequence

---

## 9. What's Next

In **[module-07-publish-telemetry](module-07-publish-telemetry.md)** you will:
- Turn a temperature reading into the compact hex format SSC expects
- Publish it on the `data` topic every 30 seconds
- Watch a live temperature chart move in the dashboard
