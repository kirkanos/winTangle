#include "Autostart.h"

#include "Paths.h"
#include "platform/Win32.h"

namespace wintangle {
namespace {

constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kValueName[] = L"WinTangle";

std::wstring QuotedExePath() { return L"\"" + ExecutablePath() + L"\""; }

}  // namespace

bool IsAutostartEnabled() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    DWORD size = 0;
    const LSTATUS status = RegQueryValueExW(key, kValueName, nullptr, &type, nullptr, &size);
    RegCloseKey(key);
    return status == ERROR_SUCCESS && type == REG_SZ;
}

bool SetAutostartEnabled(bool enabled) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRunKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key,
                        nullptr) != ERROR_SUCCESS) {
        return false;
    }

    LSTATUS status;
    if (enabled) {
        const std::wstring value = QuotedExePath();
        status = RegSetValueExW(key, kValueName, 0, REG_SZ,
                                reinterpret_cast<const BYTE*>(value.c_str()),
                                static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    } else {
        status = RegDeleteValueW(key, kValueName);
        if (status == ERROR_FILE_NOT_FOUND) status = ERROR_SUCCESS;  // war schon aus
    }
    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}

}  // namespace wintangle
