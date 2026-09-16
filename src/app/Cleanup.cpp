#include "Cleanup.h"

#include "Paths.h"
#include "WindowsSnap.h"
#include "platform/Win32.h"

namespace wintangle {
namespace {

void Note(CleanupReport& report, bool ok, Str what) {
    (ok ? report.removed : report.failed).push_back(what);
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

// Put Windows' own snapping back the way the user had it. Going through the
// same bookkeeping as everywhere else means a user who switched it off
// themselves keeps it off.
bool RestoreWindowsSnap() {
    ApplyWindowArrangingPreference(false);
    return true;
}

}  // namespace

CleanupReport RemoveAllTraces() {
    CleanupReport report;

    Note(report, RemoveConfigDirectory(), Str::TraceSettings);
    Note(report,
         DeleteValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                     L"WinTangle"),
         Str::TraceAutostart);
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\wintangle"),
         Str::TraceUrlProtocol);
    Note(report, DeleteTree(HKEY_CURRENT_USER, L"Software\\WinTangle"), Str::TraceUpdateState);
    Note(report, RestoreWindowsSnap(), Str::TraceWindowsSnap);

    return report;
}

std::wstring FormatCleanupReport(const CleanupReport& report) {
    std::wstring text;
    if (!report.removed.empty()) {
        text += T(Str::CleanupRemoved) + L"\n";
        for (Str item : report.removed) text += L"  • " + T(item) + L"\n";
    }
    if (!report.failed.empty()) {
        text += L"\n" + T(Str::CleanupNotRemoved) + L"\n";
        for (Str item : report.failed) text += L"  • " + T(item) + L"\n";
    }
    return text;
}

}  // namespace wintangle
