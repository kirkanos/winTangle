#pragma once

#include <vector>

#include "WindowRef.h"

namespace wintangle {

struct WindowFilter {
    // Nur Fenster auf diesem Monitor (nullptr = alle).
    HMONITOR monitor = nullptr;
    // Nur Fenster dieses Prozesses (0 = alle). Fuer "aktive App staffeln".
    DWORD processId = 0;
    // Minimierte Fenster mitnehmen und dabei wiederherstellen.
    bool includeMinimized = false;
    // Obergrenze, damit ein Kachelbefehl bei 30 offenen Fenstern nicht den
    // Desktop unbrauchbar macht.
    size_t maxCount = 12;
};

// Alle anordbaren Fenster in Z-Order (vorderstes zuerst).
std::vector<WindowRef> ListWindows(const WindowFilter& filter);

DWORD ProcessIdOf(HWND hwnd);

}  // namespace wintangle
