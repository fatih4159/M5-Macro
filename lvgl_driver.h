#pragma once
#include <lvgl.h>

// Initialisiert LVGL-Display (Flush-Callback) und Touch-Input-Device.
// Muss nach lv_init() und M5Dial.begin() aufgerufen werden.
void lvgl_driver_init();
