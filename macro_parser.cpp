#include "macro_parser.h"

// ── Keycode lookup table ─────────────────────────────────────────────────────
// Maps keyword strings to USB HID keycodes.
// Source: USBHIDKeyboard.h (ESP32 Arduino Core)

struct KeyEntry { const char* name; uint8_t code; };

// Modifier key or regular key?
struct ModEntry { const char* name; uint8_t code; };

static const ModEntry kModifiers[] = {
    { "CTRL",   0x80 },   // KEY_LEFT_CTRL
    { "SHIFT",  0x81 },   // KEY_LEFT_SHIFT
    { "ALT",    0x82 },   // KEY_LEFT_ALT
    { "WIN",    0x83 },   // KEY_LEFT_GUI
    { "GUI",    0x83 },
    { "LCTRL",  0x80 },
    { "LSHIFT", 0x81 },
    { "LALT",   0x82 },
    { "RCTRL",  0x84 },   // KEY_RIGHT_CTRL
    { "RSHIFT", 0x85 },
    { "RALT",   0x86 },
    { "RWIN",   0x87 },
};

static const KeyEntry kKeys[] = {
    // Control
    { "ENTER",     0xB0 },   // KEY_RETURN
    { "RETURN",    0xB0 },
    { "ESC",       0xB1 },   // KEY_ESC
    { "ESCAPE",    0xB1 },
    { "BACKSPACE", 0xB2 },
    { "TAB",       0xB3 },   // KEY_TAB
    { "SPACE",     0x20 },
    { "CAPSLOCK",  0xC1 },   // KEY_CAPS_LOCK
    // Navigation
    { "UP",        0xDA },   // KEY_UP_ARROW
    { "DOWN",      0xD9 },   // KEY_DOWN_ARROW
    { "RIGHT",     0xD7 },   // KEY_RIGHT_ARROW
    { "LEFT",      0xD8 },   // KEY_LEFT_ARROW
    { "HOME",      0xD2 },   // KEY_HOME
    { "END",       0xD5 },   // KEY_END
    { "PGUP",      0xD3 },   // KEY_PAGE_UP
    { "PGDN",      0xD6 },   // KEY_PAGE_DOWN
    { "PAGEUP",    0xD3 },
    { "PAGEDOWN",  0xD6 },
    { "INSERT",    0xD1 },
    { "DELETE",    0xD4 },
    // Function keys
    { "F1",        0xC2 },
    { "F2",        0xC3 },
    { "F3",        0xC4 },
    { "F4",        0xC5 },
    { "F5",        0xC6 },
    { "F6",        0xC7 },
    { "F7",        0xC8 },
    { "F8",        0xC9 },
    { "F9",        0xCA },
    { "F10",       0xCB },
    { "F11",       0xCC },
    { "F12",       0xCD },
    // Special keys
    { "PRINTSCREEN", 0xCE },
    { "SCROLLLOCK",  0xCF },
    { "PAUSE",       0xD0 },
    { "NUMLOCK",     0xDB },
    { "BACKTICK",    0x60 }, // `
    { "TILDE",       0x60 },
};

// ── Helper: token → modifier? ───────────────────────────────────────────────
// Returns 0 if no modifier found
static uint8_t lookupModifier(const String& tok) {
    String upper = tok;
    upper.toUpperCase();
    for (auto& m : kModifiers) {
        if (upper == m.name) return m.code;
    }
    return 0;
}

// ── Helper: token → keycode ─────────────────────────────────────────────────
static uint8_t lookupKey(const String& tok) {
    String upper = tok;
    upper.toUpperCase();
    for (auto& k : kKeys) {
        if (upper == k.name) return k.code;
    }
    // Single ASCII character (a-z, 0-9, punctuation)
    if (tok.length() == 1) {
        char c = tok.charAt(0);
        // Lowercase for USB HID
        if (c >= 'A' && c <= 'Z') c += 32;
        return (uint8_t)c;
    }
    return 0;
}

// ── Main function: parse line ────────────────────────────────────────────────
MacroStep parseLine(const String& line) {
    MacroStep step = {};
    step.type = STEP_DELAY; // Default: empty step (delay_ms=0 → ignored)

    String s = line;
    s.trim();

    // Ignore empty line or comment
    if (s.length() == 0 || s.charAt(0) == '#') return step;

    // ── TEXT: ──────────────────────────────────────────────────────────────
    if (s.startsWith("TEXT:")) {
        step.type = STEP_TEXT;
        String txt = s.substring(5);
        txt.toCharArray(step.text, STEP_TEXT_LEN);
        return step;
    }

    // ── DELAY: ────────────────────────────────────────────────────────────
    if (s.startsWith("DELAY:")) {
        step.type = STEP_DELAY;
        step.delay_ms = (uint32_t)s.substring(6).toInt();
        return step;
    }

    // ── Key combination (TOKEN+TOKEN+...) ──────────────────────────────────
    // Split tokens on '+'
    String tokens[8];
    int    n_tok = 0;
    int    start = 0;
    for (int i = 0; i <= (int)s.length(); i++) {
        if (i == (int)s.length() || s.charAt(i) == '+') {
            if (n_tok < 8) tokens[n_tok++] = s.substring(start, i);
            start = i + 1;
        }
    }

    // Last token is the main key, all preceding tokens are modifiers
    step.mod_count = 0;
    for (int i = 0; i < n_tok - 1 && step.mod_count < STEP_MOD_MAX; i++) {
        uint8_t mod = lookupModifier(tokens[i]);
        if (mod) step.modifiers[step.mod_count++] = mod;
    }

    step.keycode = lookupKey(tokens[n_tok - 1]);

    step.type = (step.mod_count > 0) ? STEP_COMBO : STEP_KEY;
    return step;
}
