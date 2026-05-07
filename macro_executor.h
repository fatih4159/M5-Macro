#pragma once
#include <Arduino.h>

// Fuehrt alle Schritte eines Makros aus (blockierend).
// index: Position im macro_store (0-basiert)
void macro_execute(int macro_id);

