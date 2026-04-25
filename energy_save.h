#pragma once
#include <Arduino.h>

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

void energy_save_set_dim_brightness(uint8_t brightness);
uint8_t energy_save_get_dim_brightness();

void energy_save_set_active_brightness(uint8_t brightness);
uint8_t energy_save_get_active_brightness();

bool energy_save_screensaver_gif_mode();
void energy_save_set_screensaver_gif_mode(bool enabled);
bool energy_save_is_showing_gif();
void energy_save_gif_tick();
bool energy_save_has_gif();
void energy_save_notify_gif_changed();