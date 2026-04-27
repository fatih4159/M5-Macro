#include "lvgl_driver.h"
#include "lvgl_littlefs.h"
#include "config.h"
#include <M5Dial.h>

// ── Render buffer (static, internal SRAM) ───────────────────────────────────
static lv_color_t s_buf[DISPLAY_WIDTH * LV_BUF_LINES];

// ── Display flush callback ───────────────────────────────────────────────────
// Called by LVGL when a region has been rendered.
static void disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);

    // M5GFX writes pixels directly into display RAM.
    // lv_color_t is uint16_t RGB565, LV_COLOR_16_SWAP=1 ensures
    // correct byte order for the GC9A01 display.
    M5Dial.Display.startWrite();
    M5Dial.Display.setAddrWindow(area->x1, area->y1, w, h);
    M5Dial.Display.writePixels((lgfx::rgb565_t*)px_map, w * h);
    M5Dial.Display.endWrite();

    lv_display_flush_ready(disp);
}

// ── Touch read callback ──────────────────────────────────────────────────────
// M5Dial.update() is already called in loop() – only read state here.
static void touch_read(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;
    auto& touch = M5Dial.Touch;

    if (touch.getCount() > 0) {
        auto t = touch.getDetail(0);
        data->state   = LV_INDEV_STATE_PRESSED;
        data->point.x = (int32_t)t.x;
        data->point.y = (int32_t)t.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// ── Initialization ───────────────────────────────────────────────────────────
void lvgl_driver_init() {
    // Create display, set buffer and flush callback
    lv_display_t* disp = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_display_set_flush_cb(disp, disp_flush);
    lv_display_set_buffers(disp, s_buf, nullptr, sizeof(s_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create touch input device
    lv_indev_t* touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, touch_read);

    // Register LittleFS as LVGL drive 'S' (needed for lv_gif file loading)
    lvgl_littlefs_init();
}
