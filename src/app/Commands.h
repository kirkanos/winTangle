#pragma once

#include "platform/Win32.h"

namespace wintangle {

// Menue- und Nachrichten-Ids an einer Stelle, damit sich Tray-Menue,
// Einstellungsdialog und Fensterprozedur nicht ins Gehege kommen.
enum : unsigned {
    // Aktionen im Tray-Menue: kCmdActionBase + Index in AllActions().
    kCmdActionBase = 1000,

    kCmdSettings = 100,
    kCmdToggleSnapAreas = 101,
    kCmdToggleAutostart = 102,
    kCmdToggleCycleSizes = 103,
    kCmdAbout = 104,
    kCmdQuit = 105,
};

// Eigene Fensternachrichten.
enum : unsigned {
    kMsgTrayCallback = WM_APP + 1,   // Klick auf das Tray-Symbol
    kMsgUriCommand = WM_APP + 2,     // wintangle://-Aufruf aus einer zweiten Instanz
    kMsgTaskbarCreated = WM_APP + 3, // Explorer neu gestartet (wird umgemappt)
    kMsgConfigChanged = WM_APP + 4,  // Einstellungen wurden gespeichert
};

}  // namespace wintangle
