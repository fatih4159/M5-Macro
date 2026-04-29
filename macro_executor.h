#pragma once
#include <Arduino.h>
#include "USBHIDKeyboard.h"

// Das globale Keyboard-Objekt wird in m5Macro.ino definiert.
// Alle anderen Dateien koennen es ueber dieses extern nutzen.
extern USBHIDKeyboard Keyboard;

// Fuehrt alle Schritte eines Makros aus (blockierend).
// index: Position im macro_store (0-basiert)
void macro_execute(int macro_id);

// Ausgabemodus: USB-HID, BLE oder beide
// Standardmaessig ist USB aktiv; BLE wird per Taste zugeschaltet.
void macro_set_output_usb(bool enabled);
void macro_set_output_ble(bool enabled);
