# Module 2 — GPIO: Output & Input

**Goal:** Control and read physical hardware from code.

## Topics

- What is a GPIO pin; voltage levels (3.3 V on SSN32)
- `pinMode()`, `digitalWrite()`, `digitalRead()`
- Ohm's law basics — why a resistor is needed for an LED
- `delay()` — what it does and why it blocks the CPU
- Non-blocking blink with `millis()` — the correct pattern for firmware
- `pinMode(pin, INPUT_PULLUP)` — internal pull-up resistors
- Why buttons bounce and how to debounce in software
- Combining input and output in one loop

## Lab

Wire an LED to GPIO 2 and a push button to GPIO 4 (not GPIO 0, that is the BOOT button).

- LED blinks every 500 ms using `millis()` (no `delay()`)
- Each button press toggles the blink rate between 500 ms and 100 ms
- Debounce the button in code
- Print a message to Serial on every confirmed press

## Deliverable

LED blinks independently. Button reliably changes blink rate with no false triggers. Serial confirms each press.
