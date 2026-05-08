#pragma once
#include <lvgl.h>

// Initialisiert LVGL-Display (Flush-Callback) und Touch-Input-Device.
// Muss nach lv_init() und M5Dial.begin() aufgerufen werden.
void lvgl_driver_init();
void lvgl_driver_apply_saved_rotation();
bool lvgl_driver_set_rotation_degrees(uint16_t degrees);
uint16_t lvgl_driver_get_rotation_degrees();
