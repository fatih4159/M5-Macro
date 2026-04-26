#pragma once

// Registers an LVGL filesystem driver (drive letter 'S') backed by Arduino LittleFS.
// Call once after lv_init().  Paths are passed without the drive prefix, e.g.
//   lv_gif_set_src(obj, "S:/screensaver.gif")  -> open_cb receives "/screensaver.gif"
void lvgl_littlefs_init();
