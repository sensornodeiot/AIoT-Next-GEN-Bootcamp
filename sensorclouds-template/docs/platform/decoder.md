# Sensorclouds Payload Decoder — Workshop Template

Paste the script below into the **Device Profile → Payload Formatter** in Sensorclouds (a CUSTOM
codec). One copy serves every device on that profile. It turns the hex string the board sends
into named values you can put on a dashboard.

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

It uses the `decodeUplink(input)` format that Sensorclouds recommends: the platform turns the
36-character hex string into `input.bytes` (an array of numbers) before calling it. Plain ES5
JavaScript, no `const` or typed arrays, so it runs in any decoder sandbox.

```js
// Sensorclouds decoder: AIoT Next-GEN Bootcamp workshop template
// Payload: 2-byte random nonce, then one big-endian float32 per name.
// Add a value with r.add("name", ...) in sensor.cpp? Add the same name
// at the END of NAMES.
var NAMES = ["temperature", "pot", "button", "led"];

function f32(b, i) {
  var bits = ((b[i] << 24) >>> 0) + (b[i + 1] << 16) + (b[i + 2] << 8) + b[i + 3];
  var sign = (bits >>> 31) ? -1 : 1;
  var exp = (bits >>> 23) & 0xFF;
  var frac = bits & 0x7FFFFF;
  if (exp === 0xFF) return frac ? NaN : sign * Infinity;
  if (exp === 0) return sign * frac * Math.pow(2, -149);
  return sign * (1 + frac / 8388608) * Math.pow(2, exp - 127);
}

function decodeUplink(input) {
  var b = input.bytes;
  var expected = 2 + NAMES.length * 4;
  if (b.length !== expected) {
    return { errors: ["Expected " + expected + " bytes, got " + b.length +
                      ": NAMES must match the r.add() calls in sensor.cpp"] };
  }
  var data = {};
  for (var k = 0; k < NAMES.length; k++) {
    var v = f32(b, 2 + k * 4);
    if (!isNaN(v)) data[NAMES[k]] = Math.round(v * 100) / 100;  // NaN = no sensor
  }
  return { data: data };
}
```

## Test it before you save

The Payload Formatter has a **Test** box. Paste one of these and check the result:

| Test payload | Expected result |
|---|---|
| `9c2e41ca8000422ab3333f80000000000000` | `{ "temperature": 25.31, "pot": 42.67, "button": 1, "led": 0 }` |
| `51077fc00000000000000000000000000000` | `{ "pot": 0, "button": 0, "led": 0 }` (no DS18B20 connected) |
| `9c2e41ca8000` | an `errors` entry saying 18 bytes were expected |

Or copy a real payload from the device's Serial monitor (the `[SSC] Telemetry: ...` line) or from the
platform's event log.

## Adding your own value

1. In `lib/sensor/sensor.cpp`, add `r.add("humidity", h);` **after** the existing four.
2. In the decoder, add `"humidity"` at the **end** of `NAMES`.
3. Save the decoder, upload the firmware. The new field appears on the next publish.

If the names and the `r.add()` calls don't match, every value after the mismatch lands in the
wrong field, so always add to the end of both lists.

## Commands the template understands

Send these from **Sensorclouds → device → Send Command**, typing the name exactly. Sensorclouds has
no built-in command list: names the template doesn't know get no ACK and end as TIMEOUT after
5 minutes. The device answers each command below with an ACK on the `.../ack` topic.

| Command | Parameters | What it does |
|---|---|---|
| `config:set_led` | `{ "state": 1 }` or `{ "state": 0 }` | Turns the LED (GPIO 2) on or off. The new state shows in the next publish |
| `config:set_interval` | `{ "publish_interval_ms": 30000 }` | Changes how often the device publishes (1000–3600000 ms). Saved, survives a reboot |
| `ota:update` | *(from the platform's OTA page)* | Downloads and installs new firmware (Module 10) |

Commands are not telemetry, so they need no decoder changes.
