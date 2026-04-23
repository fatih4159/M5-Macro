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
#define CLR_SUCCESS     0xFFFFFF   // White during macro execution
#define CLR_HINT        0x404040   // Status hint at bottom (very subtle)

// ── Widget pointers (file scope) ─────────────────────────────────────────────
static lv_obj_t* s_title  = nullptr;
static lv_obj_t* s_roller = nullptr;
static lv_obj_t* s_status = nullptr;
static int       s_count  = 0;
static String    s_hint   = "Turn=Select  |  Press=Run";

// ── App title at top ─────────────────────────────────────────────────────────
static void create_title(lv_obj_t* parent) {
    s_title = lv_label_create(parent);
    lv_label_set_text(s_title, "m5Macro");
    lv_obj_align(s_title, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_set_style_text_font (s_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_title, lv_color_hex(CLR_ACCENT), 0);
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

// ── Status hint at bottom ────────────────────────────────────────────────────
static void create_status(lv_obj_t* parent) {
    s_status = lv_label_create(parent);
    lv_label_set_text(s_status, s_hint.c_str());
    lv_obj_set_width(s_status, 180);
    lv_label_set_long_mode(s_status, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(s_status, LV_ALIGN_BOTTOM_MID, 0, -24);

    lv_obj_set_style_text_font (s_status, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_status, lv_color_hex(CLR_HINT),  0);
}

// ── Public API ───────────────────────────────────────────────────────────────

void ui_init() {
    lv_obj_t* scr = lv_scr_act();

    // Screen background
    lv_obj_set_style_bg_color(scr, lv_color_hex(CLR_BG),    0);
    lv_obj_set_style_bg_opa  (scr, LV_OPA_COVER,            0);

    //create_title(scr);
    create_roller(scr);
    //create_status(scr);
}

void ui_show_running(int index) {
    // Status text: name of running macro, green
    if (s_status) {
        const MacroInfo* m = macro_store_get(index);
        String msg = String(">> ") + (m ? m->name : "Running") + " ...";
        lv_label_set_text(s_status, msg.c_str());
        lv_obj_set_style_text_color(s_status, lv_color_hex(CLR_SUCCESS), 0);
    }

    // Title: green during execution
    if (s_title) {
        lv_obj_set_style_text_color(s_title, lv_color_hex(CLR_SUCCESS), 0);
    }

    lv_timer_handler();  // Redraw immediately before blocking execution
}

void ui_show_idle() {
    // Status text: usage hint, dimmed
    if (s_status) {
        lv_label_set_text(s_status, s_hint.c_str());
        lv_obj_set_style_text_color(s_status, lv_color_hex(CLR_HINT), 0);
    }

    // Title: back to accent color
    if (s_title) {
        lv_obj_set_style_text_color(s_title, lv_color_hex(CLR_ACCENT), 0);
    }

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

void ui_set_hint(const char* text) {
    s_hint = text;
    if (s_status) {
        lv_label_set_text(s_status, s_hint.c_str());
    }
}
