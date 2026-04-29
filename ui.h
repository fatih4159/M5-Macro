#pragma once
#include <lvgl.h>

// Creates the LVGL interface for the macro selector.
// Must be called after macro_store_init().
void ui_init();

// Sets the display state to "macro is running".
// Forces an LVGL render pass (lv_timer_handler).
void ui_show_running(int macro_id);

// Resets the display state to "ready".
void ui_show_idle();

// Returns the currently selected index in the roller.
int  ui_get_selected();

// Returns the number of items currently visible in the active roller.
int  ui_get_visible_count();

// Sets the roller index programmatically (e.g. after reload).
void ui_set_selected(int index);

// Activates the current selection.
// Returns true when a macro should be executed and writes its macro id.
bool ui_activate_selected(int* macro_id);

// Rebuilds the roller content from the current macro_store.
// Call after every macro change via the web editor.
void ui_reload();

// Reloads colors from NVS and applies them to the display immediately.
void ui_apply_colors();

