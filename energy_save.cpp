#include "energy_save.h"
#include "config.h"
#include <M5Dial.h>
#include <Preferences.h>

static bool     s_enabled        = true;
static uint32_t s_timeout_ms     = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
static uint8_t  s_dim_brightness = ENERGY_SAVE_DIM_BRIGHTNESS;
static uint8_t  s_active_brightness = ENERGY_SAVE_ACTIVE_BRIGHTNESS;
static uint32_t s_last_activity  = 0;
static bool     s_dimmed         = false;

void energy_save_init() {
    Preferences prefs;
    prefs.begin("energy", true);
    s_enabled           = prefs.getBool("enabled", true);
    s_timeout_ms        = (uint32_t)prefs.getUInt("timeout_s", ENERGY_SAVE_TIMEOUT_DEFAULT) * 1000UL;
    s_dim_brightness    = (uint8_t)prefs.getUInt("dim_br",    ENERGY_SAVE_DIM_BRIGHTNESS);
    s_active_brightness = (uint8_t)prefs.getUInt("active_br", ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    prefs.end();

    s_last_activity = millis();
    s_dimmed        = false;
    M5Dial.Display.setBrightness(s_active_brightness);
}

void energy_save_activity() {
    if (s_dimmed) {
        s_dimmed = false;
        M5Dial.Display.setBrightness(s_active_brightness);
    }
    s_last_activity = millis();
}

void energy_save_update() {
    if (!s_enabled || s_dimmed) return;
    if (millis() - s_last_activity >= s_timeout_ms) {
        s_dimmed = true;
        M5Dial.Display.setBrightness(s_dim_brightness);
    }
}

bool energy_save_is_dimmed() {
    return s_dimmed;
}
