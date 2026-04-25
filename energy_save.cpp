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

// Line buffer sized for full display width (destination after scaling)
static uint16_t    s_gif_line[240];

// Scale/offset computed once per GIF open
static float   s_gif_scale = 1.0f;
static int     s_gif_off_x = 0;
static int     s_gif_off_y = 0;

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
    int       src_w  = pDraw->iWidth;
    int       src_y  = pDraw->iY + pDraw->y;

    if (s_gif_scale == 1.0f && s_gif_off_x == 0 && s_gif_off_y == 0) {
        // Fast path: no scaling needed
        int w = (src_w > 240) ? 240 : src_w;
        for (int i = 0; i < w; i++) s_gif_line[i] = pal[pixels[i]];
        M5Dial.Display.pushImage(pDraw->iX, src_y, w, 1, s_gif_line);
        return;
    }

    // Scaled path: nearest-neighbour, maintain aspect ratio
    int dst_x = s_gif_off_x + (int)(pDraw->iX * s_gif_scale);
    int dst_w = (int)(src_w * s_gif_scale + 0.5f);
    if (dst_w < 1) dst_w = 1;
    if (dst_x < 0) { dst_w += dst_x; dst_x = 0; }
    if (dst_x + dst_w > 240) dst_w = 240 - dst_x;
    if (dst_w <= 0) return;

    // Build the scaled scanline
    for (int dx = 0; dx < dst_w; dx++) {
        int sx = (int)(dx / s_gif_scale);
        if (sx >= src_w) sx = src_w - 1;
        s_gif_line[dx] = pal[pixels[sx]];
    }

    // Push the scanline for every destination row this source row maps to
    int dst_y_lo = s_gif_off_y + (int)(src_y * s_gif_scale);
    int dst_y_hi = s_gif_off_y + (int)((src_y + 1) * s_gif_scale);
    if (dst_y_hi <= dst_y_lo) dst_y_hi = dst_y_lo + 1;
    for (int dy = dst_y_lo; dy < dst_y_hi; dy++) {
        if (dy < 0 || dy >= 240) continue;
        M5Dial.Display.pushImage(dst_x, dy, dst_w, 1, s_gif_line);
    }
}

// GIF playback state (used from the main loop on core 1)
static bool     s_showing_gif    = false;
static bool     s_gif_file_open  = false;
static uint32_t s_gif_next_frame = 0;

namespace {
  constexpr const char* PREF_NS       = "energy";
  constexpr const char* KEY_ENABLED   = "enabled";
  constexpr const char* KEY_TIMEOUT   = "timeout_s";
  constexpr const char* KEY_OFF_TO    = "off_timeout";
  constexpr const char* KEY_DIM_BR    = "dim_br";
  constexpr const char* KEY_ACTIVE_BR = "active_br";
  constexpr const char* KEY_SS_MODE   = "ss_mode";

  bool            s_enabled        = true;
  uint32_t        s_dim_timeout_ms = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
  uint32_t        s_off_timeout_ms = 0;   // 0 = never turn off
  uint8_t         s_dim_brightness = ENERGY_SAVE_DIM_BRIGHTNESS;
  uint8_t         s_active_brightness = ENERGY_SAVE_ACTIVE_BRIGHTNESS;
  uint32_t        s_last_activity  = 0;
  uint32_t        s_dim_start_ms   = 0;
  bool            s_dimmed         = false;
  bool            s_display_off    = false;
  ScreensaverMode s_ss_mode        = SS_MODE_DIM_ONLY;

  uint8_t clamp_brightness(uint8_t v) { return v; }

  void save_preferences() {
    Preferences prefs;
    if (!prefs.begin(PREF_NS, false)) return;
    prefs.putBool (KEY_ENABLED,   s_enabled);
    prefs.putUInt (KEY_TIMEOUT,   s_dim_timeout_ms / 1000UL);
    prefs.putUInt (KEY_OFF_TO,    s_off_timeout_ms / 1000UL);
    prefs.putUInt (KEY_DIM_BR,    s_dim_brightness);
    prefs.putUInt (KEY_ACTIVE_BR, s_active_brightness);
    prefs.putUInt (KEY_SS_MODE,   (uint32_t)s_ss_mode);
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
    s_showing_gif  = false;
    s_dimmed       = false;
    s_display_off  = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
    lv_obj_invalidate(lv_scr_act());
  }

  void apply_off_state() {
    gif_close_if_open();
    s_showing_gif = false;
    s_display_off = true;
    M5Dial.Display.setBrightness(0);
    M5Dial.Display.sleep();
  }

  void apply_dimmed_state() {
    s_dimmed      = true;
    s_display_off = false;
    s_dim_start_ms = millis();

    if (s_ss_mode == SS_MODE_GIF && LittleFS.exists(GIF_PATH)) {
      M5Dial.Display.wakeup();
      M5Dial.Display.setBrightness(s_active_brightness);
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
    s_enabled         = prefs.getBool(KEY_ENABLED, true);
    s_dim_timeout_ms  = (uint32_t)prefs.getUInt(KEY_TIMEOUT, ENERGY_SAVE_TIMEOUT_DEFAULT) * 1000UL;
    s_off_timeout_ms  = (uint32_t)prefs.getUInt(KEY_OFF_TO,  0) * 1000UL;
    s_dim_brightness  = (uint8_t)prefs.getUInt(KEY_DIM_BR,    ENERGY_SAVE_DIM_BRIGHTNESS);
    s_active_brightness = (uint8_t)prefs.getUInt(KEY_ACTIVE_BR, ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    s_ss_mode         = (ScreensaverMode)constrain((int)prefs.getUInt(KEY_SS_MODE, 0), 0, 2);
    prefs.end();
  }

  if (s_dim_timeout_ms == 0)
    s_dim_timeout_ms = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;

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
  if (!s_enabled) return;

  const uint32_t now = millis();

  if (!s_dimmed) {
    if ((uint32_t)(now - s_last_activity) >= s_dim_timeout_ms) {
      apply_dimmed_state();
    }
  } else if (s_ss_mode == SS_MODE_DIM_THEN_OFF && !s_display_off && s_off_timeout_ms > 0) {
    if ((uint32_t)(now - s_dim_start_ms) >= s_off_timeout_ms) {
      apply_off_state();
    }
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
    // Compute scale to fit 240×240 while preserving aspect ratio
    int cw = s_gif_decoder.getCanvasWidth();
    int ch = s_gif_decoder.getCanvasHeight();
    if (cw > 0 && ch > 0) {
      float sx = 240.0f / cw;
      float sy = 240.0f / ch;
      s_gif_scale = (sx < sy) ? sx : sy;
      s_gif_off_x = (240 - (int)(cw * s_gif_scale + 0.5f)) / 2;
      s_gif_off_y = (240 - (int)(ch * s_gif_scale + 0.5f)) / 2;
    } else {
      s_gif_scale = 1.0f;
      s_gif_off_x = 0;
      s_gif_off_y = 0;
    }
    // Clear letterbox bars before first frame
    M5Dial.Display.fillScreen(0);
    s_gif_file_open  = true;
    s_gif_next_frame = now;
  }

  if ((int32_t)(now - s_gif_next_frame) < 0) return;

  int delay_ms = 0;
  if (s_gif_decoder.playFrame(false, &delay_ms)) {
    s_gif_next_frame = now + (uint32_t)(delay_ms > 0 ? delay_ms : 10);
  } else {
    // End of animation – close and loop
    s_gif_decoder.close();
    s_gif_file_open = false;
    s_gif_next_frame = now;
  }
}

void energy_save_activity() {
  s_last_activity = millis();
  if (!s_enabled) return;
  if (s_dimmed) apply_active_state();
}

void energy_save_force_sleep() {
  if (!s_enabled) return;
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

bool energy_save_is_dimmed()  { return s_dimmed; }
bool energy_save_is_enabled() { return s_enabled; }

void energy_save_set_enabled(bool enabled) {
  s_enabled = enabled;
  if (!s_enabled) {
    s_dimmed = false;
    gif_close_if_open();
    s_showing_gif = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
  } else {
    s_last_activity = millis();
    apply_active_state();
  }
  save_preferences();
}

void energy_save_set_timeout_seconds(uint32_t timeout_s) {
  if (timeout_s == 0) timeout_s = ENERGY_SAVE_TIMEOUT_DEFAULT;
  s_dim_timeout_ms = timeout_s * 1000UL;
  s_last_activity  = millis();
  save_preferences();
}
uint32_t energy_save_get_timeout_seconds() { return s_dim_timeout_ms / 1000UL; }

void energy_save_set_off_timeout_seconds(uint32_t timeout_s) {
  s_off_timeout_ms = timeout_s * 1000UL;
  save_preferences();
}
uint32_t energy_save_get_off_timeout_seconds() { return s_off_timeout_ms / 1000UL; }

void energy_save_set_dim_brightness(uint8_t brightness) {
  s_dim_brightness = clamp_brightness(brightness);
  if (s_dimmed && !s_showing_gif && !s_display_off)
    M5Dial.Display.setBrightness(s_dim_brightness);
  save_preferences();
}
uint8_t energy_save_get_dim_brightness() { return s_dim_brightness; }

void energy_save_set_active_brightness(uint8_t brightness) {
  s_active_brightness = clamp_brightness(brightness);
  if (!s_dimmed) M5Dial.Display.setBrightness(s_active_brightness);
  save_preferences();
}
uint8_t energy_save_get_active_brightness() { return s_active_brightness; }

ScreensaverMode energy_save_get_ss_mode() { return s_ss_mode; }

void energy_save_set_ss_mode(ScreensaverMode mode) {
  s_ss_mode = mode;
  save_preferences();
}

bool energy_save_is_showing_gif() { return s_showing_gif; }
bool energy_save_has_gif()        { return LittleFS.exists(GIF_PATH); }

void energy_save_notify_gif_changed() {
  if (s_showing_gif && !LittleFS.exists(GIF_PATH)) {
    gif_close_if_open();
    s_showing_gif = false;
    M5Dial.Display.setBrightness(s_dim_brightness);
    M5Dial.Display.sleep();
  }
}
