#pragma once
#include <Arduino.h>
#include "config.h"

// ── Speicher-Limits ──────────────────────────────────────────────────────────
#define MAX_MACROS       16   // Maximale Anzahl Makros
#define MAX_STEPS_STORED 32   // Maximale Schritte pro Makro (Speicher)
#define MACRO_NAME_LEN   32   // Maximale Namenlaenge (ohne Nullterminator)

// ── Makro-Metadaten ──────────────────────────────────────────────────────────
// steps[i] zeigt auf interne String-Puffer – gueltig bis zur naechsten
// Aenderung im Store. Nicht ueber Reload-Grenzen hinaus cachen!
struct MacroInfo {
    char        name[MACRO_NAME_LEN + 1];   // Anzeigename (nullterminiert)
    const char* steps[MAX_STEPS_STORED];     // Zeiger auf Befehlszeilen
    int         step_count;                  // Anzahl belegter Eintraege
};

// ── Oeffentliche Basis-API ───────────────────────────────────────────────────

// Initialisiert LittleFS und laedt alle Makros aus /macros/*.txt.
// Legt Default-Makros an, falls noch keine Dateien existieren.
void             macro_store_init();

// Anzahl der geladenen Makros (0 … MAX_MACROS).
int              macro_store_count();

// Zeiger auf Makro-Metadaten (nur bis zur naechsten Store-Aenderung gueltig).
const MacroInfo* macro_store_get(int index);

// Laedt alle Makros neu von LittleFS (z. B. nach Web-Aenderung).
void             macro_store_reload();

// ── Erweiterungen fuer den Web-Editor ───────────────────────────────────────

// Speichert ein Makro auf LittleFS und aktualisiert den internen Cache.
//   id       : Positionsindex (0-basiert) des bestehenden Makros,
//              oder -1 um ein neues Makro anzulegen.
//   name     : Anzeigename (max. MACRO_NAME_LEN Zeichen).
//   steps_raw: Alle Befehlszeilen, durch '\n' getrennt.
// Gibt den Index des gespeicherten Makros zurueck, oder -1 bei Fehler.
int              macro_store_save(int id, const String& name, const String& steps_raw);

// Loescht das Makro mit dem angegebenen Index und nummeriert die Dateien neu.
// Gibt true zurueck bei Erfolg.
bool             macro_store_delete(int id);

// Gibt alle Schritte eines Makros als '\n'-getrennten String zurueck
// (fuer das Textarea-Feld im Web-Editor).
String           macro_store_get_steps_raw(int id);

// Gibt true zurueck wenn LittleFS erfolgreich gemountet wurde (Diagnose).
bool             macro_store_fs_ok();

// Gibt eine Liste der Dateien in /macros/ als kommagetrennte String zurueck (Diagnose).
String           macro_store_list_files();
