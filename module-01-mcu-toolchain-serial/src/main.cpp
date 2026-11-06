#include <Arduino.h>

// TODO: print "[<ms>ms] Hello from SSN32 — uptime: <s>s" once per second, using millis() (no delay()).
unsigned long lastPrint = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("SSN32 ready.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastPrint >= 1000) {
    lastPrint = now;
    unsigned long uptimeSec = now / 1000;
    Serial.printf("[%lums] Hello from SSN32 - uptime: %lus\n", now, uptimeSec);
  }
}
