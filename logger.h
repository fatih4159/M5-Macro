#pragma once
#include <Arduino.h>

enum LogLevel : uint8_t { LOG_INFO = 0, LOG_WARN = 1, LOG_ERROR = 2 };

void   logger_write(LogLevel level, const char* tag, const char* fmt, ...);
String logger_get_json();
void   logger_clear();

#define LOG_I(tag, ...) logger_write(LOG_INFO,  tag, __VA_ARGS__)
#define LOG_W(tag, ...) logger_write(LOG_WARN,  tag, __VA_ARGS__)
#define LOG_E(tag, ...) logger_write(LOG_ERROR, tag, __VA_ARGS__)
