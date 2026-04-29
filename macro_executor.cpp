#include "macro_executor.h"
#include "macro_store.h"
#include "macro_parser.h"
#include "config.h"
#include "ble_keyboard_hid.h"

static bool s_usb_enabled = true;
static bool s_ble_output  = false;

void macro_set_output_usb(bool enabled) { s_usb_enabled = enabled; }
void macro_set_output_ble(bool enabled) { s_ble_output  = enabled; }

// ── Execute single step ──────────────────────────────────────────────────────
static void executeStep(const MacroStep& s) {
    switch (s.type) {

        case STEP_TEXT:
            if (s_usb_enabled) Keyboard.print(s.text);
            if (s_ble_output)  ble_keyboard_print(s.text);
            delay(STEP_GAP_MS);
            break;

        case STEP_DELAY:
            if (s.delay_ms > 0) delay(s.delay_ms);
            break;

        case STEP_KEY:
            if (s.keycode != 0) {
                if (s_usb_enabled) Keyboard.press(s.keycode);
                if (s_ble_output)  ble_keyboard_press(s.keycode);
                delay(KEY_HOLD_MS);
                if (s_usb_enabled) Keyboard.releaseAll();
                if (s_ble_output)  ble_keyboard_release_all();
                delay(STEP_GAP_MS);
            }
            break;

        case STEP_COMBO:
            for (int i = 0; i < s.mod_count; i++) {
                if (s_usb_enabled) Keyboard.press(s.modifiers[i]);
                if (s_ble_output)  ble_keyboard_press(s.modifiers[i]);
            }
            if (s.keycode != 0) {
                if (s_usb_enabled) Keyboard.press(s.keycode);
                if (s_ble_output)  ble_keyboard_press(s.keycode);
            }
            delay(KEY_HOLD_MS);
            if (s_usb_enabled) Keyboard.releaseAll();
            if (s_ble_output)  ble_keyboard_release_all();
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

    if (s_usb_enabled) Keyboard.releaseAll();
    if (s_ble_output)  ble_keyboard_release_all();
}
