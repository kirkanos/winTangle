#pragma once

#include "platform/Win32.h"

namespace wintangle {

// Semi transparent preview of the target position while dragging.
//
// The window styles are what matters: WS_EX_TRANSPARENT lets mouse events pass
// through (otherwise the drag aborts), WS_EX_NOACTIVATE keeps the preview from
// stealing focus, WS_EX_TOOLWINDOW keeps it out of Alt+Tab.
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
