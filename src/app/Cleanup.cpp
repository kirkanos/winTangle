#include "Cleanup.h"

#include "Paths.h"
#include "platform/Win32.h"

namespace wintangle {
namespace {

void Note(CleanupReport& report, bool ok, const wchar_t* what) {
    (ok ? report.removed : report.failed).emplace_back(what);
}

// Loescht einen Registry-Baum. "Gab es nicht" zaehlt als Erfolg -- fuer den
// Benutzer ist das Ergebnis dasselbe.
bool DeleteTree(HKEY root, const wchar_t* subkey) {
    const LSTATUS status = RegDeleteTreeW(root, subkey);
    return status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND;
}

bool DeleteValue(HKEY root, const wchar_t* subkey, const wchar_t* name) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(root, subkey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) return true;
    const LSTATUS status = RegDeleteValueW(key, name);
    RegCloseKey(key);
    return status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND;
}

// Bewusst nur die Dateien loeschen, die WinTangle selbst anlegt, statt den
// Ordner rekursiv zu leeren: was sonst noch darin liegt, gehoert uns nicht.
bool RemoveConfigDirectory() {
    const std::wstring dir = AppDataDir();
    if (dir.empty()) return false;

    const wchar_t* files[] = {L"\\config.json", L"\\config.json.tmp"};
    for (const wchar_t* file : files) {
        const std::wstring path = dir + file;
        if (!DeleteFileW(path.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND) return false;
    }

    // Schlaegt fehl, wenn der Ordner nicht leer ist -- dann bleibt er stehen,
    // und das ist richtig so.
    return RemoveDirectoryW(dir.c_str()) != 0 || GetLastError() == ERROR_FILE_NOT_FOUND;
}

// Windows-eigenes Andocken wieder einschalten.
bool RestoreWindowsSnap() {
    const UINT_PTR enabled = TRUE;
    return SystemParametersInfoW(SPI_SETWINARRANGING, 0, reinterpret_cast<void*>(enabled),
                                 SPIF_SENDCHANGE) != FALSE;
}

}  // namespace

CleanupReport RemoveAllTraces() {
    CleanupReport report;

    Note(report, RemoveConfigDirectory(), L"Einstellungen (%APPDATA%\\WinTangle)");
    Note(report,
         DeleteValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                     L"WinTangle"),
         L"Autostart-Eintrag");
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\wintangle"),
         L"URL-Protokoll wintangle://");
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\WinTangle"), L"Update-Zustand");
    Note(report, RestoreWindowsSnap(), L"Windows-eigenes Andocken wieder eingeschaltet");

    return report;
}

std::wstring FormatCleanupReport(const CleanupReport& report) {
    std::wstring text;
    if (!report.removed.empty()) {
        text += L"Entfernt:\n";
        for (const auto& item : report.removed) text += L"  • " + item + L"\n";
    }
    if (!report.failed.empty()) {
        text += L"\nNicht entfernt (läuft WinTangle noch?):\n";
        for (const auto& item : report.failed) text += L"  • " + item + L"\n";
    }
    return text;
}

}  // namespace wintangle
