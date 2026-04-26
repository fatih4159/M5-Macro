#include "energy_save.h"
#include "config.h"
#include <M5Dial.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <lvgl.h>

static const char* GIF_PATH    = "/screensaver.gif";

// ── LVGL GIF widget state ─────────────────────────────────────────────────────
static lv_obj_t*   s_gif_overlay = nullptr;  // full-screen black backdrop
static lv_obj_t*   s_gif_obj     = nullptr;  // lv_gif widget
static uint8_t*    s_gif_data    = nullptr;  // raw GIF bytes in RAM
static lv_img_dsc_t s_gif_dsc   = {};       // descriptor wrapping s_gif_data

static bool s_showing_gif = false;

namespace {
  constexpr const char* PREF_NS       = "energy";
  constexpr const char* KEY_ENABLED   = "enabled";
  constexpr const char* KEY_TIMEOUT   = "timeout_s";
  constexpr const char* KEY_OFF_TO    = "off_timeout";
  constexpr const char* KEY_DIM_BR    = "dim_br";
  constexpr const char* KEY_ACTIVE_BR = "active_br";
  constexpr const char* KEY_SS_MODE   = "ss_mode";

  bool            s_enabled           = true;
  uint32_t        s_dim_timeout_ms    = (uint32_t)ENERGY_SAVE_TIMEOUT_DEFAULT * 1000UL;
  uint32_t        s_off_timeout_ms    = 0;
  uint8_t         s_dim_brightness    = ENERGY_SAVE_DIM_BRIGHTNESS;
  uint8_t         s_active_brightness = ENERGY_SAVE_ACTIVE_BRIGHTNESS;
  uint32_t        s_last_activity     = 0;
  uint32_t        s_dim_start_ms      = 0;
  bool            s_dimmed            = false;
  bool            s_display_off       = false;
  ScreensaverMode s_ss_mode           = SS_MODE_DIM_ONLY;

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

  // ── GIF widget helpers ──────────────────────────────────────────────────────

  // Read GIF canvas dimensions directly from the file header (6 bytes magic +
  // 2-byte LE width + 2-byte LE height in the Logical Screen Descriptor).
  static void read_gif_size(uint16_t& w, uint16_t& h) {
    w = 0; h = 0;
    File f = LittleFS.open(GIF_PATH, "r");
    if (!f || f.size() < 10) { if (f) f.close(); return; }
    uint8_t buf[10];
    f.read(buf, 10);
    f.close();
    w = (uint16_t)(buf[6] | (buf[7] << 8));
    h = (uint16_t)(buf[8] | (buf[9] << 8));
  }

  static void destroy_gif_widget() {
    if (s_gif_overlay) {
      lv_obj_del(s_gif_overlay);  // deletes children (s_gif_obj) automatically
      s_gif_overlay = nullptr;
      s_gif_obj     = nullptr;
    }
    // Free after lv_obj_del so the GIF decoder is fully torn down first
    free(s_gif_data);
    s_gif_data = nullptr;
    s_showing_gif = false;
  }

  static void create_gif_widget() {
    // Read GIF canvas size to compute zoom-to-fit
    uint16_t gw = 0, gh = 0;
    read_gif_size(gw, gh);
    if (gw == 0) gw = 240;
    if (gh == 0) gh = 240;

    // Load the entire GIF into RAM.
    // lv_gif_set_src only opens a file path when LV_USE_FS_IF_ANY is set
    // (i.e. a built-in LVGL FS backend is compiled in). Our custom 'S' driver
    // does not set that flag, so the file-path branch is silently skipped.
    // Passing an lv_img_dsc_t* (LV_IMG_SRC_VARIABLE) always works.
    File f = LittleFS.open(GIF_PATH, "r");
    if (!f) return;
    size_t fsize = f.size();

    // Prefer PSRAM; fall back to SRAM
    s_gif_data = (uint8_t*)heap_caps_malloc(fsize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_gif_data) s_gif_data = (uint8_t*)malloc(fsize);
    if (!s_gif_data) { f.close(); return; }

    f.read(s_gif_data, fsize);
    f.close();

    s_gif_dsc            = {};
    s_gif_dsc.header.cf  = LV_IMG_CF_RAW;
    s_gif_dsc.header.w   = gw;
    s_gif_dsc.header.h   = gh;
    s_gif_dsc.data_size  = fsize;
    s_gif_dsc.data       = s_gif_data;

    // Full-screen black backdrop (covers the LVGL UI behind the GIF)
    s_gif_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_gif_overlay);
    lv_obj_set_size(s_gif_overlay, 240, 240);
    lv_obj_set_pos(s_gif_overlay, 0, 0);
    lv_obj_set_style_bg_color(s_gif_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_gif_overlay, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_gif_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // GIF widget – use in-RAM descriptor so lv_gif_set_src always decodes it
    s_gif_obj = lv_gif_create(s_gif_overlay);
    lv_gif_set_src(s_gif_obj, &s_gif_dsc);

    // Scale uniformly to fit 240×240, preserve aspect ratio (letterbox/pillarbox)
    float scale = (float)240 / gw;
    if ((float)240 / gh < scale) scale = (float)240 / gh;
    uint16_t zoom = (uint16_t)(scale * 256.0f + 0.5f);
    if (zoom < 1) zoom = 1;

    lv_img_set_pivot(s_gif_obj, 0, 0);  // zoom from top-left
    lv_img_set_zoom(s_gif_obj, zoom);

    // Center the scaled GIF inside the 240×240 overlay
    int xoff = (int)((240.0f - gw * scale) * 0.5f + 0.5f);
    int yoff = (int)((240.0f - gh * scale) * 0.5f + 0.5f);
    lv_obj_set_pos(s_gif_obj, xoff, yoff);

    s_showing_gif = true;
  }

  // ── Display state helpers ───────────────────────────────────────────────────

  void apply_active_state() {
    destroy_gif_widget();
    s_dimmed      = false;
    s_display_off = false;
    M5Dial.Display.wakeup();
    M5Dial.Display.setBrightness(s_active_brightness);
    lv_obj_invalidate(lv_scr_act());
  }

  void apply_off_state() {
    destroy_gif_widget();
    s_display_off = true;
    M5Dial.Display.setBrightness(0);
    M5Dial.Display.sleep();
  }

  void apply_dimmed_state() {
    s_dimmed       = true;
    s_display_off  = false;
    s_dim_start_ms = millis();

    if (s_ss_mode == SS_MODE_GIF && LittleFS.exists(GIF_PATH)) {
      M5Dial.Display.wakeup();
      M5Dial.Display.setBrightness(s_active_brightness);
      create_gif_widget();
    } else {
      destroy_gif_widget();
      // Just lower brightness — do NOT call sleep() here, that blanks the panel
      M5Dial.Display.setBrightness(s_dim_brightness);
    }
  }
}

void energy_save_init() {
  destroy_gif_widget();

  Preferences prefs;
  if (prefs.begin(PREF_NS, true)) {
    s_enabled           = prefs.getBool(KEY_ENABLED, true);
    s_dim_timeout_ms    = (uint32_t)prefs.getUInt(KEY_TIMEOUT, ENERGY_SAVE_TIMEOUT_DEFAULT) * 1000UL;
    s_off_timeout_ms    = (uint32_t)prefs.getUInt(KEY_OFF_TO,  0) * 1000UL;
    s_dim_brightness    = (uint8_t)prefs.getUInt(KEY_DIM_BR,    ENERGY_SAVE_DIM_BRIGHTNESS);
    s_active_brightness = (uint8_t)prefs.getUInt(KEY_ACTIVE_BR, ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    s_ss_mode           = (ScreensaverMode)constrain((int)prefs.getUInt(KEY_SS_MODE, 0), 0, 2);
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

// No-op: GIF animation is driven by lv_timer_handler() via the lv_gif widget
void energy_save_gif_tick() {}

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
    destroy_gif_widget();
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
  s_dim_brightness = brightness;
  if (s_dimmed && !s_showing_gif && !s_display_off)
    M5Dial.Display.setBrightness(s_dim_brightness);
  save_preferences();
}
uint8_t energy_save_get_dim_brightness() { return s_dim_brightness; }

void energy_save_set_active_brightness(uint8_t brightness) {
  s_active_brightness = brightness;
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
    destroy_gif_widget();
    M5Dial.Display.setBrightness(s_dim_brightness);
  }
}
