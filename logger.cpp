#include "logger.h"
#include <stdarg.h>

static constexpr int LOG_CAP  = 60;   // ring buffer capacity
static constexpr int MSG_SIZE = 120;  // max chars per message (incl. null)

struct LogEntry {
    uint32_t ts;
    uint8_t  level;
    char     msg[MSG_SIZE];
};

static LogEntry s_buf[LOG_CAP];
static int      s_next  = 0;  // next write slot
static int      s_total = 0;  // total messages ever written

void logger_write(LogLevel level, const char* tag, const char* fmt, ...) {
    LogEntry& e = s_buf[s_next];
    e.ts    = millis();
    e.level = (uint8_t)level;

    char body[MSG_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(body, sizeof(body), fmt, args);
    va_end(args);
    snprintf(e.msg, MSG_SIZE, "[%s] %s", tag, body);

    s_next = (s_next + 1) % LOG_CAP;
    s_total++;
}

String logger_get_json() {
    int count = (s_total < LOG_CAP) ? s_total : LOG_CAP;
    int start = (s_total < LOG_CAP) ? 0 : s_next;  // oldest entry

    String out;
    out.reserve(count * (MSG_SIZE + 36));
    out += '[';
    for (int i = 0; i < count; i++) {
        const LogEntry& e = s_buf[(start + i) % LOG_CAP];
        if (i) out += ',';
        out += "{\"ts\":";
        out += e.ts;
        out += ",\"l\":";
        out += e.level;
        out += ",\"m\":\"";
        for (const char* p = e.msg; *p; p++) {
            if      (*p == '"')  { out += '\\'; out += '"'; }
            else if (*p == '\\') { out += '\\'; out += '\\'; }
            else if (*p == '\n') { out += '\\'; out += 'n'; }
            else                   out += *p;
        }
        out += "\"}";
    }
    out += ']';
    return out;
}

void logger_clear() {
    s_next  = 0;
    s_total = 0;
}
