#pragma once

#include <optional>
#include <vector>

#include "Win32.h"

namespace wintangle {

struct MonitorInfo {
    HMONITOR handle = nullptr;
    Rect bounds;   // gesamter Monitor
    Rect work;     // ohne Taskleiste und angedockte Leisten
    UINT dpi = 96;
    bool primary = false;

    bool IsValid() const { return handle != nullptr; }
};

// Alle Monitore, sortiert von links nach rechts und dann von oben nach unten.
// Diese Reihenfolge bestimmt, was "naechster Bildschirm" bedeutet.
std::vector<MonitorInfo> EnumerateMonitors();

MonitorInfo MonitorForWindow(HWND hwnd);
MonitorInfo MonitorForPoint(POINT pt);

// Nachbarmonitor in der sortierten Liste; laeuft zyklisch um, damit
// "naechster Bildschirm" auf dem letzten Monitor wieder beim ersten landet.
// Gibt nullopt zurueck, wenn es nur einen Monitor gibt.
std::optional<MonitorInfo> NeighborMonitor(HMONITOR current, int step);

}  // namespace wintangle
