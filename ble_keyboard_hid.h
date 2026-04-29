#pragma once
#include <Arduino.h>

// Prepare BLE HID keyboard state. Does not start BLE advertising by itself.
void ble_keyboard_init();

// Enable / disable BLE keyboard and advertising
void ble_keyboard_enable();
void ble_keyboard_disable();
bool ble_keyboard_enabled();
bool ble_keyboard_connected();

// Keyboard output – mirrors USBHIDKeyboard API
void ble_keyboard_print(const char* text);
void ble_keyboard_press(uint8_t key);
void ble_keyboard_release_all();
