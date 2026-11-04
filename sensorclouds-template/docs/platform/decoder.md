# Sensorclouds Payload Decoder — Workshop Template

Paste the script below into your device's **decoder** in Sensorclouds. It turns the hex
string the SSN32 sends into named values you can put on a dashboard.

## What the device sends

Every publish interval (10 s by default) the template publishes one hex string to the
`.../data` topic. It always holds the same four values, in this order:

| Bytes | Hex chars | Name | Meaning |
|---|---|---|---|
| 0–1 | 1–4 | *(nonce)* | Random number, ignored |
| 2–5 | 5–12 | `temperature` | DS18B20 temperature in °C. Left out when no sensor is connected |
| 6–9 | 13–20 | `pot` | Potentiometer (or LDR) position, 0–100 % |
| 10–13 | 21–28 | `button` | `1` = pressed (now or since the last publish), `0` = not pressed |
| 14–17 | 29–36 | `led` | `1` = LED on, `0` = off |

Each value is a 4-byte IEEE-754 float, most significant byte first (big-endian).
Total: **18 bytes = 36 hex characters**.

Example payload and what the decoder returns:

```
Payload: 9c2e41ca8000422ab3333f80000000000000
Decoded: { "temperature": 25.31, "pot": 42.67, "button": 1, "led": 0 }
```

## The decoder script

```js
// Sensorclouds decoder: AIoT Next-GEN Bootcamp workshop template
//
// The device sends 4 hex chars of random nonce, then one big-endian
// IEEE-754 float (8 hex chars) per value, in this order.
// If you add a value with r.add("name", ...) in lib/sensor/sensor.cpp,
// add the same name here, at the same position.
const NAMES = ["temperature", "pot", "button", "led"];

function decode(input) {
  const bytes = toBytes(input);
  if (!bytes) {
    return { error: "Payload is not valid hex" };
  }

  const expected = 2 + NAMES.length * 4;
  if (bytes.length !== expected) {
    return { error: "Expected " + expected + " bytes, got " + bytes.length +
                    ". Check that NAMES matches the r.add() calls in sensor.cpp" };
  }

  const view = new DataView(new Uint8Array(bytes).buffer);
  const result = {};

  NAMES.forEach(function (name, i) {
    const value = view.getFloat32(2 + i * 4, false);
    // NaN means "no reading" (e.g. DS18B20 not connected): leave the field out
    if (!isNaN(value)) {
      result[name] = Math.round(value * 100) / 100;
    }
  });

  return result;
}

// Accepts a hex string, { bytes: [...] } or a plain byte array
function toBytes(input) {
  if (typeof input === "string") {
    const hex = input.replace(/\s+/g, "");
    if (!/^[0-9a-fA-F]*$/.test(hex) || hex.length % 2 !== 0) {
      return null;
    }
    const bytes = [];
    for (let i = 0; i < hex.length; i += 2) {
      bytes.push(parseInt(hex.substr(i, 2), 16));
    }
    return bytes;
  }
  if (input && input.bytes) {
    return Array.from(input.bytes);
  }
  if (Array.isArray(input)) {
    return input;
  }
  return null;
}
```

## Test it before you save

Most decoder editors have a **Test** box. Paste one of these and check the result:

| Test payload | Expected result |
|---|---|
| `9c2e41ca8000422ab3333f80000000000000` | `{ "temperature": 25.31, "pot": 42.67, "button": 1, "led": 0 }` |
| `51077fc00000000000000000000000000000` | `{ "pot": 0, "button": 0, "led": 0 }` (no DS18B20 connected) |
| `9c2e41ca8000` | an `error` saying 18 bytes were expected |

Or copy a real payload from the device's Serial monitor (the `[SSC] Telemetry: ...` line) or from the
platform's event log.

## Adding your own value

1. In `lib/sensor/sensor.cpp`, add `r.add("humidity", h);` **after** the existing four.
2. In the decoder, add `"humidity"` at the **end** of `NAMES`.
3. Save the decoder, upload the firmware. The new field appears on the next publish.

If the names and the `r.add()` calls don't match, every value after the mismatch lands in the
wrong field, so always add to the end of both lists.

## Commands the template understands

Send these from **Sensorclouds → device → Send Command**. The device answers each one with an
ACK on the `.../ack` topic.

| Command | Parameters | What it does |
|---|---|---|
| `config:set_led` | `{ "state": 1 }` or `{ "state": 0 }` | Turns the LED (GPIO 2) on or off. The new state shows in the next publish |
| `config:set_interval` | `{ "publish_interval_ms": 30000 }` | Changes how often the device publishes (1000–3600000 ms). Saved, survives a reboot |
| `ota:update` | *(from the platform's OTA page)* | Downloads and installs new firmware (Module 10) |

Commands are not telemetry, so they need no decoder changes.
