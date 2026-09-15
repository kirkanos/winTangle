#include "WindowRef.h"

#include <psapi.h>

#include <algorithm>
#include <cwctype>

namespace wintangle {
namespace {

bool GetExtendedFrame(HWND hwnd, RECT& out) {
    return SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &out, sizeof(out)));
}

}  // namespace

RECT WindowRef::ShadowInsets() const {
    RECT insets{0, 0, 0, 0};
    RECT windowRect{};
    RECT frameRect{};
    if (!GetWindowRect(hwnd_, &windowRect)) return insets;
    if (!GetExtendedFrame(hwnd_, frameRect)) return insets;

    insets.left = frameRect.left - windowRect.left;
    insets.top = frameRect.top - windowRect.top;
    insets.right = windowRect.right - frameRect.right;
    insets.bottom = windowRect.bottom - frameRect.bottom;
    return insets;
}

Rect WindowRef::Frame() const {
    RECT frameRect{};
    if (GetExtendedFrame(hwnd_, frameRect)) return FromRECT(frameRect);
    RECT windowRect{};
    GetWindowRect(hwnd_, &windowRect);
    return FromRECT(windowRect);
}

RECT WindowRef::ToWindowRect(const Rect& frame) const {
    const RECT insets = ShadowInsets();
    return RECT{frame.left - insets.left, frame.top - insets.top, frame.right + insets.right,
                frame.bottom + insets.bottom};
}

void WindowRef::EnsureRestored() {
    if (IsMinimized() || IsMaximized()) {
        // SW_RESTORE statt SW_SHOWNORMAL: das Fenster soll nicht nach vorn
        // gerissen werden, nur aus dem maximierten Zustand heraus.
        ShowWindow(hwnd_, SW_RESTORE);
    }
}

bool WindowRef::SetFrame(const Rect& frame, DWORD* errorOut) {
    if (!*this) return false;
    EnsureRestored();

    const RECT target = ToWindowRect(frame);
    SetLastError(0);
    const BOOL ok = SetWindowPos(hwnd_, nullptr, target.left, target.top,
                                 target.right - target.left, target.bottom - target.top,
                                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (!ok && errorOut) *errorOut = GetLastError();
    return ok != FALSE;
}

bool WindowRef::IsCloaked() const {
    // Von der Shell versteckte Fenster: UWP-Apps im Hintergrund und Fenster
    // anderer virtueller Desktops. Die duerfen nicht mitgekachelt werden.
    DWORD cloaked = 0;
    if (FAILED(DwmGetWindowAttribute(hwnd_, DWMWA_CLOAKED, &cloaked, sizeof(cloaked)))) return false;
    return cloaked != 0;
}

bool WindowRef::IsManageable() const {
    if (!*this) return false;
    if (!IsWindowVisible(hwnd_)) return false;
    if (GetAncestor(hwnd_, GA_ROOT) != hwnd_) return false;  // nur echte Top-Level-Fenster
    if (IsCloaked()) return false;

    const LONG_PTR style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
    const LONG_PTR exStyle = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) return false;
    if (style & WS_CHILD) return false;
    // Ohne Rahmen laesst sich nichts sinnvoll anordnen (Splash Screens, Popups).
    if (!(style & WS_CAPTION) && !(style & WS_THICKFRAME)) return false;

    // Desktop und Taskleiste.
    wchar_t cls[64]{};
    GetClassNameW(hwnd_, cls, static_cast<int>(std::size(cls)));
    const std::wstring className(cls);
    if (className == L"Progman" || className == L"WorkerW" || className == L"Shell_TrayWnd" ||
        className == L"Windows.UI.Core.CoreWindow") {
        return false;
    }
    return true;
}

std::string WindowRef::ExeName() const {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd_, &pid);
    if (pid == 0) return {};

    // PROCESS_QUERY_LIMITED_INFORMATION reicht und funktioniert auch fuer
    // Prozesse, an die wir sonst nicht herankommen.
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!proc) return {};

    wchar_t path[MAX_PATH]{};
    DWORD size = static_cast<DWORD>(std::size(path));
    const BOOL ok = QueryFullProcessImageNameW(proc, 0, path, &size);
    CloseHandle(proc);
    if (!ok) return {};

    std::wstring full(path, size);
    const size_t slash = full.find_last_of(L"\\/");
    std::wstring name = slash == std::wstring::npos ? full : full.substr(slash + 1);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return Narrow(name);
}

std::wstring WindowRef::Title() const {
    const int len = GetWindowTextLengthW(hwnd_);
    if (len <= 0) return {};
    std::wstring title(static_cast<size_t>(len) + 1, L'\0');
    const int written = GetWindowTextW(hwnd_, title.data(), len + 1);
    title.resize(static_cast<size_t>(std::max(0, written)));
    return title;
}

}  // namespace wintangle
