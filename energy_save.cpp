#include "energy_save.h"
#include "config.h"
#include <M5Dial.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <lvgl.h>
#include <AnimatedGIF.h>

static const char* GIF_PATH = "/screensaver.gif";

// ── AnimatedGIF decoder ───────────────────────────────────────────────────────
static AnimatedGIF s_gif_decoder;
static bool        s_gif_init_done = false;
static File        s_gif_fh;
static uint16_t    s_gif_line[240];

static void* gif_open_cb(const char* path, int32_t* size) {
    s_gif_fh = LittleFS.open(path, "r");
    if (!s_gif_fh) return nullptr;
    *size = (int32_t)s_gif_fh.size();
    return (void*)&s_gif_fh;
}
static void    gif_close_cb(void*)                                { if (s_gif_fh) s_gif_fh.close(); }
static int32_t gif_read_cb (GIFFILE* , uint8_t* buf, int32_t len) { return (int32_t)s_gif_fh.read(buf, len); }
static int32_t gif_seek_cb (GIFFILE* , int32_t pos)               { s_gif_fh.seek(pos); return pos; }

static void gif_draw_cb(GIFDRAW* pDraw) {
    uint16_t* pal    = pDraw->pPalette;
    uint8_t*  pixels = pDraw->pPixels;
    int       w      = (pDraw->iWidth > 240) ? 240 : pDraw->iWidth;
    for (int i = 0; i < w; i++) s_gif_line[i] = pal[pixels[i]];
    M5Dial.Display.pushImage(pDraw->iX, pDraw->iY + pDraw->y, w, 1, s_gif_line);
}

// GIF playback state (used from the main loop on core 1)
static bool     s_showing_gif     = false;
static bool     s_gif_file_open   = false;
static uint32_t s_gif_next_frame  = 0;

namespace {
  constexpr const char* PREF_NS = "energy";
  constexpr const char* KEY_ENABLED = "enabled";
  constexpr const char* KEY_TIMEOUT = "timeout_s";
  constexpr const char* KEY_DIM_BR = "dim_br";
  constexpr const char* KEY_ACTIVE_BR = "active_br";
  constexpr const char* KEY_SS_GIF = "ss_gif";

  bool s_enabled = true;
  uint32_t s_timeout_ms = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
  uint8_t s_dim_brightness = ENERGY_SAVE_DIM_BRIGHTNESS;
  uint8_t s_active_brightness = ENERGY_SAVE_ACTIVE_BRIGHTNESS;
  uint32_t s_last_activity = 0;
  bool s_dimmed = false;
  bool s_ss_gif_mode = false;

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
    prefs.putBool(KEY_SS_GIF, s_ss_gif_mode);
    prefs.end();
  }

  void gif_close_if_open() {
    if (s_gif_file_open) {
      s_gif_decoder.close();
      s_gif_file_open = false;
    }
  }

  void apply_active_state() {
    gif_close_if_open();
    s_showing_gif = false;
    s_dimmed = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
    lv_obj_invalidate(lv_scr_act());
  }

  void apply_dimmed_state() {
    s_dimmed = true;
    if (s_ss_gif_mode && LittleFS.exists(GIF_PATH)) {
      M5Dial.Display.wakeup();
      M5Dial.Display.setBrightness(s_active_brightness);
      // Clear the LVGL frame so the GIF renders on a blank screen
      M5Dial.Display.fillScreen(0);
      gif_close_if_open();
      s_gif_next_frame = millis();
      s_showing_gif = true;
    } else {
      s_showing_gif = false;
      M5Dial.Display.setBrightness(s_dim_brightness);
      M5Dial.Display.sleep();
    }
  }
}

void energy_save_init() {
  gif_close_if_open();
  s_showing_gif = false;

  Preferences prefs;
  if (prefs.begin(PREF_NS, true)) {
    s_enabled = prefs.getBool(KEY_ENABLED, true);
    s_timeout_ms = (uint32_t)prefs.getUInt(KEY_TIMEOUT, ENERGY_SAVE_TIMEOUT_DEFAULT) * 1000UL;
    s_dim_brightness = (uint8_t)prefs.getUInt(KEY_DIM_BR, ENERGY_SAVE_DIM_BRIGHTNESS);
    s_active_brightness = (uint8_t)prefs.getUInt(KEY_ACTIVE_BR, ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    s_ss_gif_mode = prefs.getBool(KEY_SS_GIF, false);
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

void energy_save_gif_tick() {
  if (!s_showing_gif) return;

  uint32_t now = millis();

  if (!s_gif_file_open) {
    if (!LittleFS.exists(GIF_PATH)) return;
    if (!s_gif_init_done) {
      s_gif_decoder.begin();
      s_gif_init_done = true;
    }
    if (!s_gif_decoder.open(GIF_PATH, gif_open_cb, gif_close_cb,
                             gif_read_cb, gif_seek_cb, gif_draw_cb)) {
      return;
    }
    s_gif_file_open = true;
    s_gif_next_frame = now;
  }

  // Not yet time for the next frame
  if ((int32_t)(now - s_gif_next_frame) < 0) return;

  int delay_ms = 0;
  if (s_gif_decoder.playFrame(false, &delay_ms)) {
    s_gif_next_frame = now + (uint32_t)(delay_ms > 0 ? delay_ms : 10);
  } else {
    // End of animation – close and let it re-open on the next tick (loop)
    s_gif_decoder.close();
    s_gif_file_open = false;
    s_gif_next_frame = now;
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

bool energy_save_screensaver_gif_mode() {
  return s_ss_gif_mode;
}

void energy_save_set_screensaver_gif_mode(bool enabled) {
  s_ss_gif_mode = enabled;
  save_preferences();
}

bool energy_save_is_showing_gif() {
  return s_showing_gif;
}

bool energy_save_has_gif() {
  return LittleFS.exists(GIF_PATH);
}

void energy_save_notify_gif_changed() {
  if (s_showing_gif && !LittleFS.exists(GIF_PATH)) {
    gif_close_if_open();
    s_showing_gif = false;
    M5Dial.Display.setBrightness(s_dim_brightness);
    M5Dial.Display.sleep();
  }
}
