#pragma once
#include <Arduino.h>

enum HidConnectionMode : uint8_t {
    HID_CONNECTION_USB = 0,
    HID_CONNECTION_BLUETOOTH = 1,
};

void hid_connection_init();
HidConnectionMode hid_connection_mode();
bool hid_connection_set_mode(HidConnectionMode mode);
const char* hid_connection_mode_name();
bool hid_connection_ready();
bool hid_connection_ble_connected();
uint32_t hid_connection_pairing_pin();

void hid_connection_print(const char* text);
void hid_connection_press(uint8_t keycode);
void hid_connection_release_all();
