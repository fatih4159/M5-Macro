#include "ui.h"
#include "macro_store.h"
#include "config.h"
#include <Preferences.h>

// ── Runtime color palette (loaded from NVS, defaults match original design) ──
static uint32_t c_bg       = 0x000000;
static uint32_t c_surface  = 0x000000;
static uint32_t c_accent   = 0x808080;
static uint32_t c_sel_bg   = 0x1A1A1A;
static uint32_t c_text     = 0xFFFFFF;
static uint32_t c_text_dim = 0x888888;

static void load_colors() {
    Preferences prefs;
    prefs.begin("colors", true);
    c_bg       = prefs.getUInt("bg",       0x000000);
    c_surface  = prefs.getUInt("surface",  0x000000);
    c_accent   = prefs.getUInt("accent",   0x808080);
    c_sel_bg   = prefs.getUInt("sel_bg",   0x1A1A1A);
    c_text     = prefs.getUInt("text",     0xFFFFFF);
    c_text_dim = prefs.getUInt("text_dim", 0x888888);
    prefs.end();
}

// ── Widget pointers (file scope) ─────────────────────────────────────────────
static lv_obj_t* s_roller = nullptr;
static int       s_count  = 0;

// ── Gradient fade mask for roller ────────────────────────────────────────────
static void generate_mask(lv_draw_buf_t* mask)
{
    lv_obj_t* canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_draw_buf(canvas, mask);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_TRANSP);

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_grad.dir = LV_GRAD_DIR_VER;
    rect_dsc.bg_grad.stops_count = 2;
    rect_dsc.bg_grad.stops[0].color = lv_color_black();
    rect_dsc.bg_grad.stops[0].opa   = LV_OPA_COVER;
    rect_dsc.bg_grad.stops[1].color = lv_color_white();
    rect_dsc.bg_grad.stops[1].opa   = LV_OPA_COVER;
    lv_area_t a = {0, 0, (lv_coord_t)(mask->header.w - 1), (lv_coord_t)(mask->header.h / 2 - 10)};
    lv_draw_rect(&layer, &rect_dsc, &a);

    a.y1 = mask->header.h / 2 + 10;
    a.y2 = mask->header.h - 1;
    rect_dsc.bg_grad.stops[0].color = lv_color_white();
    rect_dsc.bg_grad.stops[1].color = lv_color_black();
    lv_draw_rect(&layer, &rect_dsc, &a);

    lv_canvas_finish_layer(canvas, &layer);
    lv_obj_delete(canvas);
}

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
    lv_obj_set_style_radius(s_roller, 10, LV_PART_SELECTED);

    lv_roller_set_visible_row_count(s_roller, 5);
    lv_obj_set_width(s_roller, 230);
    lv_obj_align(s_roller, LV_ALIGN_CENTER, 0, 6);

    // Main style: dark background, dimmed text
    lv_obj_set_style_bg_color   (s_roller, lv_color_hex(c_surface),  LV_PART_MAIN);
    lv_obj_set_style_bg_opa     (s_roller, LV_OPA_COVER,              LV_PART_MAIN);
    lv_obj_set_style_border_width(s_roller, 0,                         LV_PART_MAIN);
    lv_obj_set_style_text_color (s_roller, lv_color_hex(c_text_dim),  LV_PART_MAIN);
    lv_obj_set_style_text_font  (s_roller, &lv_font_montserrat_16,     LV_PART_MAIN);
    lv_obj_set_style_pad_left   (s_roller, 10,                         LV_PART_MAIN);
    lv_obj_set_style_pad_right  (s_roller, 10,                         LV_PART_MAIN);

    // Selected entry: accent color, larger font, subtle border
    lv_obj_set_style_bg_color    (s_roller, lv_color_hex(c_sel_bg),   LV_PART_SELECTED);
    lv_obj_set_style_bg_opa      (s_roller, LV_OPA_COVER,              LV_PART_SELECTED);
    lv_obj_set_style_text_color  (s_roller, lv_color_hex(c_text),     LV_PART_SELECTED);
    lv_obj_set_style_text_font   (s_roller, &lv_font_montserrat_20,    LV_PART_SELECTED);
    lv_obj_set_style_border_color(s_roller, lv_color_hex(c_accent),   LV_PART_SELECTED);
    lv_obj_set_style_border_width(s_roller, 1,                         LV_PART_SELECTED);
    lv_obj_set_style_border_opa  (s_roller, LV_OPA_50,                 LV_PART_SELECTED);
}

// ── Public API ───────────────────────────────────────────────────────────────

void ui_init() {
    load_colors();
    lv_obj_t* scr = lv_scr_act();

    // Screen background
    lv_obj_set_style_bg_color(scr, lv_color_hex(c_bg),    0);
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

void ui_apply_colors() {
    load_colors();
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(c_bg), 0);
    if (!s_roller) return;
    lv_obj_set_style_bg_color   (s_roller, lv_color_hex(c_surface),  LV_PART_MAIN);
    lv_obj_set_style_text_color (s_roller, lv_color_hex(c_text_dim), LV_PART_MAIN);
    lv_obj_set_style_bg_color   (s_roller, lv_color_hex(c_sel_bg),   LV_PART_SELECTED);
    lv_obj_set_style_text_color (s_roller, lv_color_hex(c_text),     LV_PART_SELECTED);
    lv_obj_set_style_border_color(s_roller, lv_color_hex(c_accent),  LV_PART_SELECTED);
    lv_timer_handler();
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

