#include "lvgl_driver.h"
#include "lvgl_littlefs.h"
#include "config.h"
#include <M5Dial.h>

// ── Render buffer (static, internal SRAM) ───────────────────────────────────
static lv_color_t s_buf[DISPLAY_WIDTH * LV_BUF_LINES];
static lv_disp_draw_buf_t s_draw_buf;

// ── Display flush callback ───────────────────────────────────────────────────
// Called by LVGL when a region has been rendered.
static void disp_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);

    // M5GFX writes pixels directly into display RAM.
    // lv_color_t is uint16_t RGB565, LV_COLOR_16_SWAP=1 ensures
    // correct byte order for the GC9A01 display.
    M5Dial.Display.startWrite();
    M5Dial.Display.setAddrWindow(area->x1, area->y1, w, h);
    M5Dial.Display.writePixels((lgfx::rgb565_t*)color_p, w * h);
    M5Dial.Display.endWrite();

    lv_disp_flush_ready(drv);
}

// ── Touch read callback ──────────────────────────────────────────────────────
// M5Dial.update() is already called in loop() – only read state here.
static void touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    (void)drv;
    auto& touch = M5Dial.Touch;

    if (touch.getCount() > 0) {
        auto t = touch.getDetail(0);
        data->state   = LV_INDEV_STATE_PR;
        data->point.x = (lv_coord_t)t.x;
        data->point.y = (lv_coord_t)t.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ── Initialization ───────────────────────────────────────────────────────────
void lvgl_driver_init() {
    // Register render buffer
    lv_disp_draw_buf_init(&s_draw_buf, s_buf, nullptr, DISPLAY_WIDTH * LV_BUF_LINES);

    // Configure display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res     = DISPLAY_WIDTH;
    disp_drv.ver_res     = DISPLAY_HEIGHT;
    disp_drv.flush_cb    = disp_flush;
    disp_drv.draw_buf    = &s_draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Register touch input device
    static lv_indev_drv_t touch_drv;
    lv_indev_drv_init(&touch_drv);
    touch_drv.type    = LV_INDEV_TYPE_POINTER;
    touch_drv.read_cb = touch_read;
    lv_indev_drv_register(&touch_drv);

    // Register LittleFS as LVGL drive 'S' (needed for lv_gif file loading)
    lvgl_littlefs_init();
}
