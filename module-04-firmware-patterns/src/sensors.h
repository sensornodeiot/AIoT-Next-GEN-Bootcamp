#pragma once  // include this header only once

// The DS18B20 code, organised as plain functions (no class needed).
// The .h is the "menu": the short list of what this file offers to the rest of the program.

void  sensorsBegin();   // set up the sensor — call once in setup()
void  sensorsUpdate();  // call every loop() — reads the sensor without blocking
float getTemperature(); // returns the most recent temperature in degrees C
