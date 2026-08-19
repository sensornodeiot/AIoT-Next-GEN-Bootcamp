#pragma once

// The LED blink code, as plain functions.
void ledBegin();
void ledSetInterval(unsigned long ms);
void ledUpdate();   // call every loop()
