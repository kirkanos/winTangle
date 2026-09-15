#pragma once

#include <optional>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

struct SnapZoneConfig {
    // How close the cursor has to get to the screen edge, in pixels at 96 dpi.
    int edgeMargin = 16;
    // Share of the screen height that counts as a corner on the side edges.
    double cornerFraction = 0.25;
};

// Which action does releasing the mouse at this position trigger?
//
// The split follows Rectangle:
//   left/right edge  -> half, the matching quarter in the corners
//   top edge         -> maximize
//   bottom edge      -> thirds, depending on the horizontal position
//
// `bounds` is the whole display, not the work area: while dragging, the cursor
// also reaches the strip behind the taskbar.
std::optional<Action> SnapZoneAt(int x, int y, const Rect& bounds, const SnapZoneConfig& cfg = {});

}  // namespace wintangle
