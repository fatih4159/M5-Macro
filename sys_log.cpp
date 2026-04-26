#include "sys_log.h"
#include <stdarg.h>

// ── Circular log buffer ───────────────────────────────────────────────────────
// ~6 KB of SRAM total.  Oldest entry is overwritten when full.

static constexpr int LOG_CAP     = 64;   // maximum entries kept
static constexpr int LOG_MSG_LEN = 100;  // max chars per message (incl. NUL)

struct LogEntry {
    uint32_t ts_ms;
    char     msg[LOG_MSG_LEN];
};

static LogEntry s_buf[LOG_CAP];
static int      s_head  = 0;   // next write index (ring)
static int      s_count = 0;   // entries present (0 .. LOG_CAP)

// ── Public API ────────────────────────────────────────────────────────────────

void sys_log(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LogEntry& e = s_buf[s_head];
    e.ts_ms = millis();
    vsnprintf(e.msg, LOG_MSG_LEN, fmt, ap);
    va_end(ap);
    s_head = (s_head + 1) % LOG_CAP;
    if (s_count < LOG_CAP) s_count++;
}

void sys_log_clear() {
    s_head  = 0;
    s_count = 0;
}

String sys_log_get_json() {
    String out;
    out.reserve(s_count * 60);
    out += '[';
    // oldest → newest
    int start = (s_count < LOG_CAP) ? 0 : s_head;
    for (int i = 0; i < s_count; i++) {
        const LogEntry& e = s_buf[(start + i) % LOG_CAP];
        if (i) out += ',';
        out += "{\"ts\":";
        out += e.ts_ms;
        out += ",\"msg\":\"";
        for (const char* p = e.msg; *p; p++) {
            switch (*p) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': break;
                default:   out += *p;
            }
        }
        out += "\"}";
    }
    out += ']';
    return out;
}
