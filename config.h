#pragma once

// ── Display ────────────────────────────────────────────────────────────────
#define DISPLAY_WIDTH    240
#define DISPLAY_HEIGHT   240

// LVGL Render-Puffer (Zeilen pro Flush-Block)
// 240 * 40 * 2 Byte = ~19 KB – gut fuer den internen SRAM
#define LV_BUF_LINES     40

// ── Encoder ────────────────────────────────────────────────────────────────
// M5Dial Encoder: 4 Impulse pro Raststellung
#define ENCODER_PPR      4

// ── Parser ─────────────────────────────────────────────────────────────────
#define MACRO_MAX_STEPS  128         // Max. Schritte pro Makro (Parser-Limit)
#define STEP_TEXT_LEN    128         // Max. Textlaenge fuer TEXT:-Befehle
#define STEP_MOD_MAX     4           // Max. Modifier-Tasten pro Schritt

// ── Ausfuehrung ────────────────────────────────────────────────────────────
// Pause zwischen Tastendruck und -loslassen (ms)
#define KEY_HOLD_MS      15
// Standard-Pause zwischen zwei Schritten (ms), wenn kein DELAY: angegeben
#define STEP_GAP_MS      10

// ── WiFi / Web-Editor ──────────────────────────────────────────────────────
// Das Geraet oeffnet einen eigenen WLAN-Hotspot (Access Point).
// Verbinde dich mit diesem Netzwerk und oeffne http://192.168.4.1
#define WIFI_AP_SSID     "m5Macro"
#define WIFI_AP_PASS     "m5macro1"   // mind. 8 Zeichen fuer WPA2
#define WEB_SERVER_PORT  80
