# Platform Payload Decoder

Paste this into the Sensorclouds platform decoder for the PredictiveMaintenance firmware.

## Payload Formats

All payloads publish to the same `.../data` topic. Dispatch is by byte length.

| Bytes | Description |
|---|---|
| 58 | WTVB02-485: 2-byte nonce + 14 IEEE-754 floats (vibration + health severity) |
| 54 | Power meter: 2-byte nonce + 13 IEEE-754 floats (3-phase) |
| 26 | Legacy: raw 16-bit register values (vibration) |
| 32 | Legacy: 8 raw IEEE-754 floats (power format A) |
| 27 | Legacy: mixed scaled integers (power format B) |

## Vibration param order (58 bytes)

`ax · ay · az · vx · vy · vz · temp · dx · dy · dz · hzx · hzy · hzz · severity`

## New power param order (54 bytes)

`va · vb · vc · ia · ib · ic · pa · pb · pc · pfa · pfb · pfc · freq`

## Runtime interval commands

Both publish intervals are independently configurable via MQTT command:

```json
{ "command": "config:set_interval",       "commandId": "1", "parameters": { "publish_interval_ms": 10000 } }
{ "command": "config:set_power_interval",  "commandId": "2", "parameters": { "power_publish_interval_ms": 5000 } }
```

---

```js
function decode(input) {
  let bytes;

  if (typeof input === 'string') {
    const hexString = input.replace(/\s+/g, '');
    bytes = [];
    for (let i = 0; i < hexString.length; i += 2) {
      bytes.push(parseInt(hexString.substr(i, 2), 16));
    }
  } else if (input.bytes) {
    bytes = input.bytes;
  } else {
    bytes = input;
  }

  if (bytes.length === 58) {
    return decodeVibrationHex(bytes);
  } else if (bytes.length === 54) {
    return decodePowerHex(bytes);
  } else if (bytes.length === 32) {
    return decodePowerData32(bytes);
  } else if (bytes.length === 27) {
    return decodePowerData27(bytes);
  } else if (bytes.length === 26) {
    return decodeVibrationData(bytes);
  } else {
    return { error: `Unknown payload: got ${bytes.length} bytes` };
  }
}

// ── New firmware decoders ─────────────────────────────────────────────

function decodeVibrationHex(bytes) {
  function toFloat(b, offset) {
    const buf = new ArrayBuffer(4);
    const view = new DataView(buf);
    view.setUint8(0, b[offset]);
    view.setUint8(1, b[offset + 1]);
    view.setUint8(2, b[offset + 2]);
    view.setUint8(3, b[offset + 3]);
    return view.getFloat32(0, false);
  }

  // Skip 2-byte nonce, then 14 big-endian IEEE-754 floats
  // Order: ax ay az | vx vy vz | temp | dx dy dz | hzx hzy hzz | severity
  const ax   = toFloat(bytes, 2  + 0  * 4);
  const ay   = toFloat(bytes, 2  + 1  * 4);
  const az   = toFloat(bytes, 2  + 2  * 4);
  const vx   = toFloat(bytes, 2  + 3  * 4);
  const vy   = toFloat(bytes, 2  + 4  * 4);
  const vz   = toFloat(bytes, 2  + 5  * 4);
  const temp = toFloat(bytes, 2  + 6  * 4);
  const dx   = toFloat(bytes, 2  + 7  * 4);
  const dy   = toFloat(bytes, 2  + 8  * 4);
  const dz   = toFloat(bytes, 2  + 9  * 4);
  const hzx  = toFloat(bytes, 2  + 10 * 4);
  const hzy  = toFloat(bytes, 2  + 11 * 4);
  const hzz  = toFloat(bytes, 2  + 12 * 4);
  const sev  = toFloat(bytes, 2  + 13 * 4);

  const vrms = Math.sqrt((vx * vx + vy * vy + vz * vz) / 3);
  const zone = [
    '',
    'Zone A - Good',
    'Zone B - Acceptable',
    'Zone C - Unsatisfactory',
    'Zone D - Unacceptable'
  ];

  return {
    accel_x:           Math.round(ax   * 10000) / 10000,
    accel_y:           Math.round(ay   * 10000) / 10000,
    accel_z:           Math.round(az   * 10000) / 10000,
    vel_x:             Math.round(vx   * 100)   / 100,
    vel_y:             Math.round(vy   * 100)   / 100,
    vel_z:             Math.round(vz   * 100)   / 100,
    temperature_c:     Math.round(temp * 100)   / 100,
    disp_x:            Math.round(dx),
    disp_y:            Math.round(dy),
    disp_z:            Math.round(dz),
    freq_x:            Math.round(hzx  * 10)    / 10,
    freq_y:            Math.round(hzy  * 10)    / 10,
    freq_z:            Math.round(hzz  * 10)    / 10,
    velocity_rms_mm_s: Math.round(vrms * 100)   / 100,
    severity:          Math.round(sev),
    vibration_status:  zone[Math.round(sev)] || 'Unknown'
  };
}

function decodePowerHex(bytes) {
  function toFloat(b, offset) {
    const buf = new ArrayBuffer(4);
    const view = new DataView(buf);
    view.setUint8(0, b[offset]);
    view.setUint8(1, b[offset + 1]);
    view.setUint8(2, b[offset + 2]);
    view.setUint8(3, b[offset + 3]);
    return view.getFloat32(0, false);
  }

  // Skip 2-byte nonce, then 13 big-endian IEEE-754 floats
  // Order: va vb vc | ia ib ic | pa pb pc | pfa pfb pfc | freq
  return {
    voltage_a:  Math.round(toFloat(bytes, 2  + 0  * 4) * 10)   / 10,
    voltage_b:  Math.round(toFloat(bytes, 2  + 1  * 4) * 10)   / 10,
    voltage_c:  Math.round(toFloat(bytes, 2  + 2  * 4) * 10)   / 10,
    current_a:  Math.round(toFloat(bytes, 2  + 3  * 4) * 1000) / 1000,
    current_b:  Math.round(toFloat(bytes, 2  + 4  * 4) * 1000) / 1000,
    current_c:  Math.round(toFloat(bytes, 2  + 5  * 4) * 1000) / 1000,
    power_a:    Math.round(toFloat(bytes, 2  + 6  * 4) * 10)   / 10,
    power_b:    Math.round(toFloat(bytes, 2  + 7  * 4) * 10)   / 10,
    power_c:    Math.round(toFloat(bytes, 2  + 8  * 4) * 10)   / 10,
    pf_a:       Math.round(toFloat(bytes, 2  + 9  * 4) * 1000) / 1000,
    pf_b:       Math.round(toFloat(bytes, 2  + 10 * 4) * 1000) / 1000,
    pf_c:       Math.round(toFloat(bytes, 2  + 11 * 4) * 1000) / 1000,
    frequency:  Math.round(toFloat(bytes, 2  + 12 * 4) * 100)  / 100
  };
}

// ── Legacy decoders ───────────────────────────────────────────────────

function decodeVibrationData(bytes) {
  function toSigned16(val) {
    return val > 0x7FFF ? val - 0x10000 : val;
  }

  const accelXRaw = (bytes[0] << 8) | bytes[1];
  const accelX = (toSigned16(accelXRaw) / 32768) * 16;

  const accelYRaw = (bytes[2] << 8) | bytes[3];
  const accelY = (toSigned16(accelYRaw) / 32768) * 16;

  const accelZRaw = (bytes[4] << 8) | bytes[5];
  const accelZ = (toSigned16(accelZRaw) / 32768) * 16;

  const velX = toSigned16((bytes[6]  << 8) | bytes[7])  / 100;
  const velY = toSigned16((bytes[8]  << 8) | bytes[9])  / 100;
  const velZ = toSigned16((bytes[10] << 8) | bytes[11]) / 100;

  const dispX = toSigned16((bytes[12] << 8) | bytes[13]);
  const dispY = toSigned16((bytes[14] << 8) | bytes[15]);
  const dispZ = toSigned16((bytes[16] << 8) | bytes[17]);

  const freqX = ((bytes[18] << 8) | bytes[19]) / 10;
  const freqY = ((bytes[20] << 8) | bytes[21]) / 10;
  const freqZ = ((bytes[22] << 8) | bytes[23]) / 10;

  const temp = toSigned16((bytes[24] << 8) | bytes[25]) / 100;

  const v = assessVibration(velX, velY, velZ);

  return {
    accel_x:           Math.round(accelX * 10000) / 10000,
    accel_y:           Math.round(accelY * 10000) / 10000,
    accel_z:           Math.round(accelZ * 10000) / 10000,
    vel_x:             Math.round(velX   * 100)   / 100,
    vel_y:             Math.round(velY   * 100)   / 100,
    vel_z:             Math.round(velZ   * 100)   / 100,
    disp_x:            dispX,
    disp_y:            dispY,
    disp_z:            dispZ,
    freq_x:            Math.round(freqX  * 10)    / 10,
    freq_y:            Math.round(freqY  * 10)    / 10,
    freq_z:            Math.round(freqZ  * 10)    / 10,
    temperature_c:     Math.round(temp   * 100)   / 100,
    velocity_rms_mm_s: v.velocityRMS
  };
}

function decodePowerData32(bytes) {
  function toFloat32(b, offset) {
    const buf = new ArrayBuffer(4);
    const view = new DataView(buf);
    view.setUint8(0, b[offset]);
    view.setUint8(1, b[offset + 1]);
    view.setUint8(2, b[offset + 2]);
    view.setUint8(3, b[offset + 3]);
    return view.getFloat32(0, false);
  }

  return {
    voltage:        Math.round(toFloat32(bytes, 0)  * 10)   / 10,
    current:        Math.round(toFloat32(bytes, 4)  * 1000) / 1000,
    active_power:   Math.round(toFloat32(bytes, 8)  * 1000) / 1000,
    reactive_power: Math.round(toFloat32(bytes, 12) * 1000) / 1000,
    apparent_power: Math.round(toFloat32(bytes, 16) * 1000) / 1000,
    power_factor:   Math.round(toFloat32(bytes, 20) * 1000) / 1000,
    frequency:      Math.round(toFloat32(bytes, 24) * 100)  / 100,
    total_energy:   Math.round(toFloat32(bytes, 28) * 100)  / 100
  };
}

function decodePowerData27(bytes) {
  const voltage = ((bytes[0] << 8) | bytes[1]) / 100;
  const current = ((bytes[2] << 8) | bytes[3]) / 1000;

  const activePowerRaw = (bytes[4] << 24) | (bytes[5] << 16) | (bytes[6] << 8) | bytes[7];
  const activePower = (activePowerRaw > 0x7FFFFFFF ? activePowerRaw - 0x100000000 : activePowerRaw) / 10 / 1000;

  const reactivePowerRaw = (bytes[8] << 8) | bytes[9];
  const reactivePower = (reactivePowerRaw > 0x7FFF ? reactivePowerRaw - 0x10000 : reactivePowerRaw) / 10 / 1000;

  const apparentPower = ((bytes[10] << 8) | bytes[11]) / 10 / 1000;
  const powerFactor   = bytes[12] / 100;
  const frequency     = ((bytes[13] << 8) | bytes[14]) / 100;
  const totalEnergy   = ((bytes[15] << 24) | (bytes[16] << 16) | (bytes[17] << 8) | bytes[18]) / 100;
  const monthlyEnergy = ((bytes[19] << 24) | (bytes[20] << 16) | (bytes[21] << 8) | bytes[22]) / 100;
  const uptime        = (bytes[23] << 24) | (bytes[24] << 16) | (bytes[25] << 8) | bytes[26];

  const billResult = calculateElectricityBill(monthlyEnergy);

  return {
    voltage:        Math.round(voltage        * 10)   / 10,
    current:        Math.round(current        * 1000) / 1000,
    active_power:   Math.round(activePower    * 1000) / 1000,
    reactive_power: Math.round(reactivePower  * 1000) / 1000,
    apparent_power: Math.round(apparentPower  * 1000) / 1000,
    power_factor:   Math.round(powerFactor    * 1000) / 1000,
    frequency:      Math.round(frequency      * 100)  / 100,
    total_energy:   Math.round(totalEnergy    * 100)  / 100,
    monthly_energy: Math.round(monthlyEnergy  * 100)  / 100,
    uptime:         uptime,
    bill_amount:    billResult.totalBill
  };
}

function assessVibration(vX, vY, vZ) {
  const velocityRMS = Math.sqrt((vX * vX + vY * vY + vZ * vZ) / 3);
  return { velocityRMS: Math.round(velocityRMS * 100) / 100 };
}

function calculateElectricityBill(kWh) {
  const tariff = [
    { units: 25,       rate: 563  },
    { units: 125,      rate: 648  },
    { units: 150,      rate: 1147 },
    { units: 100,      rate: 1410 },
    { units: 100,      rate: 1519 },
    { units: 500,      rate: 1564 },
    { units: 500,      rate: 1628 },
    { units: Infinity, rate: 1696 }
  ];

  let totalBill = 0;
  let remaining = kWh;

  for (let i = 0; i < tariff.length && remaining > 0; i++) {
    const unitsInTier = Math.min(remaining, tariff[i].units);
    totalBill += unitsInTier * tariff[i].rate;
    remaining -= unitsInTier;
  }

  return { totalBill: Math.round(totalBill) };
}
```
