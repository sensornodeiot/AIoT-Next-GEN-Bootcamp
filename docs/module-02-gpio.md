
# Module 2 — GPIO: Output & Input

**Goal:** Control and read physical hardware from code — blink an LED and read a push button reliably.

**Audience:** Completed [module-01-mcu-toolchain-serial](module-01-mcu-toolchain-serial.md).

**Estimated time:** 2–3 hours (reading + lab)

**Deliverable:** LED blinks using `millis()` (no `delay()`). Button press toggles blink rate with debouncing. Serial confirms each press.

---

## 1. What Is a GPIO Pin?

**GPIO** stands for General Purpose Input/Output. Each GPIO pin on the SSN32 is a physical connection point that you can configure in firmware to either:

- **OUTPUT** — drive the pin HIGH (3.3 V) or LOW (0 V) to power something
- **INPUT** — read whether external circuitry is pulling the pin HIGH or LOW

GPIO pins are how the MCU connects to the real world: they drive LEDs, control relay coils, read button states, and communicate with sensors.

### Voltage levels on the SSN32

Every GPIO pin operates at **3.3 V logic**. HIGH = 3.3 V, LOW = 0 V. Never connect a 5 V signal to a GPIO pin without level shifting — it will damage the chip.

---

## 2. Controlling an LED (Output)

### Ohm's law and the current-limiting resistor

An LED will draw as much current as the circuit allows. Without a resistor, it pulls far more than the pin can source (~12 mA GPIO limit on ESP32) and will burn out — along with the GPIO driver.

Use a **220 Ω resistor** in series with the LED:

```
GPIO 2 ──[220Ω]──[LED anode → cathode]──> GND
```

Ohm's law: `V = I × R`  
`3.3 V / 220 Ω ≈ 15 mA` — within the LED's rated range and well within the pin limit.

### pinMode and digitalWrite

```cpp
pinMode(2, OUTPUT);      // configure GPIO 2 as output
digitalWrite(2, HIGH);  // set pin HIGH (LED on)
digitalWrite(2, LOW);   // set pin LOW  (LED off)
```

Call `pinMode()` in `setup()`. Call `digitalWrite()` any time you want to change the state.

---

## 3. The Problem With delay()

The natural first instinct for blinking an LED:

```cpp
void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}
```

This works, but it is **fatally wrong for real firmware**. During both `delay(500)` calls, the MCU sits completely frozen — it cannot read a sensor, check a button, communicate over WiFi, or do anything at all. The entire program blocks.

Rule: **never use `delay()` in production firmware**. You will not use it after this module.

---

## 4. Non-Blocking Blink with millis()

The correct pattern uses `millis()` to check elapsed time without blocking:

```cpp
const unsigned long BLINK_INTERVAL = 500;  // ms
unsigned long lastToggle = 0;
bool ledState = false;

void loop() {
    unsigned long now = millis();

    if (now - lastToggle >= BLINK_INTERVAL) {
        lastToggle = now;
        ledState = !ledState;            // flip: false→true, true→false
        digitalWrite(LED_PIN, ledState); // true = HIGH (on), false = LOW (off)
    }

    // everything else here runs at full speed, unblocked
}
```

### How it works

1. `millis()` returns milliseconds since boot — it counts up continuously, never blocking anything
2. `now - lastToggle` is the elapsed time since the last toggle
3. When that exceeds the interval, act and reset `lastToggle`
4. The rest of `loop()` — reading buttons, sensors, etc. — executes every iteration unhindered

> **What is `ledState = !ledState`?** The `!` means **"not"** — it flips a true/false value to its opposite. So each pass swaps `ledState` between `true` and `false`, and the LED goes on, off, on, off… And because `digitalWrite` reads `true` as HIGH (on) and `false` as LOW (off), you can hand it `ledState` directly — no need to spell out `HIGH`/`LOW` yourself.

This is the **fundamental firmware timing pattern**. Every module from here on uses it.

---

## 5. Reading a Button (Input)

### INPUT_PULLUP

When a GPIO is configured as a plain `INPUT` and nothing is connected, the pin floats — it reads random values. The solution is to enable the **internal pull-up resistor**:

```cpp
pinMode(BUTTON_PIN, INPUT_PULLUP);
```

With `INPUT_PULLUP`:
- The pin is held HIGH internally when no button is pressed
- When you press the button and connect the pin to GND, it reads LOW

Wire your button between the GPIO pin and GND (no external resistor needed):

```
GPIO 0 ──[button]──> GND
```

Reading the button:

```cpp
int state = digitalRead(BUTTON_PIN);
// state == HIGH: button released
// state == LOW:  button pressed
```

Note the inverted logic — **LOW means pressed** when using `INPUT_PULLUP`.

---

## 6. Button Bouncing and Simple Cooldown

Mechanical buttons bounce — when you press one, the contacts make and break dozens of times in the first 5–20 ms. Without handling this, one physical press can register as many presses in code.

The simplest fix: after accepting a press, **ignore the button for the next 200 ms**. One variable is all it takes:

```cpp
unsigned long lastPressTime = 0;

int raw = digitalRead(BUTTON_PIN);
if (raw == LOW && now - lastPressTime >= 200) {
    lastPressTime = now;
    // confirmed press — will not fire again for 200 ms
}
```

This cooldown approach works well for lab bench buttons. Module 4 covers the full software debounce technique — which handles any input signal and also detects release events — as part of its firmware patterns coverage.

---

## 7. Combining Input and Output

Multiple independent tasks run in one `loop()` — each with its own timestamp variable, neither blocking the other:

```cpp
unsigned long lastToggle    = 0;
unsigned long blinkInterval = 500;
unsigned long lastPressTime = 0;
bool          ledState      = false;

void loop() {
    unsigned long now = millis();

    // --- LED blink task ---
    if (now - lastToggle >= blinkInterval) {
        lastToggle = now;
        ledState = !ledState;             // flip on/off
        digitalWrite(LED_PIN, ledState);
    }

    // --- Button task ---
    int raw = digitalRead(BUTTON_PIN);
    if (raw == LOW && now - lastPressTime >= 200) {
        lastPressTime = now;
        // toggle the blink rate between 500 ms and 100 ms
        if (blinkInterval == 500) {
            blinkInterval = 100;
        } else {
            blinkInterval = 500;
        }
        Serial.printf("[%lums] press — interval now %lums\n", now, blinkInterval);
    }
}
```

Both tasks run independently. Neither blocks the other.

---

## 8. Lab — LED Blink + Button Toggle

### Hardware

| Component | Connection |
|-----------|-----------|
| LED (anode) | GPIO 2 via 220 Ω resistor |
| LED (cathode) | GND |
| Button (pin 1) | GPIO 0 |
| Button (pin 2) | GND |

### Complete code

```cpp
#include <Arduino.h>

#define LED_PIN    2
#define BUTTON_PIN 0

unsigned long lastToggle    = 0;
unsigned long blinkInterval = 500;
unsigned long lastPressTime = 0;
bool          ledState      = false;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Module 2 ready.");
}

void loop() {
    unsigned long now = millis();

    // LED blink
    if (now - lastToggle >= blinkInterval) {
        lastToggle = now;
        ledState = !ledState;             // flip on/off
        digitalWrite(LED_PIN, ledState);
    }

    // Button press with cooldown
    int raw = digitalRead(BUTTON_PIN);
    if (raw == LOW && now - lastPressTime >= 200) {
        lastPressTime = now;
        // toggle the blink rate between 500 ms and 100 ms
        if (blinkInterval == 500) {
            blinkInterval = 100;
        } else {
            blinkInterval = 500;
        }
        Serial.printf("[%lums] press — interval now %lums\n", now, blinkInterval);
    }
}
```

### Build and upload

1. `Ctrl+Alt+U` to upload
2. `Ctrl+Alt+S` to open Serial Monitor at 115200
3. Press RST on the SSN32

### Expected behaviour

- LED blinks at 500 ms by default
- Each button press toggles between 500 ms and 100 ms
- Serial prints one confirmation line per confirmed press (not per bounce)

### Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| LED stays on/off | Wrong pin or missing resistor | Check wiring and `LED_PIN` constant |
| Multiple prints per press | Cooldown too short | Increase from `200` to `300` ms |
| LED doesn't blink (just on) | `lastToggle` not updating | Ensure `lastToggle = now;` is inside the `if` |
| Button does nothing | `INPUT_PULLUP` not set | Check `pinMode(BUTTON_PIN, INPUT_PULLUP)` |

---

## 9. Deliverable Checklist

- [x] LED blinks at 500 ms on power-up (no `delay()` in code)
- [x] Button press toggles blink rate between 500 ms and 100 ms
- [x] No repeated triggers — one physical press = one Serial print
- [x] You can explain why `delay()` is a problem and how `millis()` solves it
- [x] You can explain what button bouncing is and why a cooldown timer prevents false triggers

---

## 10. What's Next

In **[module-03-sensors](module-03-sensors.md)** you will:
- Read analog signals with the ADC (`analogRead`)
- Wire and read the DS18B20 temperature sensor using the 1-Wire protocol
- Run two independent sensor reads in one `loop()` without blocking
