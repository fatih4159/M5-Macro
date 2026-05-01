# m5Macro

> A compact, programmable **USB & Bluetooth HID macro pad** built on the **M5Dial (ESP32-S3)**.  
> Select macros with the rotary encoder, execute them instantly as real keyboard input — no drivers required.  
> Manage everything through a built-in **browser-based web editor**.

---

## Table of Contents

- [Features](#-features)
- [Hardware](#-hardware)
- [Getting Started](#-getting-started)
  - [1. Clone the Repository](#1-clone-the-repository)
  - [2. Open in Arduino IDE](#2-open-in-arduino-ide)
  - [3. Install Dependencies](#3-install-dependencies)
  - [4. Configure Arduino Settings](#4-configure-arduino-settings)
  - [5. Flash the Device](#5-flash-the-device)
- [Usage](#-usage)
- [Web Editor](#-web-editor)
- [Macro Syntax](#-macro-syntax)
- [Web API Reference](#-web-api-reference)
- [Configuration](#-configuration)
- [Storage](#-storage)
- [Limitations](#-limitations)
- [License](#-license)

---

## ✨ Features

| Feature | Description |
|---|---|
| **USB HID Keyboard** | Plug & play — recognized as a standard keyboard, no drivers needed |
| **Bluetooth HID Keyboard** | Execute macros wirelessly via BLE (toggle on/off from the display) |
| **Rotary Encoder Control** | Rotate to browse macros, press to execute |
| **Circular LVGL UI** | Smooth roller widget on the 240×240 round display |
| **Wi-Fi Toggle Button** | Enable/disable the Wi-Fi AP directly on the device display |
| **Bluetooth Toggle Button** | Enable/disable BLE output directly on the device display; active state shown with white border |
| **Web-Based Macro Editor** | Create, edit, reorder, and delete macros from any browser |
| **Standalone Wi-Fi AP** | No router needed — the device opens its own hotspot |
| **Energy Saving** | Configurable dim timeout, display-off mode, or animated GIF screensaver |
| **GIF Screensaver** | Upload a custom `.gif` via the web UI; rendered with LVGL, scaled to fit |
| **Display Color Theming** | Customize roller background, accent, text colors from the web UI |
| **Web UI Color Theming** | Customize the editor's dark theme colors and reset to defaults |
| **System Log** | In-browser log viewer with auto-refresh and manual clear |
| **Persistent Storage** | Macros saved in LittleFS; settings in NVS (survive reboot) |
| **Remote Restart** | Reboot or enter USB bootloader mode directly from the browser |

---

## 🧰 Hardware

Designed exclusively for the **[M5Dial](https://docs.m5stack.com/en/core/M5Dial)**:

| Component | Details |
|---|---|
| MCU | ESP32-S3 (dual-core, 240 MHz, 8 MB Flash) |
| Display | 240×240 round TFT (GC9A01 driver) |
| Input | Rotary encoder + capacitive touch (FT3267) |
| USB | Native USB-OTG via TinyUSB (HID keyboard) |
| Bluetooth | BLE 5.0 HID keyboard |
| Filesystem | LittleFS (880 KB partition) |

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/fatih4159/M5-Macro.git
cd M5-Macro
```

### 2. Open in Arduino IDE

Open the main sketch file:

```
M5-Macro/M5-Macro.ino
```

### 3. Install Dependencies

Install the following libraries via **Arduino IDE → Tools → Manage Libraries** or the links below.  
Built-in libraries (shipped with the ESP32 Arduino Core) require no separate installation.

#### External Libraries

| Library | Author | Version | Install |
|---|---|---|---|
| **M5Unified** | M5Stack | ≥ 0.2.6 | Arduino Library Manager |
| **M5GFX** | M5Stack | ≥ 0.2.4 | Arduino Library Manager |
| **M5Dial** | M5Stack | ≥ 1.0.3 | Arduino Library Manager |
| **lvgl** | LVGL LLC (kisvegabor) | ≥ 9.2.0 | Arduino Library Manager |
| **ESP32-BLE-Keyboard** | T-vK | 0.3.3-beta | [GitHub](https://github.com/T-vK/ESP32-BLE-Keyboard) |

> **ESP32-BLE-Keyboard** is not in the Arduino Library Manager.  
> Download the `.zip` from GitHub and install via **Sketch → Include Library → Add .ZIP Library**.

#### Built-in Libraries (ESP32 Arduino Core ≥ 3.x)

| Library | Purpose |
|---|---|
| `USB` / `USBHIDKeyboard` | TinyUSB HID keyboard output |
| `WiFi` | Wi-Fi Access Point |
| `WebServer` | HTTP server for the web editor |
| `Preferences` | NVS key-value storage |
| `LittleFS` | Flash filesystem for macros & GIF |

#### LVGL Configuration

Copy or symlink `lv_conf.h` from the project root into the LVGL library folder so it is picked up at compile time:

```
Arduino/libraries/lvgl/lv_conf.h  →  (copy from project root)
```

### 4. Configure Arduino Settings

Open **Tools** in the Arduino IDE and apply the following settings:

| Setting | Value |
|---|---|
| Board | **M5Stack → M5Dial** |
| USB Mode | **USB-OTG (TinyUSB)** |
| USB CDC On Boot | **Disabled** |
| Partition Scheme | **Custom** → select `partitions.csv` |
| Upload Speed | **921600** |
| PSRAM | **OPI PSRAM** |

> ⚠️ **Important:** HID keyboard output will **not work** if _USB CDC On Boot_ is enabled.  
> The custom `partitions.csv` allocates 3 MB for the app and 880 KB for LittleFS.

### 5. Flash the Device

1. Connect the M5Dial via USB-C.
2. If the device does not appear as a COM port, hold **BOOT** and press **RESET** to enter download mode.
3. Select the correct port in Arduino IDE and click **Upload**.
4. After flashing, press **RESET** once to boot into normal mode.

---

## 🎮 Usage

### Encoder Controls

| Action | Result |
|---|---|
| Rotate clockwise | Select next macro |
| Rotate counter-clockwise | Select previous macro |
| Press encoder | Execute selected macro |

### Display Buttons

Two icon buttons appear on the device screen above the roller:

| Button | Function |
|---|---|
| Wi-Fi icon | Toggle the Wi-Fi Access Point on/off |
| Bluetooth icon | Toggle BLE HID output on/off |

An active button is shown with a **white border**. When the display is dimmed (energy-saving mode), the first encoder interaction wakes the display without executing a macro.

---

## 🌐 Web Editor

### Connect to the Device

1. On your phone or computer, connect to the Wi-Fi network:
   - **SSID:** `m5Macro`
   - **Password:** `m5macro1`
2. Open your browser and navigate to **http://192.168.4.1**

The SSID and password can be changed in the **Settings → Wi-Fi** panel of the web editor.

### Editor Features

- **Sidebar** — lists all saved macros with index numbers.
- **Step builder** — add/remove/reorder steps visually (Key, Combo, Text, Delay).
- **Ctrl + S** — keyboard shortcut to save the current macro.
- **Settings modal** — Wi-Fi credentials, energy saving, display colors, web UI colors, system log, restart/bootloader.
- **GIF screensaver upload** — upload a `.gif` file directly from the browser (Settings → Energy Saving → GIF screensaver mode).

---

## 🧠 Macro Syntax

Each macro is stored as plain text. The **first line** is the macro name; all subsequent lines are steps executed in order.

### Step Types

| Syntax | Description | Example |
|---|---|---|
| `TEXT:<string>` | Type a literal string | `TEXT:Hello, World!` |
| `DELAY:<ms>` | Wait in milliseconds | `DELAY:250` |
| `<KEY>` | Press a single key | `ENTER` |
| `<MOD>+<KEY>` | Key combination | `CTRL+C` |
| `<MOD>+<MOD>+<KEY>` | Multi-modifier combo | `CTRL+SHIFT+ESC` |
| `# <comment>` | Line is ignored | `# this is a comment` |

### Supported Keys

**Modifiers:** `CTRL`, `SHIFT`, `ALT`, `WIN` / `GUI`  
(also: `LCTRL`, `RCTRL`, `LSHIFT`, `RSHIFT`, `LALT`, `RALT`, `RWIN`)

**Special keys:**  
`ENTER`, `ESC`, `BACKSPACE`, `TAB`, `SPACE`, `DELETE`, `INSERT`, `CAPSLOCK`

**Navigation:**  
`UP`, `DOWN`, `LEFT`, `RIGHT`, `HOME`, `END`, `PGUP`, `PGDN`

**Function keys:** `F1` – `F12`

**Other:** `PRINTSCREEN`, `SCROLLLOCK`, `PAUSE`, `NUMLOCK`, `BACKTICK` / `TILDE`

**Single characters:** Any single ASCII character (a–z, 0–9, punctuation)

### Example Macro

```
Copy-Paste All
CTRL+A
DELAY:50
CTRL+C
DELAY:100
CTRL+V
```

---

## 🔌 Web API Reference

Base URL: `http://192.168.4.1`

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/` | Web editor HTML |
| `GET` | `/api/macros` | List all macros (JSON) |
| `POST` | `/api/save` | Create or update a macro |
| `POST` | `/api/delete` | Delete a macro by ID |
| `GET` | `/api/status` | Filesystem status & macro count |
| `GET` | `/api/settings` | Get Wi-Fi SSID |
| `POST` | `/api/settings` | Set Wi-Fi SSID & password (triggers restart) |
| `GET` | `/api/energy` | Get energy-saving settings |
| `POST` | `/api/energy` | Update energy-saving settings |
| `GET` | `/api/colors` | Get display (firmware) color theme |
| `POST` | `/api/colors` | Set display color theme |
| `GET` | `/api/webcolors` | Get web UI color theme |
| `POST` | `/api/webcolors` | Set web UI color theme |
| `POST` | `/api/webcolors/reset` | Reset web UI colors to defaults |
| `GET` | `/api/screensaver` | Check if a GIF is uploaded |
| `POST` | `/api/screensaver/upload` | Upload screensaver GIF (multipart) |
| `POST` | `/api/screensaver/delete` | Delete screensaver GIF |
| `GET` | `/api/log` | Get system log entries (JSON) |
| `POST` | `/api/log/clear` | Clear system log |
| `POST` | `/api/force-screensaver` | Immediately trigger energy-saving mode |
| `POST` | `/api/restart` | Restart the device |
| `POST` | `/api/restart-bootloader` | Restart into USB bootloader mode |

---

## ⚙️ Configuration

Compile-time defaults are defined in `config.h`:

| Constant | Default | Description |
|---|---|---|
| `DISPLAY_WIDTH` / `DISPLAY_HEIGHT` | `240` | Display resolution |
| `LV_BUF_LINES` | `40` | LVGL render buffer lines (~19 KB) |
| `ENCODER_PPR` | `4` | Encoder pulses per detent |
| `MACRO_MAX_STEPS` | `128` | Maximum steps per macro |
| `STEP_TEXT_LEN` | `128` | Maximum characters in a TEXT step |
| `KEY_HOLD_MS` | `15` | Key press duration (ms) |
| `STEP_GAP_MS` | `10` | Default delay between steps (ms) |
| `ENERGY_SAVE_TIMEOUT_DEFAULT` | `30` | Seconds of inactivity until dim |
| `ENERGY_SAVE_DIM_BRIGHTNESS` | `10` | Display brightness when dimmed (0–255) |
| `ENERGY_SAVE_ACTIVE_BRIGHTNESS` | `128` | Normal display brightness (0–255) |
| `WIFI_AP_SSID` | `"m5Macro"` | Default Wi-Fi SSID |
| `WIFI_AP_PASS` | `"m5macro1"` | Default Wi-Fi password (min. 8 chars) |
| `WEB_SERVER_PORT` | `80` | HTTP server port |

All energy-saving and Wi-Fi settings can also be changed at runtime through the web editor and are persisted in NVS.

---

## 💾 Storage

| Data | Location | Details |
|---|---|---|
| Macros | LittleFS | One file per macro in the `880 KB` LittleFS partition |
| Screensaver GIF | LittleFS | Stored as `/screensaver.gif` |
| Wi-Fi credentials | NVS (`wifi` namespace) | Survives firmware updates |
| Energy settings | NVS (`energy` namespace) | Survives firmware updates |
| Display colors | NVS (`colors` namespace) | Survives firmware updates |
| Web UI colors | NVS (`webclr` namespace) | Survives firmware updates |

The custom `partitions.csv` layout:

| Partition | Type | Size |
|---|---|---|
| `nvs` | data/nvs | 20 KB |
| `app0` | app/ota_0 | 3 MB |
| `littlefs` | data/spiffs | 880 KB |
| `coredump` | data/coredump | 64 KB |

---

## ⚠️ Limitations

- **Macro execution is blocking** — the UI freezes while a macro runs.
- **No HTTPS** — the web editor runs over plain HTTP.
- **Single output mode per step** — both USB and BLE can be active simultaneously, but are not synchronized (BLE requires an active connection).
- **GIF screensaver uses RAM/PSRAM** — very large GIF files may fail to load on devices without PSRAM.

---

## 📜 License

MIT License — Copyright (c) 2026 **Fatih Tuluk**

See [LICENSE](LICENSE) for the full text.

---

## 💡 Contributing

Pull requests and feature ideas are welcome.  
Please open an issue first for larger changes.
