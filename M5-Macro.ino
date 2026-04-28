/*
 * m5Macro – Keyboard Macro Pad for M5Dial
 * =========================================
 * Hardware : M5Dial (ESP32-S3, GC9A01 240x240 Round Display, Rotary Encoder,
 *                    FT3267 Touch, USB-OTG)
 * UI       : LVGL 8.x  (Roller widget, encoder navigation)
 * Macros   : Hardcoded in macro_store.cpp
 * Output   : USB-HID-Keyboard  (ESP32 Arduino Core TinyUSB)
 *
 * IMPORTANT – Arduino IDE Board Settings:
 *   Board          : M5Stack / M5Dial
 *   USB Mode       : USB-OTG (TinyUSB)  ← REQUIRED for HID-Keyboard!
 *   USB CDC On Boot: Disabled
 *
 * Controls:
 *   Rotate encoder  → Select macro
 *   Press encoder   → Execute macro
 */

// ── USB must be included first ──────────────────────────────────────────────
#include "USB.h"
#include "USBHIDKeyboard.h"

// Global keyboard instance (declared as extern in macro_executor.h)
USBHIDKeyboard Keyboard;

// ── Additional libraries ────────────────────────────────────────────────────
#include <M5Dial.h>
#include <lvgl.h>

// ── Project files ───────────────────────────────────────────────────────────
#include "config.h"
#include "logger.h"
#include "lvgl_driver.h"
#include "macro_store.h"
#include "macro_executor.h"
#include "ble_keyboard_hid.h"
#include "ui.h"
#include "web_server.h"
#include "energy_save.h"

// ── State variables ─────────────────────────────────────────────────────────
static bool  s_executing   = false;   // Prevents double execution
static int   s_enc_accum   = 0;       // Accumulated encoder value

// ── Encoder handling ────────────────────────────────────────────────────────
// Called once per loop(), BEFORE lv_timer_handler().
static void handle_encoder() {
    // Read raw pulses since last call
    int32_t raw = M5Dial.Encoder.readAndReset();
    s_enc_accum += raw;

    // Count one detent only after ENCODER_PPR pulses
    int steps = s_enc_accum / ENCODER_PPR;
    s_enc_accum %= ENCODER_PPR;

    if (steps != 0) {
        // In energy-saving mode: only wake up, don't change selection
        if (energy_save_is_dimmed()) {
            energy_save_activity();
            s_enc_accum = 0;
            return;
        }
        if (!s_executing) {
            int current = ui_get_selected();
            int count   = macro_store_count();
            if (count > 0) {
                // Modulo arithmetic with wrap-around
                int next = ((current + steps) % count + count) % count;
                ui_set_selected(next);
            }
        }
        energy_save_activity();
    }
}

// ── Encoder button handling ─────────────────────────────────────────────────
static void handle_button() {
    if (s_executing) return;

    if (M5Dial.BtnA.wasPressed()) {
        // In energy-saving mode: only wake up, don't execute macro
        if (energy_save_is_dimmed()) {
            energy_save_activity();
            return;
        }
        int idx = ui_get_selected();
        if (macro_store_count() > 0) {
            s_executing = true;
            energy_save_activity();
            ui_show_running(idx);
            macro_execute(idx);
            ui_show_idle();
            s_executing = false;
        }
    }
}

// ── Setup ───────────────────────────────────────────────────────────────────
void setup() {
    // 1. Initialize USB FIRST (ESP32-S3 TinyUSB)
    USB.productName("m5Macro Keyboard");
    USB.manufacturerName("m5Stack");
    USB.begin();
    Keyboard.begin();
    delay(500); // Wait for USB enumeration

    // 2. Initialize M5Dial hardware
    //    Parameters: config, enable encoder, disable RFID
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setRotation(2);
    M5Dial.Display.setSwapBytes(true);

    // Set encoder reference position
    M5Dial.Encoder.readAndReset();

    // 3. Initialize LVGL
    lv_init();
    lv_tick_set_cb(millis);  // LVGL v9: runtime tick source (replaces LV_TICK_CUSTOM)
    lvgl_driver_init();

    // 4. Initialize macros (hardcoded, no filesystem needed)
    macro_store_init();

    // 5. Build UI
    ui_init();

    // 6. Initialize energy-saving mode (also sets display brightness)
    energy_save_init();

    // 7. Start web editor (WiFi AP + HTTP server)
    web_server_init();

    // 8. Prepare BLE keyboard (stays off until user enables via button)
    ble_keyboard_init();

    LOG_I("BOOT", "ready – IP %s", web_server_ip().c_str());
}

// ── Main loop ────────────────────────────────────────────────────────────────
void loop() {
    // Single hardware update (buttons, touch, encoder interrupts)
    M5Dial.update();

    // Process encoder rotation
    handle_encoder();

    // Process encoder button
    handle_button();

    // LVGL renders everything – including the lv_gif screensaver widget
    lv_timer_handler();

    // Handle incoming HTTP requests (non-blocking)
    web_server_handle();

    // Check energy-saving mode and dim display if needed
    energy_save_update();

    // ~5 ms delay → ~200 fps cap, gives RTOS tasks breathing room
    delay(5);
}
