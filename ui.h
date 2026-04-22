#pragma once
#include <lvgl.h>

// Erstellt die LVGL-Oberflaeche fuer den Makro-Selektor.
// Muss nach macro_store_init() aufgerufen werden.
void ui_init();

// Setzt den Anzeigestatus auf "Makro wird ausgefuehrt".
// Erzwingt einen LVGL-Render-Pass (lv_timer_handler).
void ui_show_running(int index);

// Setzt den Anzeigestatus zurueck auf "Bereit".
void ui_show_idle();

// Gibt den aktuell im Roller ausgewaehlten Index zurueck.
int  ui_get_selected();

// Setzt den Roller-Index programmatisch (z.B. nach Reload).
void ui_set_selected(int index);

// Baut den Roller-Inhalt aus dem aktuellen macro_store neu auf.
// Aufruf nach jeder Makro-Aenderung ueber den Web-Editor.
void ui_reload();

// Setzt den Hinweistext in der Statuszeile (z.B. WLAN-Adresse).
void ui_set_hint(const char* text);
