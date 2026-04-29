#pragma once
#include <Arduino.h>
#include "config.h"

#define MAX_MACROS       16
#define MAX_GROUPS       8
#define MAX_STORE_ITEMS  (MAX_MACROS + MAX_GROUPS)
#define MAX_STEPS_STORED 32
#define MACRO_NAME_LEN   32

struct MacroInfo {
    int         id;
    int         parent_group_id;
    char        name[MACRO_NAME_LEN + 1];
    const char* steps[MAX_STEPS_STORED];
    int         step_count;
};

struct MacroGroupInfo {
    int  id;
    char name[MACRO_NAME_LEN + 1];
};

enum MacroItemType {
    MACRO_ITEM_MACRO = 0,
    MACRO_ITEM_GROUP = 1,
};

struct MacroItemInfo {
    int           id;
    int           parent_group_id;
    MacroItemType type;
    char          name[MACRO_NAME_LEN + 1];
    int           child_count;
};

void             macro_store_init();
void             macro_store_reload();

int              macro_store_macro_count();
const MacroInfo* macro_store_get_macro(int index);
const MacroInfo* macro_store_get_macro_by_id(int id);
String           macro_store_get_steps_raw_by_id(int id);

int              macro_store_group_count();
const MacroGroupInfo* macro_store_get_group(int index);
const MacroGroupInfo* macro_store_get_group_by_id(int id);

int              macro_store_item_count(int parent_group_id);
const MacroItemInfo* macro_store_get_item(int parent_group_id, int index);

int              macro_store_save_macro(int id, const String& name, const String& steps_raw, int parent_group_id);
int              macro_store_save_group(int id, const String& name);
bool             macro_store_delete_item(int id);
bool             macro_store_move_item(int id, int direction);

bool             macro_store_fs_ok();
String           macro_store_list_files();
