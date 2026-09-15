#pragma once

#include <optional>
#include <vector>

#include "Win32.h"

namespace wintangle {

struct MonitorInfo {
    HMONITOR handle = nullptr;
    Rect bounds;   // the whole display
    Rect work;     // without the taskbar and docked appbars
    UINT dpi = 96;
    bool primary = false;

    bool IsValid() const { return handle != nullptr; }
};

// Every display, sorted left to right and then top to bottom. That order is
// what "next display" means.
std::vector<MonitorInfo> EnumerateMonitors();

MonitorInfo MonitorForWindow(HWND hwnd);
MonitorInfo MonitorForPoint(POINT pt);

// Neighbouring display in that sorted list; wraps around, so "next display"
// goes back to the first one from the last. Returns nullopt when there is only
// a single display.
std::optional<MonitorInfo> NeighborMonitor(HMONITOR current, int step);

}  // namespace wintangle
