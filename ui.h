#pragma once
#include <lvgl.h>

// Creates the LVGL interface for the macro selector.
// Must be called after macro_store_init().
void ui_init();

// Sets the display state to "macro is running".
// Forces an LVGL render pass (lv_timer_handler).
void ui_show_running(int index);

// Resets the display state to "ready".
void ui_show_idle();

// Returns the currently selected index in the roller.
int  ui_get_selected();

// Sets the roller index programmatically (e.g. after reload).
void ui_set_selected(int index);

// Rebuilds the roller content from the current macro_store.
// Call after every macro change via the web editor.
void ui_reload();

// Sets the hint text in the status line (e.g. WiFi address).
void ui_set_hint(const char* text);
