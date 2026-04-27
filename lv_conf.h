/* lv_conf.h - Konfiguration fuer M5Dial Macro Keyboard */
#ifndef LV_CONF_H
#define LV_CONF_H

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

#define LV_CONF_SKIP
#if 1

/* Farbe */
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0  /* Pflicht fuer M5GFX/M5Stack Displays */

/* Speicher */
#define LV_MEM_SIZE        (48U * 1024U)

/* Tick - millis() direkt nutzen */
#define LV_TICK_CUSTOM     1
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

/* Fonts */
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_20    1
#define LV_FONT_MONTSERRAT_28    1

/* Memory – use system malloc so lv_gif can allocate large GIF canvas buffers in PSRAM */
#define LV_MEM_CUSTOM         1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC   malloc
#define LV_MEM_CUSTOM_REALLOC realloc
#define LV_MEM_CUSTOM_FREE    free

/* GIF screensaver widget */
#define LV_USE_GIF        1

/* Widgets */
#define LV_USE_ARC        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_LABEL      1
#define LV_USE_LIST       1
#define LV_USE_ROLLER     1
#define LV_USE_SPINNER    1
#define LV_USE_ANIMIMG    0

/* Gruppen (fuer Encoder-Navigation) */
#define LV_USE_GROUP      1

/* Animation */
#define LV_USE_ANIMATION  1

/* Log (ausgeschaltet fuer Produktion) */
#define LV_USE_LOG        0

/* Asserts ausschalten fuer Performance */
#define LV_USE_ASSERT_NULL          0
#define LV_USE_ASSERT_MALLOC        0
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

#endif
#endif
