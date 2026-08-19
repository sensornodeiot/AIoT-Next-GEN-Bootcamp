#include <Arduino.h>
#include "sensors.h"
#include "led.h"

unsigned long lastTempPrint = 0;
unsigned long lastUptime    = 0;

// TODO: run three independent timed tasks in loop(), none blocking the others:
//  - ledUpdate()     every iteration (LED_PIN blink, 300 ms — set via ledSetInterval)
//  - sensorsUpdate() every iteration; print getTemperature() every 2 s
//  - uptime print    every 5 s
// main.cpp should read like a description of the system: no pin numbers, no sensor internals.

void setup() {
  Serial.begin(115200);
  sensorsBegin();
  ledBegin();
  ledSetInterval(300);
  Serial.println("Module 4 ready.");
}

void loop() {
  unsigned long now = millis();

    ledUpdate();       // blinks at 300 ms internally
    sensorsUpdate();   // reads temperature every 2 s internally

    if (now - lastTempPrint >= 2000) {          // print temp every 2 s
        lastTempPrint = now;
        Serial.printf("[%lums] temp: %.2f C\n", now, getTemperature());
    }

    if (now - lastUptime >= 5000) {             // print uptime every 5 s
        lastUptime = now;
        Serial.printf("[%lums] uptime: %lus\n", now, now / 1000);
    }
}
