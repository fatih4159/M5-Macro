#include "ble_keyboard_hid.h"
#include <BleKeyboard.h>

static BleKeyboard s_ble_keyboard("m5Macro", "m5Stack", 100);
static bool        s_enabled = false;

void ble_keyboard_init() {
    // Initialize BLE stack once at boot, before USB starts, to prevent
    // BLEDevice::init() from interfering with active TinyUSB interrupt traffic.
    // enable/disable only control output routing, not the stack lifecycle.
    s_ble_keyboard.begin();
}

void ble_keyboard_enable() {
    if (s_enabled) return;
    s_enabled = true;
}

void ble_keyboard_disable() {
    if (!s_enabled) return;
    s_enabled = false;
}

bool ble_keyboard_enabled() {
    return s_enabled;
}

bool ble_keyboard_connected() {
    return s_enabled && s_ble_keyboard.isConnected();
}

void ble_keyboard_print(const char* text) {
    if (!ble_keyboard_connected()) return;
    s_ble_keyboard.print(text);
}

void ble_keyboard_press(uint8_t key) {
    if (!ble_keyboard_connected()) return;
    s_ble_keyboard.press(key);
}

void ble_keyboard_release_all() {
    if (!ble_keyboard_connected()) return;
    s_ble_keyboard.releaseAll();
}
