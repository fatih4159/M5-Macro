#include "macro_executor.h"
#include "macro_store.h"
#include "macro_parser.h"
#include "config.h"
#include "hid_connection.h"

// ── Execute single step ──────────────────────────────────────────────────────
static void executeStep(const MacroStep& s) {
    switch (s.type) {

        case STEP_TEXT:
            hid_connection_print(s.text);
            delay(STEP_GAP_MS);
            break;

        case STEP_DELAY:
            if (s.delay_ms > 0) delay(s.delay_ms);
            break;

        case STEP_KEY:
            if (s.keycode != 0) {
                hid_connection_press(s.keycode);
                delay(KEY_HOLD_MS);
                hid_connection_release_all();
                delay(STEP_GAP_MS);
            }
            break;

        case STEP_COMBO:
            for (int i = 0; i < s.mod_count; i++) {
                hid_connection_press(s.modifiers[i]);
            }
            if (s.keycode != 0) {
                hid_connection_press(s.keycode);
            }
            delay(KEY_HOLD_MS);
            hid_connection_release_all();
            delay(STEP_GAP_MS);
            break;
    }
}

// ── Execute macro ────────────────────────────────────────────────────────────
void macro_execute(int macro_id) {
    const MacroInfo* info = macro_store_get_macro_by_id(macro_id);
    if (!info) return;

    for (int i = 0; i < info->step_count; i++) {
        MacroStep step = parseLine(String(info->steps[i]));
        executeStep(step);
    }

    hid_connection_release_all();
}
