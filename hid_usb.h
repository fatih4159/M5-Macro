#pragma once
#include <Arduino.h>

void hid_usb_init();
void hid_usb_print(const char* text);
void hid_usb_press(uint8_t keycode);
void hid_usb_release_all();
