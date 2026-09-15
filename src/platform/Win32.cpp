#include "Win32.h"

namespace wintangle {

std::wstring Widen(std::string_view utf8) {
    if (utf8.empty()) return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
                                        nullptr, 0);
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), out.data(), len);
    return out;
}

std::string Narrow(std::wstring_view utf16) {
    if (utf16.empty()) return {};
    const int len = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()),
                                        nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), out.data(), len,
                        nullptr, nullptr);
    return out;
}

std::wstring LastErrorMessage(DWORD code) {
    LPWSTR buffer = nullptr;
    const DWORD len = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    std::wstring out = len && buffer ? std::wstring(buffer, len) : L"Fehler " + std::to_wstring(code);
    if (buffer) LocalFree(buffer);
    while (!out.empty() && (out.back() == L'\n' || out.back() == L'\r')) out.pop_back();
    return out;
}

}  // namespace wintangle
