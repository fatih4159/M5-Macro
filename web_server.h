#pragma once
#include <Arduino.h>

// Initializes the WiFi Access Point and HTTP server.
// Must be called after macro_store_init().
void web_server_init();

// Must be called in every loop() iteration.
// Handles incoming HTTP requests (non-blocking).
void web_server_handle();

// Returns the IP address of the Access Point as a string ("192.168.4.1").
String web_server_ip();

// Returns true if the WiFi AP is currently active.
bool web_server_wifi_enabled();

// Toggles the WiFi AP and HTTP server on/off.
void web_server_wifi_toggle();
