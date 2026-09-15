#pragma once

#include "platform/Win32.h"

namespace wintangle {

// Halbtransparente Vorschau der Zielposition waehrend des Ziehens.
//
// Wichtig sind die Fensterstile: WS_EX_TRANSPARENT laesst Mausereignisse
// hindurch (sonst bricht das Ziehen ab), WS_EX_NOACTIVATE verhindert, dass die
// Vorschau den Fokus stiehlt, WS_EX_TOOLWINDOW haelt sie aus Alt+Tab heraus.
class Footprint {
public:
    ~Footprint();

    bool Create(HINSTANCE instance);
    void ShowAt(const Rect& frame);
    void Hide();
    bool IsVisible() const { return visible_; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void Paint(HDC dc, const RECT& client) const;

    HWND hwnd_ = nullptr;
    bool visible_ = false;
    Rect current_;
};

}  // namespace wintangle
