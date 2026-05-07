#include "hid_connection.h"
#include "hid_usb.h"
#include "logger.h"
#include <BleKeyboard.h>
#include <BLESecurity.h>
#include <Preferences.h>

static constexpr uint32_t HID_BLE_PAIRING_PIN = 415900;
static BleKeyboard s_ble_keyboard("m5Macro BLE", "m5Stack", 100);
static HidConnectionMode s_mode = HID_CONNECTION_USB;
static bool s_ble_started = false;

static HidConnectionMode read_mode_from_prefs() {
    Preferences prefs;
    prefs.begin("hid", true);
    uint8_t mode = prefs.getUChar("mode", (uint8_t)HID_CONNECTION_USB);
    prefs.end();
    return mode == (uint8_t)HID_CONNECTION_BLUETOOTH ? HID_CONNECTION_BLUETOOTH : HID_CONNECTION_USB;
}

static void write_mode_to_prefs(HidConnectionMode mode) {
    Preferences prefs;
    prefs.begin("hid", false);
    prefs.putUChar("mode", (uint8_t)mode);
    prefs.end();
}

static void ensure_ble_started() {
    if (s_ble_started) return;
    s_ble_keyboard.begin();
    BLESecurity security;
    security.setStaticPIN(HID_BLE_PAIRING_PIN);
    security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    security.setCapability(ESP_IO_CAP_OUT);
    security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    security.setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    s_ble_started = true;
    LOG_I("HID", "Bluetooth keyboard started with pairing PIN %06u", HID_BLE_PAIRING_PIN);
}

void hid_connection_init() {
    hid_usb_init();

    s_mode = read_mode_from_prefs();
    if (s_mode == HID_CONNECTION_BLUETOOTH) {
        ensure_ble_started();
    }

    LOG_I("HID", "active output: %s", hid_connection_mode_name());
}

HidConnectionMode hid_connection_mode() {
    return s_mode;
}

bool hid_connection_set_mode(HidConnectionMode mode) {
    if (mode != HID_CONNECTION_USB && mode != HID_CONNECTION_BLUETOOTH) {
        return false;
    }
    if (mode == HID_CONNECTION_BLUETOOTH) {
        ensure_ble_started();
    }
    hid_connection_release_all();
    s_mode = mode;
    write_mode_to_prefs(mode);
    LOG_I("HID", "active output changed to %s", hid_connection_mode_name());
    return true;
}

const char* hid_connection_mode_name() {
    return s_mode == HID_CONNECTION_BLUETOOTH ? "bluetooth" : "usb";
}

bool hid_connection_ready() {
    if (s_mode == HID_CONNECTION_BLUETOOTH) {
        return s_ble_started && s_ble_keyboard.isConnected();
    }
    return true;
}

bool hid_connection_ble_connected() {
    return s_ble_started && s_ble_keyboard.isConnected();
}

uint32_t hid_connection_pairing_pin() {
    return HID_BLE_PAIRING_PIN;
}

void hid_connection_print(const char* text) {
    if (!text) return;
    if (s_mode == HID_CONNECTION_BLUETOOTH) {
        if (hid_connection_ready()) s_ble_keyboard.print(text);
        return;
    }
    hid_usb_print(text);
}

void hid_connection_press(uint8_t keycode) {
    if (s_mode == HID_CONNECTION_BLUETOOTH) {
        if (hid_connection_ready()) s_ble_keyboard.press(keycode);
        return;
    }
    hid_usb_press(keycode);
}

void hid_connection_release_all() {
    if (s_ble_started) {
        s_ble_keyboard.releaseAll();
    }
    hid_usb_release_all();
}
