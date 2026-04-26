#pragma once
#include <Arduino.h>

// Simple circular log buffer – call sys_log() from any .cpp.
// All entries are stored in SRAM (no filesystem, no Serial).
// Read them back via /api/log over the web UI.

void    sys_log(const char* fmt, ...);
void    sys_log_clear();
String  sys_log_get_json();   // JSON array of {ts, msg} objects
