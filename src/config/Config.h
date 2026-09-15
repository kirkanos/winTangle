#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "core/Action.h"
#include "core/Rect.h"
#include "core/Shortcut.h"

namespace wintangle {

// Die komplette Konfiguration. Plattformfrei, damit Laden/Speichern/Migrieren
// ohne Windows testbar ist; nur der Pfad zur Datei kommt aus dem Win32-Teil.
struct Config {
    // Version des Dateiformats, damit spaetere Migrationen moeglich sind.
    static constexpr int kVersion = 1;

    Gaps gaps;
    bool cycleSizes = true;       // wiederholter Druck zykliert 1/2 -> 2/3 -> 1/3
    bool snapAreasEnabled = true; // Drag an den Bildschirmrand
    bool disableWindowsSnap = false;  // Windows-eigenes AeroSnap abschalten
    bool launchAtLogin = false;
    bool moveCursorWithWindow = false;

    // Exe-Namen (klein geschrieben, ohne Pfad), fuer die WinTangle nichts tut.
    std::vector<std::string> ignoredApps;

    // Nur belegte Aktionen stehen drin. Eine Aktion ohne Eintrag hat bewusst
    // keinen Hotkey.
    std::map<Action, Shortcut> shortcuts;

    // Rectangle-Vorgaben, uebersetzt nach Windows: ⌃⌥ wird zu Ctrl+Alt,
    // ⌘ zu Win. Win+Pfeil bleibt frei, weil Windows das selbst belegt.
    static Config Defaults();

    std::optional<Shortcut> ShortcutFor(Action a) const;
    bool IsIgnored(std::string_view exeName) const;

    std::string ToJson() const;

    // Liest die Konfiguration. Unbekannte Felder werden ignoriert, fehlende
    // aus den Vorgaben ergaenzt. `warnings` sammelt alles, was uebersprungen
    // wurde (unbekannte Aktion, unlesbarer Shortcut) -- die Datei wird
    // deswegen nie verworfen.
    static bool FromJson(std::string_view text, Config& out, std::string& error,
                         std::vector<std::string>* warnings = nullptr);

    bool SaveToFile(const std::string& path, std::string& error) const;
    static bool LoadFromFile(const std::string& path, Config& out, std::string& error,
                             std::vector<std::string>* warnings = nullptr);
};

}  // namespace wintangle
