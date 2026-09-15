#pragma once

#include <vector>

#include "WindowRef.h"

namespace wintangle {

struct WindowFilter {
    // Only windows on this display (nullptr = all).
    HMONITOR monitor = nullptr;
    // Only windows of this process (0 = all). For "cascade active app".
    DWORD processId = 0;
    // Include minimized windows, restoring them in the process.
    bool includeMinimized = false;
    // Upper bound, so a tile command with 30 open windows does not turn the
    // desktop into confetti.
    size_t maxCount = 12;
};

// Every arrangeable window in z-order (frontmost first).
std::vector<WindowRef> ListWindows(const WindowFilter& filter);

DWORD ProcessIdOf(HWND hwnd);

}  // namespace wintangle
