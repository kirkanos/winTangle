// Plain geometry. Deliberately free of <windows.h> so the entire calculation
// core can be built and tested on macOS and Linux as well.
#pragma once

#include <algorithm>
#include <cmath>

namespace wintangle {

// Left/top/right/bottom like the Win32 RECT: right and bottom are exclusive.
struct Rect {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    static Rect FromXYWH(int x, int y, int w, int h) { return Rect{x, y, x + w, y + h}; }

    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
    int CenterX() const { return left + Width() / 2; }
    int CenterY() const { return top + Height() / 2; }
    bool IsEmpty() const { return Width() <= 0 || Height() <= 0; }

    bool operator==(const Rect& o) const {
        return left == o.left && top == o.top && right == o.right && bottom == o.bottom;
    }
    bool operator!=(const Rect& o) const { return !(*this == o); }

    Rect Inset(int dx, int dy) const { return Rect{left + dx, top + dy, right - dx, bottom - dy}; }

    Rect Offset(int dx, int dy) const {
        return Rect{left + dx, top + dy, right + dx, bottom + dy};
    }

    // Moves (without scaling) just far enough to fit inside `bounds`.
    Rect ClampedInto(const Rect& bounds) const {
        Rect r = *this;
        if (r.Width() > bounds.Width()) {
            r.left = bounds.left;
            r.right = bounds.right;
        } else if (r.left < bounds.left) {
            r = r.Offset(bounds.left - r.left, 0);
        } else if (r.right > bounds.right) {
            r = r.Offset(bounds.right - r.right, 0);
        }
        if (r.Height() > bounds.Height()) {
            r.top = bounds.top;
            r.bottom = bounds.bottom;
        } else if (r.top < bounds.top) {
            r = r.Offset(0, bounds.top - r.top);
        } else if (r.bottom > bounds.bottom) {
            r = r.Offset(0, bounds.bottom - r.bottom);
        }
        return r;
    }

    bool Contains(int x, int y) const {
        return x >= left && x < right && y >= top && y < bottom;
    }
};

// Fractional description of a target position inside the work area.
// Left half, for example: {0.0, 0.0, 0.5, 1.0}
struct Fraction {
    double x = 0.0;
    double y = 0.0;
    double w = 1.0;
    double h = 1.0;

    bool operator==(const Fraction& o) const {
        auto eq = [](double a, double b) { return std::fabs(a - b) < 1e-9; };
        return eq(x, o.x) && eq(y, o.y) && eq(w, o.w) && eq(h, o.h);
    }
};

// Turns a fraction into absolute pixels inside `area`. Rounds the edges rather
// than the width and height, so two adjacent halves meet exactly and leave no
// one-pixel seam between them.
inline Rect ApplyFraction(const Rect& area, const Fraction& f) {
    const double w = static_cast<double>(area.Width());
    const double h = static_cast<double>(area.Height());
    const int l = area.left + static_cast<int>(std::lround(f.x * w));
    const int t = area.top + static_cast<int>(std::lround(f.y * h));
    const int r = area.left + static_cast<int>(std::lround((f.x + f.w) * w));
    const int b = area.top + static_cast<int>(std::lround((f.y + f.h) * h));
    return Rect{l, t, r, b};
}

// Outer margin to the screen edge and inner gap between windows. The inner gap
// is applied by halves per edge, so two neighbouring windows end up exactly
// `inner` pixels apart.
struct Gaps {
    int outer = 0;
    int inner = 0;
    bool IsZero() const { return outer == 0 && inner == 0; }
};

// Applies the gaps to an already calculated target rect. Edges that sit on the
// boundary of the work area get the outer gap, all others half the inner one.
inline Rect ApplyGaps(const Rect& target, const Rect& area, const Gaps& gaps) {
    if (gaps.IsZero()) return target;
    const int half = gaps.inner / 2;
    Rect r = target;
    r.left += (target.left <= area.left) ? gaps.outer : half;
    r.top += (target.top <= area.top) ? gaps.outer : half;
    r.right -= (target.right >= area.right) ? gaps.outer : half;
    r.bottom -= (target.bottom >= area.bottom) ? gaps.outer : half;
    return r;
}

}  // namespace wintangle
