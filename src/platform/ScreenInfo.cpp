#include "ScreenInfo.h"

#include <algorithm>

namespace wintangle {
namespace {

UINT DpiForMonitor(HMONITOR mon) {
    UINT x = 96, y = 96;
    // GetDpiForMonitor exists since Windows 8.1; if it fails, 96 stands. That
    // only matters for scaling what we draw, not for window coordinates (which
    // are always physical under PerMonitorV2).
    if (SUCCEEDED(GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &x, &y))) return x;
    return 96;
}

MonitorInfo Describe(HMONITOR mon) {
    MonitorInfo info;
    if (!mon) return info;
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(mon, &mi)) return info;

    info.handle = mon;
    info.bounds = FromRECT(mi.rcMonitor);
    info.work = FromRECT(mi.rcWork);
    info.primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
    info.dpi = DpiForMonitor(mon);
    return info;
}

BOOL CALLBACK CollectMonitor(HMONITOR mon, HDC, LPRECT, LPARAM param) {
    auto* out = reinterpret_cast<std::vector<MonitorInfo>*>(param);
    MonitorInfo info = Describe(mon);
    if (info.IsValid()) out->push_back(info);
    return TRUE;
}

}  // namespace

std::vector<MonitorInfo> EnumerateMonitors() {
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(nullptr, nullptr, CollectMonitor, reinterpret_cast<LPARAM>(&monitors));
    std::sort(monitors.begin(), monitors.end(), [](const MonitorInfo& a, const MonitorInfo& b) {
        if (a.bounds.left != b.bounds.left) return a.bounds.left < b.bounds.left;
        return a.bounds.top < b.bounds.top;
    });
    return monitors;
}

MonitorInfo MonitorForWindow(HWND hwnd) {
    return Describe(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
}

MonitorInfo MonitorForPoint(POINT pt) {
    return Describe(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST));
}

std::optional<MonitorInfo> NeighborMonitor(HMONITOR current, int step) {
    const auto monitors = EnumerateMonitors();
    if (monitors.size() < 2) return std::nullopt;

    const auto it = std::find_if(monitors.begin(), monitors.end(),
                                 [&](const MonitorInfo& m) { return m.handle == current; });
    if (it == monitors.end()) return std::nullopt;

    const auto count = static_cast<int>(monitors.size());
    int index = static_cast<int>(std::distance(monitors.begin(), it)) + step;
    index = ((index % count) + count) % count;  // wraps, negative step included
    return monitors[static_cast<size_t>(index)];
}

}  // namespace wintangle
