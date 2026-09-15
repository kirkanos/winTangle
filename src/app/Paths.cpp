#include "Paths.h"

#include <shlobj.h>

namespace wintangle {

std::wstring AppDataDir() {
    static const std::wstring dir = [] {
        PWSTR roaming = nullptr;
        std::wstring path;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming))) {
            path = roaming;
            CoTaskMemFree(roaming);
        }
        if (path.empty()) return std::wstring();
        path += L"\\WinTangle";
        CreateDirectoryW(path.c_str(), nullptr);  // already existing is fine
        return path;
    }();
    return dir;
}

std::wstring ConfigPath() {
    const std::wstring dir = AppDataDir();
    return dir.empty() ? std::wstring() : dir + L"\\config.json";
}

std::wstring ExecutablePath() {
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0) return {};
        if (len < path.size()) {
            path.resize(len);
            return path;
        }
        path.resize(path.size() * 2);  // path longer than MAX_PATH
    }
}

}  // namespace wintangle
