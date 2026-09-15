#pragma once

#include <cstddef>
#include <vector>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Calculates the target positions for the multi-window actions (tile, rows,
// columns, cascade). Platform free as well, so the layout stays testable
// without a running Windows; picking the windows happens in the Win32 layer.
//
// `count` is the number of windows to place, in z-order from front to back.
// The result always has exactly `count` entries.
std::vector<Rect> LayoutWindows(Action action, size_t count, const Rect& area, const Gaps& gaps);

}  // namespace wintangle
