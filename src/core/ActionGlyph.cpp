#include "ActionGlyph.h"

#include "Calculation.h"

namespace wintangle {
namespace {

// Inset so that a shape touching the edge still reads as a block inside the
// frame rather than merging with it.
constexpr double kPad = 0.06;

Glyph Filled(Fraction f) { return Glyph{{f}, {}, true}; }

// Two displays side by side, one of them the target.
Glyph Displays(bool targetIsRight) {
    const Fraction left{0.0, 0.2, 0.44, 0.6};
    const Fraction right{0.56, 0.2, 0.44, 0.6};
    Glyph glyph;
    glyph.frame = false;
    glyph.filled.push_back(targetIsRight ? right : left);
    glyph.outlined.push_back(targetIsRight ? left : right);
    return glyph;
}

// Evenly divided strips, as produced by "arrange as rows" and "as columns".
Glyph Strips(bool horizontal, int count) {
    Glyph glyph;
    const double span = (1.0 - 2 * kPad);
    const double size = (span - kPad * (count - 1)) / count;
    for (int i = 0; i < count; ++i) {
        const double offset = kPad + i * (size + kPad);
        glyph.filled.push_back(horizontal ? Fraction{kPad, offset, span, size}
                                          : Fraction{offset, kPad, size, span});
    }
    return glyph;
}

}  // namespace

Glyph GlyphFor(Action action) {
    // Anything that maps to a grid position draws exactly that position -- the
    // icon and the behaviour cannot drift apart, because both come from the
    // same function.
    if (const auto fraction = FractionFor(action, 0, /*cycleSizes=*/false)) {
        return Filled(*fraction);
    }

    switch (action) {
        case Action::MaximizeHeight: return Filled({0.3, 0.0, 0.4, 1.0});
        case Action::MaximizeWidth:  return Filled({0.0, 0.3, 1.0, 0.4});

        case Action::Larger:  return Filled({0.1, 0.1, 0.8, 0.8});
        case Action::Smaller: return Filled({0.3, 0.3, 0.4, 0.4});

        case Action::Center:            return Filled({0.25, 0.25, 0.5, 0.5});
        case Action::CenterProminently: return Filled({0.15, 0.1, 0.7, 0.7});
        case Action::Restore:           return Filled({0.2, 0.2, 0.6, 0.6});

        // Two blocks facing each other, for swapping sides.
        case Action::ReverseAll:
            return Glyph{{{0.0, 0.32, 0.36, 0.36}, {0.64, 0.32, 0.36, 0.36}}, {}, true};

        case Action::MoveLeft:  return Filled({0.0, 0.25, 0.5, 0.5});
        case Action::MoveRight: return Filled({0.5, 0.25, 0.5, 0.5});
        case Action::MoveUp:    return Filled({0.25, 0.0, 0.5, 0.5});
        case Action::MoveDown:  return Filled({0.25, 0.5, 0.5, 0.5});

        case Action::NextDisplay:     return Displays(/*targetIsRight=*/true);
        case Action::PreviousDisplay: return Displays(/*targetIsRight=*/false);

        case Action::TileAll:
            return Glyph{{{kPad, kPad, 0.4, 0.4},
                          {0.54, kPad, 0.4, 0.4},
                          {kPad, 0.54, 0.4, 0.4},
                          {0.54, 0.54, 0.4, 0.4}},
                         {},
                         true};
        case Action::RowsAll:    return Strips(/*horizontal=*/true, 3);
        case Action::ColumnsAll: return Strips(/*horizontal=*/false, 3);

        // The front window solid, the ones behind it as outlines.
        case Action::CascadeAll:
        case Action::CascadeActiveApp:
            return Glyph{{{0.34, 0.34, 0.6, 0.6}},
                         {{0.06, 0.06, 0.6, 0.6}, {0.2, 0.2, 0.6, 0.6}},
                         true};

        default:
            // Nothing should reach this, and the test says so.
            return Filled({0.25, 0.25, 0.5, 0.5});
    }
}

}  // namespace wintangle
