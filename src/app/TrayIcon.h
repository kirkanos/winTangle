#pragma once

#include <string>

#include "config/Config.h"
#include "platform/Win32.h"

namespace wintangle {

// Notification area icon plus context menu. The menu carries the entire action
// catalogue -- actions without a key combination are reachable through it too,
// exactly as in Rectangle.
class TrayIcon {
public:
    TrayIcon(HWND owner, HICON icon);
    ~TrayIcon();

    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

    // After Explorer restarts the icon has to be registered again.
    void Recreate();

    void ShowBalloon(const std::wstring& title, const std::wstring& text, bool warning = false);

    // Shows the context menu at the current cursor position.
    void ShowMenu(const Config& config, bool autostartEnabled, bool updatesSupported);

private:
    NOTIFYICONDATAW data_{};
    HWND owner_;
    HICON icon_;
};

}  // namespace wintangle
