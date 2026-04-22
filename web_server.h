#pragma once
#include <Arduino.h>

// Initialisiert den WiFi-Access-Point und den HTTP-Server.
// Muss nach macro_store_init() aufgerufen werden.
void web_server_init();

// Muss in jedem loop()-Durchlauf aufgerufen werden.
// Verarbeitet eingehende HTTP-Anfragen (nicht-blockierend).
void web_server_handle();

// Gibt die IP-Adresse des Access Points als String zurueck ("192.168.4.1").
String web_server_ip();
