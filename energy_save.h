#pragma once
#include <Arduino.h>

enum ScreensaverMode : uint8_t {
    SS_MODE_DIM_ONLY     = 0,  // Dim display, stay dimmed
    SS_MODE_DIM_THEN_OFF = 1,  // Dim, then turn off after off_timeout_s
    SS_MODE_GIF          = 2,  // Play GIF screensaver (no off)
};

void energy_save_init();
void energy_save_update();
void energy_save_activity();

void energy_save_force_sleep();
void energy_save_wakeup();

bool energy_save_is_dimmed();
bool energy_save_is_enabled();

void energy_save_set_enabled(bool enabled);

void energy_save_set_timeout_seconds(uint32_t timeout_s);
uint32_t energy_save_get_timeout_seconds();

void energy_save_set_off_timeout_seconds(uint32_t timeout_s);
uint32_t energy_save_get_off_timeout_seconds();

void energy_save_set_dim_brightness(uint8_t brightness);
uint8_t energy_save_get_dim_brightness();

void energy_save_set_active_brightness(uint8_t brightness);
uint8_t energy_save_get_active_brightness();

ScreensaverMode energy_save_get_ss_mode();
void energy_save_set_ss_mode(ScreensaverMode mode);

bool energy_save_is_showing_gif();
void energy_save_gif_tick();
bool energy_save_has_gif();
void energy_save_notify_gif_changed();
