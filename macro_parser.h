#pragma once
#include <Arduino.h>
#include "config.h"

// ── Step types ──────────────────────────────────────────────────────────────
enum StepType : uint8_t {
    STEP_KEY,    // single key (no modifier)
    STEP_COMBO,  // modifier + key (e.g. CTRL+C)
    STEP_TEXT,   // type text (Keyboard.print)
    STEP_DELAY,  // delay in ms
};

// ── Single macro step ────────────────────────────────────────────────────────
struct MacroStep {
    StepType type;
    uint8_t  modifiers[STEP_MOD_MAX]; // modifier keycodes (KEY_LEFT_CTRL etc.)
    uint8_t  mod_count;               // number of active modifiers
    uint8_t  keycode;                 // main keycode
    char     text[STEP_TEXT_LEN];     // only for STEP_TEXT
    uint32_t delay_ms;                // only for STEP_DELAY
};

// ── Public function ──────────────────────────────────────────────────────────
// Parses a single line from a macro file.
// Comment lines (#) and empty lines produce STEP_DELAY with delay_ms=0
// (ignored by the executor).
MacroStep parseLine(const String& line);
