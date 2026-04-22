#include "macro_executor.h"
#include "macro_store.h"
#include "macro_parser.h"
#include "config.h"

// ── Einzelnen Schritt ausfuehren ─────────────────────────────────────────────
static void executeStep(const MacroStep& s) {
    switch (s.type) {

        case STEP_TEXT:
            Keyboard.print(s.text);
            delay(STEP_GAP_MS);
            break;

        case STEP_DELAY:
            if (s.delay_ms > 0) delay(s.delay_ms);
            break;

        case STEP_KEY:
            if (s.keycode != 0) {
                Keyboard.press(s.keycode);
                delay(KEY_HOLD_MS);
                Keyboard.releaseAll();
                delay(STEP_GAP_MS);
            }
            break;

        case STEP_COMBO:
            // Modifier zuerst druecken
            for (int i = 0; i < s.mod_count; i++) {
                Keyboard.press(s.modifiers[i]);
            }
            if (s.keycode != 0) Keyboard.press(s.keycode);
            delay(KEY_HOLD_MS);
            Keyboard.releaseAll();
            delay(STEP_GAP_MS);
            break;
    }
}

// ── Makro ausfuehren ─────────────────────────────────────────────────────────
void macro_execute(int index) {
    const MacroInfo* info = macro_store_get(index);
    if (!info) return;

    for (int i = 0; i < info->step_count; i++) {
        MacroStep step = parseLine(String(info->steps[i]));
        executeStep(step);
    }

    Keyboard.releaseAll(); // Sicherheits-Release am Ende
}
