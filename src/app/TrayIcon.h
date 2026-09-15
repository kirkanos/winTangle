#pragma once

#include <string>

#include "config/Config.h"
#include "platform/Win32.h"

namespace wintangle {

// Symbol im Infobereich samt Kontextmenue. Das Menue enthaelt den gesamten
// Aktionskatalog -- auch die Aktionen ohne Tastenkombination sind darueber
// erreichbar, genau wie in Rectangle.
class TrayIcon {
public:
    TrayIcon(HWND owner, HICON icon);
    ~TrayIcon();

    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

    // Nach einem Neustart des Explorers muss das Symbol neu angemeldet werden.
    void Recreate();

    void ShowBalloon(const std::wstring& title, const std::wstring& text, bool warning = false);

    // Zeigt das Kontextmenue an der aktuellen Mausposition.
    void ShowMenu(const Config& config, bool autostartEnabled, bool updatesSupported);

private:
    NOTIFYICONDATAW data_{};
    HWND owner_;
    HICON icon_;
};

}  // namespace wintangle
