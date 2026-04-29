#include "macro_store.h"
#include <LittleFS.h>

static const char* MACRO_DIR = "/macros";
static const char* INDEX_PATH = "/macros/index.txt";
static const int ROOT_GROUP_ID = -1;

struct StoredMacro {
    int    id;
    int    parent_group_id;
    String name;
    String steps[MAX_STEPS_STORED];
    int    step_count;
};

struct StoredGroup {
    int    id;
    String name;
};

struct StoredItem {
    int           id;
    int           parent_group_id;
    MacroItemType type;
    int           ref_index;
};

struct DefaultMacro {
    const char* name;
    const char* steps;
};

static const DefaultMacro DEFAULTS[] = {
    { "Select all+paste",  "CTRL+A\nDELAY:50\nCTRL+C\nDELAY:100\nCTRL+V" },
    { "Save+Close",        "CTRL+S\nDELAY:300\nALT+F4" },
    { "Undo+Redo",         "CTRL+Z\nDELAY:100\nCTRL+Z\nDELAY:100\nCTRL+Y" },
    { "Screenshot",        "WIN+SHIFT+S" },
    { "Lock screen",       "WIN+L" },
    { "Task Manager",      "CTRL+SHIFT+ESC" },
    { "VS Code Terminal",  "CTRL+SHIFT+BACKTICK" },
    { "Type signature",    "TEXT:Kind regards\nENTER\nTEXT:Your Name" },
};
static const int DEFAULT_COUNT = (int)(sizeof(DEFAULTS) / sizeof(DEFAULTS[0]));

static StoredMacro     s_macros[MAX_MACROS];
static StoredGroup     s_groups[MAX_GROUPS];
static StoredItem      s_items[MAX_STORE_ITEMS];
static MacroInfo       s_macro_info[MAX_MACROS];
static MacroGroupInfo  s_group_info[MAX_GROUPS];
static MacroItemInfo   s_item_info[MAX_STORE_ITEMS];
static int             s_macro_count = 0;
static int             s_group_count = 0;
static int             s_item_count = 0;
static int             s_next_id = 1;
static bool            s_fs_ok = false;

static String macro_path(int id) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s/%d.txt", MACRO_DIR, id);
    return String(buf);
}

static String escape_field(const String& value) {
    String out;
    for (int i = 0; i < (int)value.length(); i++) {
        char c = value[i];
        if (c == '\\' || c == '\t' || c == '\n' || c == '\r') out += '\\';
        if (c == '\t') out += 't';
        else if (c == '\n') out += 'n';
        else if (c == '\r') out += 'r';
        else out += c;
    }
    return out;
}

static String unescape_field(const String& value) {
    String out;
    bool esc = false;
    for (int i = 0; i < (int)value.length(); i++) {
        char c = value[i];
        if (!esc) {
            if (c == '\\') esc = true;
            else out += c;
            continue;
        }
        esc = false;
        if (c == 't') out += '\t';
        else if (c == 'n') out += '\n';
        else if (c == 'r') out += '\r';
        else out += c;
    }
    if (esc) out += '\\';
    return out;
}

static void clear_store() {
    s_macro_count = 0;
    s_group_count = 0;
    s_item_count = 0;
    s_next_id = 1;
}

static int find_macro_index_by_id(int id) {
    for (int i = 0; i < s_macro_count; i++) {
        if (s_macros[i].id == id) return i;
    }
    return -1;
}

static int find_group_index_by_id(int id) {
    for (int i = 0; i < s_group_count; i++) {
        if (s_groups[i].id == id) return i;
    }
    return -1;
}

static int find_item_index_by_id(int id) {
    for (int i = 0; i < s_item_count; i++) {
        if (s_items[i].id == id) return i;
    }
    return -1;
}

static bool valid_parent_group_id(int parent_group_id) {
    return parent_group_id == ROOT_GROUP_ID || find_group_index_by_id(parent_group_id) >= 0;
}

static void rebuild_macro_cache(int index) {
    s_macro_info[index].id = s_macros[index].id;
    s_macro_info[index].parent_group_id = s_macros[index].parent_group_id;
    strncpy(s_macro_info[index].name, s_macros[index].name.c_str(), MACRO_NAME_LEN);
    s_macro_info[index].name[MACRO_NAME_LEN] = '\0';
    s_macro_info[index].step_count = s_macros[index].step_count;
    for (int i = 0; i < s_macros[index].step_count; i++) {
        s_macro_info[index].steps[i] = s_macros[index].steps[i].c_str();
    }
}

static int count_children(int group_id) {
    int count = 0;
    for (int i = 0; i < s_item_count; i++) {
        if (s_items[i].parent_group_id == group_id) count++;
    }
    return count;
}

static void rebuild_group_cache(int index) {
    s_group_info[index].id = s_groups[index].id;
    strncpy(s_group_info[index].name, s_groups[index].name.c_str(), MACRO_NAME_LEN);
    s_group_info[index].name[MACRO_NAME_LEN] = '\0';
}

static void rebuild_item_cache(int index) {
    const StoredItem& item = s_items[index];
    MacroItemInfo& out = s_item_info[index];
    out.id = item.id;
    out.parent_group_id = item.parent_group_id;
    out.type = item.type;
    out.child_count = 0;

    String name;
    if (item.type == MACRO_ITEM_GROUP) {
        name = s_groups[item.ref_index].name;
        out.child_count = count_children(item.id);
    } else {
        name = s_macros[item.ref_index].name;
    }

    strncpy(out.name, name.c_str(), MACRO_NAME_LEN);
    out.name[MACRO_NAME_LEN] = '\0';
}

static void rebuild_all_caches() {
    for (int i = 0; i < s_macro_count; i++) rebuild_macro_cache(i);
    for (int i = 0; i < s_group_count; i++) rebuild_group_cache(i);
    for (int i = 0; i < s_item_count; i++) rebuild_item_cache(i);
}

static void parse_steps_into(StoredMacro& macro, const String& steps_raw) {
    macro.step_count = 0;
    int start = 0;
    while (start <= (int)steps_raw.length()) {
        int end = steps_raw.indexOf('\n', start);
        if (end == -1) end = steps_raw.length();
        String line = steps_raw.substring(start, end);
        if (line.endsWith("\r")) line = line.substring(0, line.length() - 1);
        if (line.length() > 0 && macro.step_count < MAX_STEPS_STORED) {
            macro.steps[macro.step_count++] = line;
        }
        start = end + 1;
    }
}

static String build_steps_raw(const StoredMacro& macro) {
    String result;
    for (int i = 0; i < macro.step_count; i++) {
        if (i > 0) result += "\n";
        result += macro.steps[i];
    }
    return result;
}

static bool write_macro_steps(const StoredMacro& macro) {
    File file = LittleFS.open(macro_path(macro.id), "w");
    if (!file) return false;
    file.print(build_steps_raw(macro));
    file.flush();
    file.close();
    return true;
}

static bool read_macro_steps(StoredMacro& macro) {
    File file = LittleFS.open(macro_path(macro.id), "r");
    if (!file) return false;
    String content = file.readString();
    file.close();
    parse_steps_into(macro, content);
    return true;
}

static bool persist_all() {
    if (!s_fs_ok) return true;

    File index = LittleFS.open(INDEX_PATH, "w");
    if (!index) return false;
    index.println("M5M2");
    for (int i = 0; i < s_item_count; i++) {
        const StoredItem& item = s_items[i];
        if (item.type == MACRO_ITEM_GROUP) {
            const StoredGroup& group = s_groups[item.ref_index];
            index.printf("G\t%d\t%s\n", group.id, escape_field(group.name).c_str());
        } else {
            const StoredMacro& macro = s_macros[item.ref_index];
            index.printf("M\t%d\t%d\t%s\n", macro.id, macro.parent_group_id, escape_field(macro.name).c_str());
        }
    }
    index.flush();
    index.close();

    for (int i = 0; i < s_macro_count; i++) {
        if (!write_macro_steps(s_macros[i])) return false;
    }

    File dir = LittleFS.open(MACRO_DIR);
    if (dir && dir.isDirectory()) {
        File entry = dir.openNextFile();
        while (entry) {
            String name = String(entry.name());
            bool keep = name == INDEX_PATH;
            if (!keep && name.startsWith(String(MACRO_DIR) + "/")) {
                int slash = name.lastIndexOf('/');
                String base = slash >= 0 ? name.substring(slash + 1) : name;
                if (base != "index.txt" && base.endsWith(".txt")) {
                    String id_part = base.substring(0, base.length() - 4);
                    int id = id_part.toInt();
                    keep = find_macro_index_by_id(id) >= 0;
                }
            }
            entry.close();
            if (!keep) LittleFS.remove(name);
            entry = dir.openNextFile();
        }
    }

    return true;
}

static bool load_v2_from_fs() {
    if (!LittleFS.exists(INDEX_PATH)) return false;

    File index = LittleFS.open(INDEX_PATH, "r");
    if (!index) return false;

    clear_store();
    String header = index.readStringUntil('\n');
    header.trim();
    if (header != "M5M2") {
        index.close();
        return false;
    }

    while (index.available()) {
        String line = index.readStringUntil('\n');
        if (line.endsWith("\r")) line = line.substring(0, line.length() - 1);
        if (line.length() == 0) continue;

        int t1 = line.indexOf('\t');
        int t2 = t1 < 0 ? -1 : line.indexOf('\t', t1 + 1);
        int t3 = t2 < 0 ? -1 : line.indexOf('\t', t2 + 1);
        if (t1 < 0 || t2 < 0) continue;

        char type = line[0];
        if (type == 'G') {
            if (s_group_count >= MAX_GROUPS || s_item_count >= MAX_STORE_ITEMS) continue;
            int id = line.substring(t1 + 1, t2).toInt();
            String name = unescape_field(line.substring(t2 + 1));
            s_groups[s_group_count].id = id;
            s_groups[s_group_count].name = name;
            s_items[s_item_count++] = { id, ROOT_GROUP_ID, MACRO_ITEM_GROUP, s_group_count };
            s_group_count++;
            if (id >= s_next_id) s_next_id = id + 1;
        } else if (type == 'M' && t3 >= 0) {
            if (s_macro_count >= MAX_MACROS || s_item_count >= MAX_STORE_ITEMS) continue;
            int id = line.substring(t1 + 1, t2).toInt();
            int parent_group_id = line.substring(t2 + 1, t3).toInt();
            String name = unescape_field(line.substring(t3 + 1));
            s_macros[s_macro_count].id = id;
            s_macros[s_macro_count].parent_group_id = parent_group_id;
            s_macros[s_macro_count].name = name;
            if (!read_macro_steps(s_macros[s_macro_count])) {
                s_macros[s_macro_count].step_count = 0;
            }
            s_items[s_item_count++] = { id, parent_group_id, MACRO_ITEM_MACRO, s_macro_count };
            s_macro_count++;
            if (id >= s_next_id) s_next_id = id + 1;
        }
    }
    index.close();

    for (int i = 0; i < s_macro_count; i++) {
        if (!valid_parent_group_id(s_macros[i].parent_group_id)) {
            s_macros[i].parent_group_id = ROOT_GROUP_ID;
            int item_index = find_item_index_by_id(s_macros[i].id);
            if (item_index >= 0) s_items[item_index].parent_group_id = ROOT_GROUP_ID;
        }
    }

    rebuild_all_caches();
    return true;
}

static bool load_legacy_from_fs() {
    clear_store();
    for (int slot = 0; slot < MAX_MACROS; slot++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s/%02d.txt", MACRO_DIR, slot);
        String path(buf);
        if (!LittleFS.exists(path)) continue;

        File file = LittleFS.open(path, "r");
        if (!file) continue;
        String content = file.readString();
        file.close();

        int nl = content.indexOf('\n');
        String name = nl >= 0 ? content.substring(0, nl) : content;
        if (name.endsWith("\r")) name = name.substring(0, name.length() - 1);
        if (name.length() == 0 || s_macro_count >= MAX_MACROS || s_item_count >= MAX_STORE_ITEMS) continue;

        StoredMacro& macro = s_macros[s_macro_count];
        macro.id = s_next_id++;
        macro.parent_group_id = ROOT_GROUP_ID;
        macro.name = name;
        parse_steps_into(macro, nl >= 0 ? content.substring(nl + 1) : "");
        s_items[s_item_count++] = { macro.id, ROOT_GROUP_ID, MACRO_ITEM_MACRO, s_macro_count };
        s_macro_count++;
    }

    rebuild_all_caches();
    return s_macro_count > 0;
}

static void load_defaults_ram() {
    clear_store();
    for (int i = 0; i < DEFAULT_COUNT && i < MAX_MACROS; i++) {
        StoredMacro& macro = s_macros[s_macro_count];
        macro.id = s_next_id++;
        macro.parent_group_id = ROOT_GROUP_ID;
        macro.name = DEFAULTS[i].name;
        parse_steps_into(macro, DEFAULTS[i].steps);
        s_items[s_item_count++] = { macro.id, ROOT_GROUP_ID, MACRO_ITEM_MACRO, s_macro_count };
        s_macro_count++;
    }
    rebuild_all_caches();
}

static void write_defaults() {
    load_defaults_ram();
    persist_all();
}

void macro_store_init() {
    if (!LittleFS.begin(true, "/lfs", 10, "littlefs")) {
        if (!LittleFS.begin(true, "/lfs", 10, "spiffs")) {
            load_defaults_ram();
            s_fs_ok = false;
            return;
        }
    }
    s_fs_ok = true;

    if (!LittleFS.exists(MACRO_DIR)) {
        LittleFS.mkdir(MACRO_DIR);
    }

    if (!load_v2_from_fs()) {
        if (load_legacy_from_fs()) persist_all();
        else write_defaults();
    }
}

void macro_store_reload() {
    if (s_fs_ok) load_v2_from_fs();
}

int macro_store_macro_count() {
    return s_macro_count;
}

const MacroInfo* macro_store_get_macro(int index) {
    if (index < 0 || index >= s_macro_count) return nullptr;
    return &s_macro_info[index];
}

const MacroInfo* macro_store_get_macro_by_id(int id) {
    int index = find_macro_index_by_id(id);
    return index >= 0 ? &s_macro_info[index] : nullptr;
}

String macro_store_get_steps_raw_by_id(int id) {
    int index = find_macro_index_by_id(id);
    if (index < 0) return "";
    return build_steps_raw(s_macros[index]);
}

int macro_store_group_count() {
    return s_group_count;
}

const MacroGroupInfo* macro_store_get_group(int index) {
    if (index < 0 || index >= s_group_count) return nullptr;
    return &s_group_info[index];
}

const MacroGroupInfo* macro_store_get_group_by_id(int id) {
    int index = find_group_index_by_id(id);
    return index >= 0 ? &s_group_info[index] : nullptr;
}

int macro_store_item_count(int parent_group_id) {
    int count = 0;
    for (int i = 0; i < s_item_count; i++) {
        if (s_items[i].parent_group_id == parent_group_id) count++;
    }
    return count;
}

const MacroItemInfo* macro_store_get_item(int parent_group_id, int index) {
    int seen = 0;
    for (int i = 0; i < s_item_count; i++) {
        if (s_items[i].parent_group_id != parent_group_id) continue;
        if (seen == index) return &s_item_info[i];
        seen++;
    }
    return nullptr;
}

int macro_store_save_macro(int id, const String& name, const String& steps_raw, int parent_group_id) {
    String trimmed = name;
    trimmed.trim();
    if (trimmed.length() == 0) return -1;
    if (!valid_parent_group_id(parent_group_id)) return -1;

    int macro_index = find_macro_index_by_id(id);
    if (macro_index < 0) {
        if (s_macro_count >= MAX_MACROS || s_item_count >= MAX_STORE_ITEMS) return -1;
        StoredMacro& macro = s_macros[s_macro_count];
        macro.id = s_next_id++;
        macro.parent_group_id = parent_group_id;
        macro.name = trimmed;
        parse_steps_into(macro, steps_raw);
        s_items[s_item_count++] = { macro.id, parent_group_id, MACRO_ITEM_MACRO, s_macro_count };
        rebuild_macro_cache(s_macro_count);
        rebuild_item_cache(s_item_count - 1);
        s_macro_count++;
        if (!persist_all()) return -1;
        rebuild_all_caches();
        return macro.id;
    }

    s_macros[macro_index].name = trimmed;
    s_macros[macro_index].parent_group_id = parent_group_id;
    parse_steps_into(s_macros[macro_index], steps_raw);

    int item_index = find_item_index_by_id(id);
    if (item_index >= 0) s_items[item_index].parent_group_id = parent_group_id;

    if (!persist_all()) return -1;
    rebuild_all_caches();
    return id;
}

int macro_store_save_group(int id, const String& name) {
    String trimmed = name;
    trimmed.trim();
    if (trimmed.length() == 0) return -1;

    int group_index = find_group_index_by_id(id);
    if (group_index < 0) {
        if (s_group_count >= MAX_GROUPS || s_item_count >= MAX_STORE_ITEMS) return -1;
        StoredGroup& group = s_groups[s_group_count];
        group.id = s_next_id++;
        group.name = trimmed;
        s_items[s_item_count++] = { group.id, ROOT_GROUP_ID, MACRO_ITEM_GROUP, s_group_count };
        rebuild_group_cache(s_group_count);
        rebuild_item_cache(s_item_count - 1);
        s_group_count++;
        if (!persist_all()) return -1;
        rebuild_all_caches();
        return group.id;
    }

    s_groups[group_index].name = trimmed;
    if (!persist_all()) return -1;
    rebuild_all_caches();
    return id;
}

bool macro_store_delete_item(int id) {
    int item_index = find_item_index_by_id(id);
    if (item_index < 0) return false;

    if (s_items[item_index].type == MACRO_ITEM_GROUP) {
        if (count_children(id) > 0) return false;
        int group_index = s_items[item_index].ref_index;
        for (int i = group_index; i < s_group_count - 1; i++) {
            s_groups[i] = s_groups[i + 1];
        }
        s_group_count--;
        for (int i = 0; i < s_item_count; i++) {
            if (s_items[i].type == MACRO_ITEM_GROUP && s_items[i].ref_index > group_index) {
                s_items[i].ref_index--;
            }
        }
    } else {
        int macro_index = s_items[item_index].ref_index;
        int macro_id = s_macros[macro_index].id;
        if (s_fs_ok) LittleFS.remove(macro_path(macro_id));
        for (int i = macro_index; i < s_macro_count - 1; i++) {
            s_macros[i] = s_macros[i + 1];
        }
        s_macro_count--;
        for (int i = 0; i < s_item_count; i++) {
            if (s_items[i].type == MACRO_ITEM_MACRO && s_items[i].ref_index > macro_index) {
                s_items[i].ref_index--;
            }
        }
    }

    for (int i = item_index; i < s_item_count - 1; i++) {
        s_items[i] = s_items[i + 1];
    }
    s_item_count--;

    if (!persist_all()) return false;
    rebuild_all_caches();
    return true;
}

bool macro_store_move_item(int id, int direction) {
    if (direction != -1 && direction != 1) return false;
    int item_index = find_item_index_by_id(id);
    if (item_index < 0) return false;

    int target = item_index + direction;
    while (target >= 0 && target < s_item_count) {
        if (s_items[target].parent_group_id == s_items[item_index].parent_group_id) break;
        target += direction;
    }
    if (target < 0 || target >= s_item_count) return false;
    if (s_items[target].parent_group_id != s_items[item_index].parent_group_id) return false;

    StoredItem tmp = s_items[item_index];
    s_items[item_index] = s_items[target];
    s_items[target] = tmp;

    if (!persist_all()) return false;
    rebuild_all_caches();
    return true;
}

bool macro_store_fs_ok() {
    return s_fs_ok;
}

String macro_store_list_files() {
    if (!s_fs_ok) return "FS_NOT_MOUNTED";
    File dir = LittleFS.open(MACRO_DIR);
    if (!dir || !dir.isDirectory()) return "DIR_NOT_FOUND";

    String result;
    File file = dir.openNextFile();
    while (file) {
        if (result.length() > 0) result += ",";
        result += String(file.name()) + "(" + String(file.size()) + "B)";
        file = dir.openNextFile();
    }
    return result.length() > 0 ? result : "EMPTY";
}
