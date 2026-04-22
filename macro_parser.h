#pragma once
#include <Arduino.h>
#include "config.h"

// ── Schritt-Typen ───────────────────────────────────────────────────────────
enum StepType : uint8_t {
    STEP_KEY,    // einzelne Taste (kein Modifier)
    STEP_COMBO,  // Modifier + Taste (z.B. CTRL+C)
    STEP_TEXT,   // Text tippen (Keyboard.print)
    STEP_DELAY,  // Verzoegerung in ms
};

// ── Einzelner Makro-Schritt ─────────────────────────────────────────────────
struct MacroStep {
    StepType type;
    uint8_t  modifiers[STEP_MOD_MAX]; // Modifier-Keycodes (KEY_LEFT_CTRL etc.)
    uint8_t  mod_count;               // Anzahl aktiver Modifier
    uint8_t  keycode;                 // Haupt-Keycode
    char     text[STEP_TEXT_LEN];     // Nur fuer STEP_TEXT
    uint32_t delay_ms;                // Nur fuer STEP_DELAY
};

// ── Oeffentliche Funktion ───────────────────────────────────────────────────
// Parst eine einzelne Zeile aus einer Makro-Datei.
// Kommentarzeilen (#) und leere Zeilen ergeben STEP_DELAY mit delay_ms=0
// (werden vom Executor ignoriert).
MacroStep parseLine(const String& line);
