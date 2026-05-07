#include "hid_usb.h"
#include "USB.h"
#include "USBHIDKeyboard.h"

static USBHIDKeyboard s_usb_keyboard;

void hid_usb_init() {
    USB.productName("m5Macro Keyboard");
    USB.manufacturerName("m5Stack");
    USB.begin();
    s_usb_keyboard.begin();
}

void hid_usb_print(const char* text) {
    if (text) {
        s_usb_keyboard.print(text);
    }
}

void hid_usb_press(uint8_t keycode) {
    s_usb_keyboard.press(keycode);
}

void hid_usb_release_all() {
    s_usb_keyboard.releaseAll();
}
