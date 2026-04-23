#pragma once
#include <Arduino.h>

// Energy-saving mode: dims the display after configurable inactivity.
// Rotating the encoder or pressing a button restores normal brightness.

void energy_save_init();      // Load settings from NVS, initialize state
void energy_save_activity();  // Signal user activity (resets timer)
void energy_save_update();    // Call once per loop() – checks timeout
bool energy_save_is_dimmed(); // true while display is dimmed
