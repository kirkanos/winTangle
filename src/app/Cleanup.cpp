#include "Cleanup.h"

#include "Paths.h"
#include "platform/Win32.h"

namespace wintangle {
namespace {

void Note(CleanupReport& report, bool ok, const wchar_t* what) {
    (ok ? report.removed : report.failed).emplace_back(what);
}

// Deletes a registry tree. "It was not there" counts as success -- from the
// user's point of view the result is the same.
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

// Deliberately delete only the files WinTangle creates itself instead of
// emptying the folder recursively: whatever else is in there is not ours.
bool RemoveConfigDirectory() {
    const std::wstring dir = AppDataDir();
    if (dir.empty()) return false;

    const wchar_t* files[] = {L"\\config.json", L"\\config.json.tmp"};
    for (const wchar_t* file : files) {
        const std::wstring path = dir + file;
        if (!DeleteFileW(path.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND) return false;
    }

    // Fails when the folder is not empty -- in which case it stays, and that
    // is the right outcome.
    return RemoveDirectoryW(dir.c_str()) != 0 || GetLastError() == ERROR_FILE_NOT_FOUND;
}

// Switch Windows' own snapping back on.
bool RestoreWindowsSnap() {
    const UINT_PTR enabled = TRUE;
    return SystemParametersInfoW(SPI_SETWINARRANGING, 0, reinterpret_cast<void*>(enabled),
                                 SPIF_SENDCHANGE) != FALSE;
}

}  // namespace

CleanupReport RemoveAllTraces() {
    CleanupReport report;

    Note(report, RemoveConfigDirectory(), L"Settings (%APPDATA%\\WinTangle)");
    Note(report,
         DeleteValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                     L"WinTangle"),
         L"Autostart entry");
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\wintangle"),
         L"URL protocol wintangle://");
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\WinTangle"), L"Update state");
    Note(report, RestoreWindowsSnap(), L"Windows' own snapping switched back on");

    return report;
}

std::wstring FormatCleanupReport(const CleanupReport& report) {
    std::wstring text;
    if (!report.removed.empty()) {
        text += L"Removed:\n";
        for (const auto& item : report.removed) text += L"  • " + item + L"\n";
    }
    if (!report.failed.empty()) {
        text += L"\nNot removed (is WinTangle still running?):\n";
        for (const auto& item : report.failed) text += L"  • " + item + L"\n";
    }
    return text;
}

}  // namespace wintangle
