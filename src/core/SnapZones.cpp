#include "SnapZones.h"

namespace wintangle {

std::optional<Action> SnapZoneAt(int x, int y, const Rect& bounds, const SnapZoneConfig& cfg) {
    if (bounds.IsEmpty()) return std::nullopt;

    const int margin = cfg.edgeMargin;
    const bool nearLeft = x <= bounds.left + margin;
    const bool nearRight = x >= bounds.right - 1 - margin;
    const bool nearTop = y <= bounds.top + margin;
    const bool nearBottom = y >= bounds.bottom - 1 - margin;

    if (!nearLeft && !nearRight && !nearTop && !nearBottom) return std::nullopt;

    const int cornerHeight = static_cast<int>(bounds.Height() * cfg.cornerFraction);

    // Side edges first: they own the corners, the top edge does not.
    if (nearLeft) {
        if (y < bounds.top + cornerHeight) return Action::TopLeft;
        if (y > bounds.bottom - cornerHeight) return Action::BottomLeft;
        return Action::LeftHalf;
    }
    if (nearRight) {
        if (y < bounds.top + cornerHeight) return Action::TopRight;
        if (y > bounds.bottom - cornerHeight) return Action::BottomRight;
        return Action::RightHalf;
    }
    if (nearTop) return Action::Maximize;

    // Bottom edge: split into thirds horizontally.
    const int third = bounds.Width() / 3;
    if (x < bounds.left + third) return Action::FirstThird;
    if (x < bounds.left + 2 * third) return Action::CenterThird;
    return Action::LastThird;
}

}  // namespace wintangle
