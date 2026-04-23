#include "energy_save.h"
#include "config.h"
#include <M5Dial.h>
#include <Preferences.h>

namespace {
  constexpr const char* PREF_NS = "energy";
  constexpr const char* KEY_ENABLED = "enabled";
  constexpr const char* KEY_TIMEOUT = "timeout_s";
  constexpr const char* KEY_DIM_BR = "dim_br";
  constexpr const char* KEY_ACTIVE_BR = "active_br";

  bool s_enabled = true;
  uint32_t s_timeout_ms = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
  uint8_t s_dim_brightness = ENERGY_SAVE_DIM_BRIGHTNESS;
  uint8_t s_active_brightness = ENERGY_SAVE_ACTIVE_BRIGHTNESS;
  uint32_t s_last_activity = 0;
  bool s_dimmed = false;

  uint8_t clamp_brightness(uint8_t value) {
    if (value > 255) return 255;
    return value;
  }

  void save_preferences() {
    Preferences prefs;
    if (!prefs.begin(PREF_NS, false)) {
      return;
    }

    prefs.putBool(KEY_ENABLED, s_enabled);
    prefs.putUInt(KEY_TIMEOUT, s_timeout_ms / 1000UL);
    prefs.putUInt(KEY_DIM_BR, s_dim_brightness);
    prefs.putUInt(KEY_ACTIVE_BR, s_active_brightness);
    prefs.end();
  }

  void apply_active_state() {
    s_dimmed = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
  }

  void apply_dimmed_state() {
    s_dimmed = true;
    M5Dial.Display.setBrightness(s_dim_brightness);
    M5Dial.Display.sleep();
  }
}

void energy_save_init() {
  Preferences prefs;
  if (prefs.begin(PREF_NS, true)) {
    s_enabled = prefs.getBool(KEY_ENABLED, true);
    s_timeout_ms = (uint32_t)prefs.getUInt(KEY_TIMEOUT, ENERGY_SAVE_TIMEOUT_DEFAULT) * 1000UL;
    s_dim_brightness = (uint8_t)prefs.getUInt(KEY_DIM_BR, ENERGY_SAVE_DIM_BRIGHTNESS);
    s_active_brightness = (uint8_t)prefs.getUInt(KEY_ACTIVE_BR, ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    prefs.end();
  }

  s_dim_brightness = clamp_brightness(s_dim_brightness);
  s_active_brightness = clamp_brightness(s_active_brightness);

  if (s_timeout_ms == 0) {
    s_timeout_ms = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
  }

  s_last_activity = millis();

  if (s_enabled) {
    apply_active_state();
  } else {
    s_dimmed = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
  }
}

void energy_save_update() {
  if (!s_enabled || s_dimmed) {
    return;
  }

  const uint32_t now = millis();
  if ((uint32_t)(now - s_last_activity) >= s_timeout_ms) {
    apply_dimmed_state();
  }
}

void energy_save_activity() {
  s_last_activity = millis();

  if (!s_enabled) {
    return;
  }

  if (s_dimmed) {
    apply_active_state();
  }
}

void energy_save_force_sleep() {
  if (!s_enabled) {
    return;
  }

  apply_dimmed_state();
}

void energy_save_wakeup() {
  s_last_activity = millis();

  if (!s_enabled) {
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
    s_dimmed = false;
    return;
  }

  apply_active_state();
}

bool energy_save_is_dimmed() {
  return s_dimmed;
}

bool energy_save_is_enabled() {
  return s_enabled;
}

void energy_save_set_enabled(bool enabled) {
  s_enabled = enabled;

  if (!s_enabled) {
    s_dimmed = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
  } else {
    s_last_activity = millis();
    apply_active_state();
  }

  save_preferences();
}

void energy_save_set_timeout_seconds(uint32_t timeout_s) {
  if (timeout_s == 0) {
    timeout_s = ENERGY_SAVE_TIMEOUT_DEFAULT;
  }

  s_timeout_ms = timeout_s * 1000UL;
  s_last_activity = millis();
  save_preferences();
}

uint32_t energy_save_get_timeout_seconds() {
  return s_timeout_ms / 1000UL;
}

void energy_save_set_dim_brightness(uint8_t brightness) {
  s_dim_brightness = clamp_brightness(brightness);

  if (s_dimmed) {
    M5Dial.Display.setBrightness(s_dim_brightness);
  }

  save_preferences();
}

uint8_t energy_save_get_dim_brightness() {
  return s_dim_brightness;
}

void energy_save_set_active_brightness(uint8_t brightness) {
  s_active_brightness = clamp_brightness(brightness);

  if (!s_dimmed) {
    M5Dial.Display.setBrightness(s_active_brightness);
  }

  save_preferences();
}

uint8_t energy_save_get_active_brightness() {
  return s_active_brightness;
}