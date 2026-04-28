#pragma once
#include <Arduino.h>

// Initialize BLE HID keyboard (call once in setup, after USB init)
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
