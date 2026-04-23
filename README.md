# m5Macro

A compact, programmable **USB HID macro pad** built on the **M5Dial
(ESP32-S3)**.\
Select macros using a rotary encoder and execute them instantly as real
keyboard input --- no drivers required.

Macros are created and managed through a clean, built-in **web
interface**, accessible from any browser.

------------------------------------------------------------------------

## ✨ Features

-   🔌 **Plug & Play USB Keyboard** -- recognized as a standard HID
    device
-   🎛 **Rotary Encoder Control** -- rotate to select, press to execute
-   🖥 **Circular UI** -- smooth macro selection via LVGL roller widget
-   🌐 **Web-Based Editor** -- create and edit macros from your browser
-   📡 **Standalone WiFi AP** -- no router required
-   💾 **Persistent Storage** -- macros saved in LittleFS
-   ⚙️ **Configurable** -- WiFi credentials editable via UI
-   🔁 **Remote Restart** -- reboot or enter bootloader from browser

------------------------------------------------------------------------

## 🧰 Hardware

Designed specifically for the **M5Dial**:

-   ESP32-S3 (dual-core, 240 MHz)
-   240×240 round TFT display (GC9A01)
-   Rotary encoder + capacitive touch
-   Native USB (TinyUSB HID)
-   8 MB flash (LittleFS)

------------------------------------------------------------------------

## 🚀 Getting Started

### 1. Clone the repository

``` bash
git clone https://github.com/<user>/m5Macro.git
```

### 2. Open in Arduino IDE

    m5Macro/m5Macro.ino

### 3. Install dependencies

-   M5Unified\
-   M5GFX\
-   LVGL (≥ 8.x)\
-   TinyUSB (ESP32 core)\
-   LittleFS\
-   Preferences\
-   WebServer

------------------------------------------------------------------------

### 4. Configure Arduino settings

  Setting            Value
  ------------------ -------------------------
  Board              M5Stack → M5Dial
  USB Mode           USB-OTG (TinyUSB)
  USB CDC On Boot    ❌ Disabled
  Partition Scheme   Default (with LittleFS)
  Upload Speed       921600

> ⚠️ HID will NOT work if "USB CDC On Boot" is enabled.

------------------------------------------------------------------------

### 5. Flash the device

-   Connect via USB-C\
-   Enter download mode if needed (BOOT + RESET)\
-   Upload sketch

------------------------------------------------------------------------

## 🎮 Usage

  Action          Result
  --------------- ----------------
  Rotate right    Next macro
  Rotate left     Previous macro
  Press encoder   Execute macro

------------------------------------------------------------------------

## 🌐 Web Editor

### Connect

-   WiFi: `m5Macro`\
-   Password: `m5macro1`\
-   Open: http://192.168.4.1

------------------------------------------------------------------------

## 🧠 Macro Format

    Line 1: Name
    Line 2+: Steps

### Example

    CTRL+A
    DELAY:50
    CTRL+C
    DELAY:100
    CTRL+V

------------------------------------------------------------------------

## 🔌 Web API

Base URL:

    http://192.168.4.1

------------------------------------------------------------------------

## 💾 Storage

-   Macros: LittleFS\
-   Settings: NVS

------------------------------------------------------------------------

## ⚙️ Configuration

Defined in `config.h`.

------------------------------------------------------------------------

## ⚠️ Limitations

-   Blocking execution\
-   No HTTPS

------------------------------------------------------------------------

## 📜 License

MIT License

------------------------------------------------------------------------

## 💡 Contributions

Pull requests and ideas are welcome 🚀
