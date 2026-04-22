#pragma once
#include <Arduino.h>

// Energiesparmodus: dimmt das Display nach konfigurierbarer Inaktivitaet.
// Encoder-Drehen oder Tastendruck reaktiviert die normale Helligkeit.

void energy_save_init();      // Einstellungen aus NVS laden, Zustand initialisieren
void energy_save_activity();  // Benutzeraktivitaet signalisieren (setzt Timer zurueck)
void energy_save_update();    // Einmal pro loop() aufrufen – prueft Timeout
bool energy_save_is_dimmed(); // true waehrend Display gedimmt ist
