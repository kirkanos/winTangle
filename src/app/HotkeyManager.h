#pragma once

#include <map>
#include <string>
#include <vector>

#include "config/Config.h"
#include "core/Action.h"
#include "platform/Win32.h"

namespace wintangle {

// Registriert die globalen Hotkeys. Bewusst RegisterHotKey und kein
// WH_KEYBOARD_LL-Hook: der Hook wuerde jeden Tastendruck des Systems durch
// unseren Prozess schleusen (Latenz, Virenscanner-Verdacht) und braucht bei
// erhoehten Fenstern ohnehin dieselben Rechte. Preis dafuer: Kombinationen,
// die Windows selbst belegt (Win+Pfeil), bekommen wir nicht.
class HotkeyManager {
public:
    struct Conflict {
        Action action;
        std::wstring combo;
    };

    explicit HotkeyManager(HWND messageWindow) : window_(messageWindow) {}
    ~HotkeyManager() { UnregisterAll(); }

    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;

    // Registriert alles neu. Gibt die Kombinationen zurueck, die Windows
    // abgelehnt hat (meist bereits von einem anderen Programm belegt) --
    // die App zeigt sie dem Benutzer, statt sie stumm zu schlucken.
    std::vector<Conflict> Apply(const Config& config);

    void UnregisterAll();

    // Aus WM_HOTKEY: liefert die Aktion zur gemeldeten Id.
    bool ActionForId(int id, Action& out) const;

private:
    HWND window_;
    std::map<int, Action> byId_;
};

}  // namespace wintangle
