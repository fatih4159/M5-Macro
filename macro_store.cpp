#include "macro_store.h"
#include "sys_log.h"
#include <LittleFS.h>

// ── File paths ────────────────────────────────────────────────────────────────
// Macros are stored as /macros/00.txt … /macros/15.txt.
// Format: line 1 = name, lines 2+ = command steps.
static const char* MACRO_DIR = "/macros";

// ── Internal storage ──────────────────────────────────────────────────────────
static String   s_names[MAX_MACROS];
static String   s_step_strs[MAX_MACROS][MAX_STEPS_STORED];
static int      s_step_counts[MAX_MACROS];
static int      s_count = 0;

// Cache for the externally visible MacroInfo structure
static MacroInfo s_info[MAX_MACROS];

static bool s_fs_ok = false;   // LittleFS mounted successfully?

// ── Default macros (fallback if no filesystem) ────────────────────────────────
struct DefaultMacro {
    const char* name;
    const char* steps;   // '\n'-separated
};

static const DefaultMacro DEFAULTS[] = {
    { "Select all+paste",  "CTRL+A\nDELAY:50\nCTRL+C\nDELAY:100\nCTRL+V"                  },
    { "Save+Close",        "CTRL+S\nDELAY:300\nALT+F4"                                     },
    { "Undo+Redo",         "CTRL+Z\nDELAY:100\nCTRL+Z\nDELAY:100\nCTRL+Y"                 },
    { "Screenshot",        "WIN+SHIFT+S"                                                    },
    { "Lock screen",       "WIN+L"                                                          },
    { "Task Manager",      "CTRL+SHIFT+ESC"                                                 },
    { "VS Code Terminal",  "CTRL+SHIFT+BACKTICK"                                            },
    { "Type signature",    "TEXT:Kind regards\nENTER\nTEXT:Your Name"                       },
};
static const int DEFAULT_COUNT = (int)(sizeof(DEFAULTS) / sizeof(DEFAULTS[0]));

// ── Helper functions ───────────────────────────────────────────────────────────

// Filename for slot i: /macros/00.txt … /macros/15.txt
static String slot_path(int i) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%s/%02d.txt", MACRO_DIR, i);
    return String(buf);
}

// Rebuild MacroInfo cache for slot i from string arrays
static void rebuild_cache(int i) {
    strncpy(s_info[i].name, s_names[i].c_str(), MACRO_NAME_LEN);
    s_info[i].name[MACRO_NAME_LEN] = '\0';
    s_info[i].step_count = s_step_counts[i];
    for (int j = 0; j < s_step_counts[i]; j++) {
        s_info[i].steps[j] = s_step_strs[i][j].c_str();
    }
}

// Parse a macro entry from LittleFS file content
// Format: line 0 = name, lines 1+ = steps
static bool parse_file(int slot, const String& content) {
    int start = 0;
    int lineIdx = 0;
    s_step_counts[slot] = 0;

    while (start <= (int)content.length()) {
        int end = content.indexOf('\n', start);
        if (end == -1) end = content.length();

        String line = content.substring(start, end);
        // Strip Windows line endings
        if (line.endsWith("\r")) line = line.substring(0, line.length() - 1);

        if (lineIdx == 0) {
            s_names[slot] = line;
        } else {
            if (line.length() > 0 && s_step_counts[slot] < MAX_STEPS_STORED) {
                s_step_strs[slot][s_step_counts[slot]++] = line;
            }
        }
        lineIdx++;
        start = end + 1;
    }
    return (s_names[slot].length() > 0);
}

// Write file: name on line 1, steps from line 2
static bool write_file(int slot, const String& name, const String& steps_raw) {
    String path = slot_path(slot);
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    f.println(name);
    f.print(steps_raw);
    // Ensure last line ends with '\n'
    if (!steps_raw.endsWith("\n")) f.println();
    // Explicitly flush to flash before closing.
    // Without flush() a power loss immediately after saving could leave
    // data in the LittleFS buffer and lose it.
    f.flush();
    f.close();
    return true;
}

// Load all macro files from MACRO_DIR (sorted 00 … 15)
static void load_all_from_fs() {
    s_count = 0;
    for (int i = 0; i < MAX_MACROS && s_count < MAX_MACROS; i++) {
        String path = slot_path(i);
        if (!LittleFS.exists(path)) continue;

        File f = LittleFS.open(path, "r");
        if (!f) continue;
        String content = f.readString();
        f.close();

        if (parse_file(s_count, content)) {
            rebuild_cache(s_count);
            s_count++;
        }
    }
}

// Write all default macros to LittleFS and load them
static void write_defaults() {
    for (int i = 0; i < DEFAULT_COUNT && i < MAX_MACROS; i++) {
        write_file(i, String(DEFAULTS[i].name), String(DEFAULTS[i].steps));
    }
    load_all_from_fs();
}

// Load default macros into RAM only (no LittleFS)
static void load_defaults_ram() {
    s_count = 0;
    for (int i = 0; i < DEFAULT_COUNT && i < MAX_MACROS; i++) {
        s_names[i] = DEFAULTS[i].name;
        // Parse steps
        String raw = DEFAULTS[i].steps;
        int start = 0;
        s_step_counts[i] = 0;
        while (start <= (int)raw.length()) {
            int end = raw.indexOf('\n', start);
            if (end == -1) end = raw.length();
            String line = raw.substring(start, end);
            if (line.length() > 0 && s_step_counts[i] < MAX_STEPS_STORED) {
                s_step_strs[i][s_step_counts[i]++] = line;
            }
            start = end + 1;
        }
        rebuild_cache(i);
        s_count++;
    }
}

// ── Public API ───────────────────────────────────────────────────────────────

void macro_store_init() {
    // formatOnFail=true: format partition on first boot or after corruption.
    // LittleFS is power-loss-safe; reformatting only occurs when the
    // filesystem was never initialized or is irreparably damaged.
    if (!LittleFS.begin(true, "/lfs", 10, "littlefs")) {
        // Fallback: try default_8MB-scheme partition label
        if (!LittleFS.begin(true, "/lfs", 10, "spiffs")) {
            sys_log("FS: LittleFS mount FAILED, using RAM defaults");
            load_defaults_ram();
            s_fs_ok = false;
            return;
        }
    }
    s_fs_ok = true;
    sys_log("FS: LittleFS mounted ok, total=%u used=%u",
            (unsigned)LittleFS.totalBytes(), (unsigned)LittleFS.usedBytes());

    // Create directory if needed
    if (!LittleFS.exists(MACRO_DIR)) {
        LittleFS.mkdir(MACRO_DIR);
    }

    // Load macros – if none exist, write defaults
    load_all_from_fs();
    if (s_count == 0) {
        sys_log("FS: no macros found, writing defaults");
        write_defaults();
    }
    sys_log("FS: loaded %d macros", s_count);
}

int macro_store_count() {
    return s_count;
}

const MacroInfo* macro_store_get(int index) {
    if (index < 0 || index >= s_count) return nullptr;
    return &s_info[index];
}

void macro_store_reload() {
    if (s_fs_ok) {
        load_all_from_fs();
    }
}

int macro_store_save(int id, const String& name, const String& steps_raw) {
    if (name.length() == 0) return -1;

    int target_slot;

    if (id == -1) {
        // New macro: find next free slot number
        // Always write sequentially, so next number = s_count
        if (s_count >= MAX_MACROS) return -1;
        target_slot = s_count;
    } else {
        if (id < 0 || id >= s_count) return -1;
        // Existing macro: slot number = id (files were written sequentially,
        // so index = slot number)
        target_slot = id;
    }

    if (!s_fs_ok) {
        // Update RAM only
        s_names[target_slot] = name;
        s_step_counts[target_slot] = 0;
        String raw = steps_raw;
        int start = 0;
        while (start <= (int)raw.length()) {
            int end = raw.indexOf('\n', start);
            if (end == -1) end = raw.length();
            String line = raw.substring(start, end);
            if (line.endsWith("\r")) line = line.substring(0, line.length()-1);
            if (line.length() > 0 && s_step_counts[target_slot] < MAX_STEPS_STORED) {
                s_step_strs[target_slot][s_step_counts[target_slot]++] = line;
            }
            start = end + 1;
        }
        rebuild_cache(target_slot);
        if (id == -1) s_count++;
        return target_slot;
    }

    if (!write_file(target_slot, name, steps_raw)) return -1;
    load_all_from_fs();
    return target_slot;
}

bool macro_store_delete(int id) {
    if (id < 0 || id >= s_count) return false;

    if (!s_fs_ok) {
        // RAM only: shift slots down
        for (int i = id; i < s_count - 1; i++) {
            s_names[i]       = s_names[i + 1];
            s_step_counts[i] = s_step_counts[i + 1];
            for (int j = 0; j < s_step_counts[i]; j++) {
                s_step_strs[i][j] = s_step_strs[i + 1][j];
            }
            rebuild_cache(i);
        }
        s_count--;
        return true;
    }

    // 1. Remove file to delete
    LittleFS.remove(slot_path(id));

    // 2. Rename subsequent files (keep numbering gap-free)
    for (int i = id; i < MAX_MACROS - 1; i++) {
        String next = slot_path(i + 1);
        if (LittleFS.exists(next)) {
            LittleFS.rename(next, slot_path(i));
        } else {
            break;
        }
    }

    load_all_from_fs();
    return true;
}

bool macro_store_fs_ok() {
    return s_fs_ok;
}

String macro_store_list_files() {
    if (!s_fs_ok) return "FS_NOT_MOUNTED";
    String result = "";
    File dir = LittleFS.open(MACRO_DIR);
    if (!dir || !dir.isDirectory()) return "DIR_NOT_FOUND";
    File f = dir.openNextFile();
    while (f) {
        if (result.length() > 0) result += ",";
        result += String(f.name()) + "(" + String(f.size()) + "B)";
        f = dir.openNextFile();
    }
    return result.length() > 0 ? result : "EMPTY";
}

String macro_store_get_steps_raw(int id) {
    if (id < 0 || id >= s_count) return "";
    String result = "";
    for (int j = 0; j < s_step_counts[id]; j++) {
        if (j > 0) result += "\n";
        result += s_step_strs[id][j];
    }
    return result;
}
