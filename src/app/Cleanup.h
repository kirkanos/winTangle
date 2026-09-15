#pragma once

#include <string>
#include <vector>

namespace wintangle {

// Entfernt restlos alle Spuren, die WinTangle auf dem System hinterlaesst.
//
// Es sind genau fuenf, und sie stehen alle unter HKEY_CURRENT_USER bzw. im
// Benutzerprofil -- WinTangle schreibt nirgends systemweit:
//
//   1. %APPDATA%\WinTangle\            Konfiguration
//   2. HKCU\...\CurrentVersion\Run     Autostart-Eintrag
//   3. HKCU\Software\Classes\wintangle URL-Protokoll
//   4. HKCU\Software\WinTangle         Update-Zustand von WinSparkle
//   5. SPI_SETWINARRANGING             Windows-eigenes Andocken, falls das
//                                      Programm es abgeschaltet hat
//
// Punkt 5 ist der leicht zu uebersehende: das ist eine Windows-Einstellung,
// die WinTangle veraendert haben kann. Sie bleibt sonst nach der
// Deinstallation abgeschaltet zurueck.
//
// Diese Funktion ist die einzige Stelle, an der diese Liste gepflegt wird --
// der Uninstaller ruft sie ueber "wintangle.exe --cleanup" auf, statt die
// Loeschungen in seinem eigenen Skript zu wiederholen.
struct CleanupReport {
    std::vector<std::wstring> removed;  // was tatsaechlich entfernt wurde
    std::vector<std::wstring> failed;   // was nicht entfernt werden konnte

    bool AnythingRemoved() const { return !removed.empty(); }
};

CleanupReport RemoveAllTraces();

// Aufbereitet fuer eine Meldung an den Benutzer.
std::wstring FormatCleanupReport(const CleanupReport& report);

}  // namespace wintangle
