#include <Arduino.h>
// Define LED_BUILTIN if the board framework does not provide it

// TODO:
// - Blink LED_BUILTIN using millis() at a rate that toggles between 500ms and 100ms
// - Read BUTTON_PIN (INPUT_PULLUP) with software debounce
// - Each confirmed press toggles the blink rate and prints to Serial
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