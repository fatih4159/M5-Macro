#include "ui.h"
#include "macro_store.h"
#include "config.h"

// ── Dark mode color palette ──────────────────────────────────────────────────
// All colors as RGB hex for lv_color_hex()
#define CLR_BG          0x000000   // Deep black
#define CLR_SURFACE     0x000000   // Roller background
#define CLR_ACCENT      0x808080   // Medium gray (border of selected entry)
#define CLR_SEL_BG      0x1A1A1A   // Background of selected entry
#define CLR_TEXT        0xFFFFFF   // Bright text (selected / active)
#define CLR_TEXT_DIM    0x888888   // Dimmed text (not selected)

// ── Widget pointers (file scope) ─────────────────────────────────────────────
static lv_obj_t* s_roller = nullptr;
static int       s_count  = 0;

// ── Macro roller (center) ────────────────────────────────────────────────────
static void create_roller(lv_obj_t* parent) {
    // Build options string from macro store
    String opts = "";
    s_count = macro_store_count();
    for (int i = 0; i < s_count; i++) {
        const MacroInfo* m = macro_store_get(i);
        if (m) {
            if (opts.length() > 0) opts += "\n";
            opts += String(m->name);
        }
    }
    if (opts.length() == 0) opts = "(no macros)";

    s_roller = lv_roller_create(parent);
    lv_roller_set_options(s_roller, opts.c_str(), LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(s_roller, 7);
    lv_obj_set_width(s_roller, 230);
    lv_obj_align(s_roller, LV_ALIGN_CENTER, 0, 6);

    // Main style: dark background, dimmed text
    lv_obj_set_style_bg_color   (s_roller, lv_color_hex(CLR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa     (s_roller, LV_OPA_COVER,              LV_PART_MAIN);
    lv_obj_set_style_border_width(s_roller, 0,                         LV_PART_MAIN);
    lv_obj_set_style_text_color (s_roller, lv_color_hex(CLR_TEXT_DIM), LV_PART_MAIN);
    lv_obj_set_style_text_font  (s_roller, &lv_font_montserrat_16,     LV_PART_MAIN);
    lv_obj_set_style_pad_left   (s_roller, 10,                         LV_PART_MAIN);
    lv_obj_set_style_pad_right  (s_roller, 10,                         LV_PART_MAIN);

    // Selected entry: accent color, larger font, subtle border
    lv_obj_set_style_bg_color    (s_roller, lv_color_hex(CLR_SEL_BG),  LV_PART_SELECTED);
    lv_obj_set_style_bg_opa      (s_roller, LV_OPA_COVER,              LV_PART_SELECTED);
    lv_obj_set_style_text_color  (s_roller, lv_color_hex(CLR_TEXT),    LV_PART_SELECTED);
    lv_obj_set_style_text_font   (s_roller, &lv_font_montserrat_20,    LV_PART_SELECTED);
    lv_obj_set_style_border_color(s_roller, lv_color_hex(CLR_ACCENT),  LV_PART_SELECTED);
    lv_obj_set_style_border_width(s_roller, 1,                         LV_PART_SELECTED);
    lv_obj_set_style_border_opa  (s_roller, LV_OPA_50,                 LV_PART_SELECTED);
}

// ── Public API ───────────────────────────────────────────────────────────────

void ui_init() {
    lv_obj_t* scr = lv_scr_act();

    // Screen background
    lv_obj_set_style_bg_color(scr, lv_color_hex(CLR_BG),    0);
    lv_obj_set_style_bg_opa  (scr, LV_OPA_COVER,            0);

    create_roller(scr);
}

void ui_show_running(int /*index*/) {
    lv_timer_handler();  // Redraw immediately before blocking execution
}

void ui_show_idle() {
    lv_timer_handler();
}

int ui_get_selected() {
    if (!s_roller || s_count == 0) return 0;
    return (int)lv_roller_get_selected(s_roller);
}

void ui_set_selected(int index) {
    if (!s_roller || s_count == 0) return;
    lv_roller_set_selected(s_roller, (uint16_t)index, LV_ANIM_ON);
}

void ui_reload() {
    // Rebuild roller options from updated macro store
    String opts = "";
    s_count = macro_store_count();
    for (int i = 0; i < s_count; i++) {
        const MacroInfo* m = macro_store_get(i);
        if (m) {
            if (opts.length() > 0) opts += "\n";
            opts += String(m->name);
        }
    }
    if (opts.length() == 0) opts = "(no macros)";

    if (s_roller) {
        lv_roller_set_options(s_roller, opts.c_str(), LV_ROLLER_MODE_INFINITE);
        lv_roller_set_selected(s_roller, 0, LV_ANIM_OFF);
    }
}

