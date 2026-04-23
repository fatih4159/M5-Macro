#pragma once

// ── Display ────────────────────────────────────────────────────────────────
#define DISPLAY_WIDTH    240
#define DISPLAY_HEIGHT   240

// LVGL render buffer (lines per flush block)
// 240 * 40 * 2 bytes = ~19 KB – fits well in internal SRAM
#define LV_BUF_LINES     40

// ── Encoder ────────────────────────────────────────────────────────────────
// M5Dial encoder: 4 pulses per detent
#define ENCODER_PPR      4

// ── Parser ─────────────────────────────────────────────────────────────────
#define MACRO_MAX_STEPS  128         // Max steps per macro (parser limit)
#define STEP_TEXT_LEN    128         // Max text length for TEXT: commands
#define STEP_MOD_MAX     4           // Max modifier keys per step

// ── Execution ──────────────────────────────────────────────────────────────
// Delay between key press and release (ms)
#define KEY_HOLD_MS      15
// Default delay between two steps (ms) if no DELAY: is specified
#define STEP_GAP_MS      10

// ── Energy saving ──────────────────────────────────────────────────────────
#define ENERGY_SAVE_TIMEOUT_DEFAULT    30   // Seconds until display dims
#define ENERGY_SAVE_DIM_BRIGHTNESS     10   // Brightness in energy-saving mode (0–255)
#define ENERGY_SAVE_ACTIVE_BRIGHTNESS  128  // Normal brightness (0–255)

// ── WiFi / Web editor ──────────────────────────────────────────────────────
// The device opens its own WiFi hotspot (Access Point).
// Connect to this network and open http://192.168.4.1
#define WIFI_AP_SSID     "m5Macro"
#define WIFI_AP_PASS     "m5macro1"   // min. 8 characters for WPA2
#define WEB_SERVER_PORT  80
