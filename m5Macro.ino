/*
 * m5Macro – Tastatur-Makro-Pad fuer M5Dial
 * =========================================
 * Hardware : M5Dial (ESP32-S3, GC9A01 240x240 Round Display, Rotary Encoder,
 *                    FT3267 Touch, USB-OTG)
 * UI       : LVGL 8.x  (Roller-Widget, Encoder-Navigation)
 * Makros   : Fest in macro_store.cpp einkodiert
 * Ausgabe  : USB-HID-Keyboard  (ESP32 Arduino Core TinyUSB)
 *
 * WICHTIG – Arduino IDE Board-Einstellungen:
 *   Board          : M5Stack / M5Dial
 *   USB Mode       : USB-OTG (TinyUSB)  ← PFLICHT fuer HID-Keyboard!
 *   USB CDC On Boot: Disabled
 *
 * Bedienung:
 *   Encoder drehen  → Makro auswaehlen
 *   Encoder druecken → Makro ausfuehren
 *   Touchscreen     → Makro antippen zum Auswaehlen/Ausfuehren
 */

// ── USB muss als Erstes eingebunden werden ──────────────────────────────────
#include "USB.h"
#include "USBHIDKeyboard.h"

// Globale Keyboard-Instanz (wird in macro_executor.h als extern deklariert)
USBHIDKeyboard Keyboard;

// ── Weitere Bibliotheken ────────────────────────────────────────────────────
#include <M5Dial.h>
#include <lvgl.h>

// ── Projektdateien ──────────────────────────────────────────────────────────
#include "config.h"
#include "lvgl_driver.h"
#include "macro_store.h"
#include "macro_executor.h"
#include "ui.h"
#include "web_server.h"

// ── Zustandsvariablen ───────────────────────────────────────────────────────
static bool  s_executing   = false;   // Verhindert Doppel-Ausfuehrung
static int   s_enc_accum   = 0;       // Akkumulierter Encoder-Wert

// ── Encoder-Verarbeitung ────────────────────────────────────────────────────
// Wird einmal pro loop() aufgerufen, VOR lv_timer_handler().
static void handle_encoder() {
    // Rohimpulse seit letztem Aufruf lesen
    int32_t raw = M5Dial.Encoder.readAndReset();
    s_enc_accum += raw;

    // Erst nach ENCODER_PPR Impulsen eine Raststellung zaehlen
    int steps = s_enc_accum / ENCODER_PPR;
    s_enc_accum %= ENCODER_PPR;

    if (steps != 0 && !s_executing) {
        int current = ui_get_selected();
        int count   = macro_store_count();
        if (count > 0) {
            // Modulo-Arithmetik mit Wrap-Around
            int next = ((current + steps) % count + count) % count;
            ui_set_selected(next);
        }
    }
}

// ── Touch-Gesten-Verarbeitung ───────────────────────────────────────────────
// Einfaches Tap-to-Execute: kurzer Tap auf beliebige Bildschirmstelle.
static uint32_t s_touch_start = 0;
static bool     s_touch_active = false;

static void handle_touch() {
    if (s_executing) return;

    bool pressed = M5Dial.Touch.getCount() > 0;

    if (pressed && !s_touch_active) {
        s_touch_active = true;
        s_touch_start  = millis();
    } else if (!pressed && s_touch_active) {
        s_touch_active = false;
        uint32_t duration = millis() - s_touch_start;
        // Tap: kuerzer als 400 ms
        if (duration < 400) {
            int idx = ui_get_selected();
            if (macro_store_count() > 0) {
                //s_executing = true;
                //ui_show_running(idx);
                //macro_execute(idx);
                //ui_show_idle();
                s_executing = false;
            }
        }
    }
}

// ── Encoder-Taster-Verarbeitung ─────────────────────────────────────────────
static void handle_button() {
    if (s_executing) return;

    if (M5Dial.BtnA.wasPressed()) {
        int idx = ui_get_selected();
        if (macro_store_count() > 0) {
            s_executing = true;
            ui_show_running(idx);
            macro_execute(idx);
            ui_show_idle();
            s_executing = false;
        }
    }
}

// ── Setup ───────────────────────────────────────────────────────────────────
void setup() {
    // 1. USB ZUERST initialisieren (ESP32-S3 TinyUSB)
    USB.productName("m5Macro Keyboard");
    USB.manufacturerName("m5Stack");
    USB.begin();
    Keyboard.begin();
    delay(500); // USB-Enumeration abwarten

    // 2. M5Dial Hardware initialisieren
    //    Parameter: config, Encoder aktivieren, RFID deaktivieren
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setRotation(2);
    M5Dial.Display.setBrightness(128);
    M5Dial.Display.setSwapBytes(true);

    // Encoder-Referenzposition setzen
    M5Dial.Encoder.readAndReset();

    // 3. LVGL initialisieren
    lv_init();
    lvgl_driver_init();

    // 4. Makros initialisieren (fest einkodiert, kein Dateisystem noetig)
    macro_store_init();

    // 5. UI aufbauen
    ui_init();

    // 6. Web-Editor starten (WiFi-AP + HTTP-Server)
    web_server_init();
    ui_set_hint(("http://" + web_server_ip()).c_str());
}

// ── Hauptschleife ────────────────────────────────────────────────────────────
void loop() {
    // Einmaliges Hardware-Update (Buttons, Touch, Encoder-Interrupts)
    M5Dial.update();

    // Encoder-Rotation verarbeiten
    handle_encoder();

    // Touch-Gesten verarbeiten
    handle_touch();

    // Encoder-Taster verarbeiten
    handle_button();

    // LVGL rendern (inkl. Touch-Input aus lvgl_driver.cpp)
    lv_timer_handler();

    // Eingehende HTTP-Anfragen verarbeiten (nicht-blockierend)
    web_server_handle();

    // ~5 ms Pause → ~200 fps Deckelung, gibt RTOS-Tasks Luft
    delay(5);
}
