#pragma once

#include <vector>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// A small picture of what an action does, the way Rectangle shows one next to
// every menu entry: the screen as a frame, the resulting window as a filled
// block inside it.
//
// Described here in fractions of the icon, without a single pixel or Windows
// type, for the same reason the rest of the core is: it can be checked by a
// test. The Win32 side turns these into a bitmap.
struct Glyph {
    // Drawn solid, in order.
    std::vector<Fraction> filled;
    // Drawn as an outline only -- for the windows behind, and for the display
    // a window is moving away from.
    std::vector<Fraction> outlined;
    // Whether to draw the frame representing the screen. Off for the actions
    // that show two displays, which draw their own.
    bool frame = true;
};

// Never empty: every action in the catalogue has a picture.
Glyph GlyphFor(Action action);

}  // namespace wintangle
