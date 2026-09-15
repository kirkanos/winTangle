#pragma once

#include <cstdint>
#include <string>

#include "Win32.h"

namespace wintangle {

// A thin wrapper around an HWND. It deals with the two things every window
// tool on Windows trips over first:
//
//  * The invisible shadow border. GetWindowRect includes it, but what the user
//    sees is DWMWA_EXTENDED_FRAME_BOUNDS. Without correcting for it every
//    window sits a few pixels off and two "halves" overlap.
//  * Maximized windows. SetWindowPos on a maximized window leaves it in an
//    inconsistent state; it has to be restored first.
class WindowRef {
public:
    WindowRef() = default;
    explicit WindowRef(HWND hwnd) : hwnd_(hwnd) {}

    HWND Handle() const { return hwnd_; }
    explicit operator bool() const { return hwnd_ != nullptr && IsWindow(hwnd_); }
    std::uint64_t Id() const { return reinterpret_cast<std::uint64_t>(hwnd_); }

    // The visible frame (shadow corrected). That is the size the user sees and
    // the one the calculation core works with.
    Rect Frame() const;

    // Sets the visible frame. Returns false when Windows refuses -- typically
    // for windows of elevated processes (UIPI).
    bool SetFrame(const Rect& frame, DWORD* errorOut = nullptr);

    // Converts a desired visible frame into the coordinates SetWindowPos and
    // DeferWindowPos expect. Needed by the multi-window path, which places all
    // windows in one go.
    RECT ToWindowRect(const Rect& frame) const;

    bool IsMaximized() const { return IsZoomed(hwnd_) != FALSE; }
    bool IsMinimized() const { return IsIconic(hwnd_) != FALSE; }
    bool IsCloaked() const;

    // Can and should WinTangle touch this window? Filters out the desktop, the
    // shell, tool windows, invisible ones and windows cloaked by the shell.
    bool IsManageable() const;

    // File name of the owning executable, lower case ("notepad.exe").
    std::string ExeName() const;
    std::wstring Title() const;

    // Brings the window out of a maximized or minimized state.
    void EnsureRestored();

private:
    // Difference between the window rect and the visible frame, per edge.
    RECT ShadowInsets() const;

    HWND hwnd_ = nullptr;
};

}  // namespace wintangle
