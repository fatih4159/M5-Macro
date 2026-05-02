#pragma once

// Let M5GFX's LVGL font compatibility shim find the real PlatformIO LVGL
// package instead of falling back to its bundled LVGL 9.3 type stubs.
#if __has_include("../.pio/libdeps/m5dial/lvgl/lvgl.h")
#include "../.pio/libdeps/m5dial/lvgl/lvgl.h"
#else
#include <lvgl.h>
#endif

#ifndef LV_FONT_GLYPH_FORMAT_A1_ALIGNED
#define LV_FONT_GLYPH_FORMAT_A1_ALIGNED 0x011
#endif
#ifndef LV_FONT_GLYPH_FORMAT_A2_ALIGNED
#define LV_FONT_GLYPH_FORMAT_A2_ALIGNED 0x012
#endif
#ifndef LV_FONT_GLYPH_FORMAT_A4_ALIGNED
#define LV_FONT_GLYPH_FORMAT_A4_ALIGNED 0x014
#endif
