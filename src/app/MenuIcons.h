#pragma once

#include <map>

#include "core/Action.h"
#include "platform/Win32.h"

namespace wintangle {

// Renders the little pictures next to the tray menu entries and keeps them
// alive for as long as the menu needs them.
//
// Menu bitmaps have to be 32-bit with premultiplied alpha to sit correctly on
// both a light and a dark menu background; anything else leaves a grey box
// around the icon. They are drawn pixel by pixel rather than through GDI,
// which keeps the alpha under our control.
class MenuIcons {
public:
    ~MenuIcons();

    MenuIcons(const MenuIcons&) = delete;
    MenuIcons& operator=(const MenuIcons&) = delete;
    MenuIcons() = default;

    // Bitmap for an action at the given dpi, created on first use. Returns
    // nullptr if it cannot be created -- the caller then simply shows a menu
    // entry without a picture.
    HBITMAP For(Action action, UINT dpi);

    // Drops every cached bitmap. Called when the menu language or the dpi
    // changes, and on shutdown.
    void Clear();

private:
    UINT dpi_ = 0;
    std::map<Action, HBITMAP> bitmaps_;
};

}  // namespace wintangle
