#pragma once

#include <string>
#include <string_view>

namespace wintangle {

// Modifier-Bits sind absichtlich identisch zu den MOD_*-Werten von
// RegisterHotKey, damit zwischen Konfiguration und Win32 nichts umgerechnet
// werden muss.
enum Modifier : unsigned {
    kModAlt = 0x0001,
    kModCtrl = 0x0002,
    kModShift = 0x0004,
    kModWin = 0x0008,
};

// Eine Tastenkombination: Modifier plus ein Virtual-Key-Code.
struct Shortcut {
    unsigned mods = 0;
    unsigned vk = 0;

    bool IsValid() const { return vk != 0; }
    bool operator==(const Shortcut& o) const { return mods == o.mods && vk == o.vk; }
    bool operator!=(const Shortcut& o) const { return !(*this == o); }
};

// "Ctrl+Alt+Left" -> Shortcut. Gross-/Kleinschreibung egal, "Control", "Strg",
// "Win"/"Super"/"Meta" und "Option" werden als Synonyme akzeptiert.
// Gibt false zurueck, wenn die Taste unbekannt ist oder kein Modifier dabei
// ist -- ein globaler Hotkey ohne Modifier wuerde die Tastatur blockieren.
bool ParseShortcut(std::string_view text, Shortcut& out);

// Kanonische Schreibweise, so wie sie in die Konfigdatei geschrieben und im
// Einstellungsdialog angezeigt wird: "Ctrl+Alt+Shift+Win+Taste".
std::string FormatShortcut(const Shortcut& s);

// Anzeigename eines Virtual-Key-Codes ("Left", "F3", "A"), leer bei unbekannt.
std::string_view KeyName(unsigned vk);

}  // namespace wintangle
