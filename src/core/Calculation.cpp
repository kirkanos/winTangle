#include "Calculation.h"

#include <array>

namespace wintangle {
namespace {

constexpr double kThird = 1.0 / 3.0;
constexpr double kTwoThirds = 2.0 / 3.0;

// Zyklus bei wiederholtem Druck derselben Hälften-Aktion: 1/2 -> 2/3 -> 1/3.
constexpr std::array<double, 3> kSizeCycle{0.5, kTwoThirds, kThird};

double CycleSize(int repeat, bool cycleSizes) {
    if (!cycleSizes) return 0.5;
    return kSizeCycle[static_cast<size_t>(repeat) % kSizeCycle.size()];
}

// Verkleinert/vergrößert um einen Schritt, behält den Mittelpunkt und bleibt
// in der Arbeitsfläche. Schrittweite ist ein Zwölftel der Arbeitsfläche, damit
// sich das Verhalten auf jedem Monitor gleich anfühlt.
Rect Resize(const Rect& window, const Rect& area, int direction) {
    const int stepX = area.Width() / 12;
    const int stepY = area.Height() / 12;
    const int cx = window.CenterX();
    const int cy = window.CenterY();

    int w = window.Width() + direction * stepX;
    int h = window.Height() + direction * stepY;

    // Untergrenze, damit "Kleiner" ein Fenster nicht unbedienbar macht.
    const int minW = std::max(240, area.Width() / 8);
    const int minH = std::max(160, area.Height() / 8);
    w = std::clamp(w, minW, area.Width());
    h = std::clamp(h, minH, area.Height());

    return Rect::FromXYWH(cx - w / 2, cy - h / 2, w, h).ClampedInto(area);
}

Rect CenterIn(const Rect& window, const Rect& area) {
    return Rect::FromXYWH(area.CenterX() - window.Width() / 2,
                          area.CenterY() - window.Height() / 2,
                          window.Width(), window.Height())
        .ClampedInto(area);
}

// Wie Rectangles "center prominently": mittig, aber mit mehr Luft unten als
// oben, und mindestens auf eine gut sichtbare Größe gebracht.
Rect CenterProminentlyIn(const Rect& window, const Rect& area) {
    const int w = std::max(window.Width(), static_cast<int>(area.Width() * 0.78));
    const int h = std::max(window.Height(), static_cast<int>(area.Height() * 0.78));
    const int x = area.left + (area.Width() - w) / 2;
    const int y = area.top + (area.Height() - h) / 3;  // Drittel statt Hälfte
    return Rect::FromXYWH(x, y, std::min(w, area.Width()), std::min(h, area.Height()))
        .ClampedInto(area);
}

Rect MoveBy(const Rect& window, const Rect& area, int dx, int dy) {
    const int stepX = area.Width() / 20;
    const int stepY = area.Height() / 20;
    return window.Offset(dx * stepX, dy * stepY).ClampedInto(area);
}

// Spiegelt horizontal an der Mittelachse der Arbeitsfläche.
Rect Reverse(const Rect& window, const Rect& area) {
    const int left = area.left + (area.right - window.right);
    return Rect::FromXYWH(left, window.top, window.Width(), window.Height()).ClampedInto(area);
}

}  // namespace

std::optional<Fraction> FractionFor(Action a, int repeat, bool cycleSizes) {
    const double c = CycleSize(repeat, cycleSizes);

    switch (a) {
        // Hälften zyklieren bei Wiederholung.
        case Action::LeftHalf:   return Fraction{0.0, 0.0, c, 1.0};
        case Action::RightHalf:  return Fraction{1.0 - c, 0.0, c, 1.0};
        case Action::TopHalf:    return Fraction{0.0, 0.0, 1.0, c};
        case Action::BottomHalf: return Fraction{0.0, 1.0 - c, 1.0, c};
        case Action::CenterHalf: return Fraction{(1.0 - c) / 2.0, 0.0, c, 1.0};

        case Action::TopLeft:     return Fraction{0.0, 0.0, 0.5, 0.5};
        case Action::TopRight:    return Fraction{0.5, 0.0, 0.5, 0.5};
        case Action::BottomLeft:  return Fraction{0.0, 0.5, 0.5, 0.5};
        case Action::BottomRight: return Fraction{0.5, 0.5, 0.5, 0.5};

        case Action::FirstThird:     return Fraction{0.0, 0.0, kThird, 1.0};
        case Action::CenterThird:    return Fraction{kThird, 0.0, kThird, 1.0};
        case Action::LastThird:      return Fraction{kTwoThirds, 0.0, kThird, 1.0};
        case Action::FirstTwoThirds: return Fraction{0.0, 0.0, kTwoThirds, 1.0};
        case Action::LastTwoThirds:  return Fraction{kThird, 0.0, kTwoThirds, 1.0};

        case Action::TopLeftSixth:      return Fraction{0.0, 0.0, kThird, 0.5};
        case Action::TopCenterSixth:    return Fraction{kThird, 0.0, kThird, 0.5};
        case Action::TopRightSixth:     return Fraction{kTwoThirds, 0.0, kThird, 0.5};
        case Action::BottomLeftSixth:   return Fraction{0.0, 0.5, kThird, 0.5};
        case Action::BottomCenterSixth: return Fraction{kThird, 0.5, kThird, 0.5};
        case Action::BottomRightSixth:  return Fraction{kTwoThirds, 0.5, kThird, 0.5};

        case Action::TopLeftEighth:            return Fraction{0.0, 0.0, 0.25, 0.5};
        case Action::TopCenterLeftEighth:      return Fraction{0.25, 0.0, 0.25, 0.5};
        case Action::TopCenterRightEighth:     return Fraction{0.5, 0.0, 0.25, 0.5};
        case Action::TopRightEighth:           return Fraction{0.75, 0.0, 0.25, 0.5};
        case Action::BottomLeftEighth:         return Fraction{0.0, 0.5, 0.25, 0.5};
        case Action::BottomCenterLeftEighth:   return Fraction{0.25, 0.5, 0.25, 0.5};
        case Action::BottomCenterRightEighth:  return Fraction{0.5, 0.5, 0.25, 0.5};
        case Action::BottomRightEighth:        return Fraction{0.75, 0.5, 0.25, 0.5};

        case Action::TopLeftNinth:      return Fraction{0.0, 0.0, kThird, kThird};
        case Action::TopCenterNinth:    return Fraction{kThird, 0.0, kThird, kThird};
        case Action::TopRightNinth:     return Fraction{kTwoThirds, 0.0, kThird, kThird};
        case Action::MiddleLeftNinth:   return Fraction{0.0, kThird, kThird, kThird};
        case Action::MiddleCenterNinth: return Fraction{kThird, kThird, kThird, kThird};
        case Action::MiddleRightNinth:  return Fraction{kTwoThirds, kThird, kThird, kThird};
        case Action::BottomLeftNinth:   return Fraction{0.0, kTwoThirds, kThird, kThird};
        case Action::BottomCenterNinth: return Fraction{kThird, kTwoThirds, kThird, kThird};
        case Action::BottomRightNinth:  return Fraction{kTwoThirds, kTwoThirds, kThird, kThird};

        case Action::Maximize:       return Fraction{0.0, 0.0, 1.0, 1.0};
        case Action::AlmostMaximize: return Fraction{0.05, 0.05, 0.9, 0.9};

        default:
            return std::nullopt;
    }
}

Rect MapRectToArea(const Rect& rect, const Rect& from, const Rect& to) {
    if (from.IsEmpty()) return rect;
    const double sx = static_cast<double>(to.Width()) / from.Width();
    const double sy = static_cast<double>(to.Height()) / from.Height();
    const int x = to.left + static_cast<int>(std::lround((rect.left - from.left) * sx));
    const int y = to.top + static_cast<int>(std::lround((rect.top - from.top) * sy));
    const int w = static_cast<int>(std::lround(rect.Width() * sx));
    const int h = static_cast<int>(std::lround(rect.Height() * sy));
    return Rect::FromXYWH(x, y, w, h).ClampedInto(to);
}

std::optional<Rect> Calculate(const CalcInput& in) {
    if (IsMultiWindowAction(in.action)) return std::nullopt;
    if (in.workArea.IsEmpty()) return std::nullopt;

    // Rasterbasierte Aktionen: Bruchteil anwenden, dann Abstände.
    if (const auto f = FractionFor(in.action, in.repeat, in.cycleSizes)) {
        const Rect target = ApplyFraction(in.workArea, *f);
        return ApplyGaps(target, in.workArea, in.gaps);
    }

    // Abstände gelten auch für die kantenbündigen Sonderfälle, deshalb wird die
    // Arbeitsfläche dafür vorab um den Außenabstand verkleinert.
    const Rect area = in.gaps.outer > 0 ? in.workArea.Inset(in.gaps.outer, in.gaps.outer)
                                        : in.workArea;

    switch (in.action) {
        case Action::MaximizeHeight:
            return Rect{in.window.left, area.top, in.window.right, area.bottom};
        case Action::MaximizeWidth:
            return Rect{area.left, in.window.top, area.right, in.window.bottom};

        case Action::Larger:  return Resize(in.window, area, +1);
        case Action::Smaller: return Resize(in.window, area, -1);

        case Action::Center:             return CenterIn(in.window, area);
        case Action::CenterProminently:  return CenterProminentlyIn(in.window, area);
        case Action::ReverseAll:         return Reverse(in.window, area);

        case Action::MoveLeft:  return MoveBy(in.window, area, -1, 0);
        case Action::MoveRight: return MoveBy(in.window, area, +1, 0);
        case Action::MoveUp:    return MoveBy(in.window, area, 0, -1);
        case Action::MoveDown:  return MoveBy(in.window, area, 0, +1);

        case Action::Restore:
            return in.restoreRect;

        case Action::NextDisplay:
        case Action::PreviousDisplay:
            if (!in.targetArea) return std::nullopt;
            return MapRectToArea(in.window, in.workArea, *in.targetArea);

        default:
            return std::nullopt;
    }
}

}  // namespace wintangle
