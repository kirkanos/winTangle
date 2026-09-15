#include "Layout.h"

#include <cmath>

namespace wintangle {
namespace {

// Zeilen-/Spaltenraster. `cols` Spalten, Zeilen ergeben sich; die letzte Zeile
// darf teilbesetzt sein, ihre Fenster werden dann breiter verteilt, damit kein
// Loch im Raster bleibt.
std::vector<Rect> Grid(size_t count, size_t cols, const Rect& area, const Gaps& gaps) {
    std::vector<Rect> out;
    if (count == 0 || cols == 0) return out;
    const size_t rows = (count + cols - 1) / cols;

    for (size_t i = 0; i < count; ++i) {
        const size_t row = i / cols;
        const bool lastRow = row == rows - 1;
        const size_t inRow = lastRow ? count - row * cols : cols;
        const size_t col = i - row * cols;

        const Fraction f{static_cast<double>(col) / static_cast<double>(inRow),
                         static_cast<double>(row) / static_cast<double>(rows),
                         1.0 / static_cast<double>(inRow),
                         1.0 / static_cast<double>(rows)};
        out.push_back(ApplyGaps(ApplyFraction(area, f), area, gaps));
    }
    return out;
}

// Gestaffelt: gleich große Fenster, jeweils um einen festen Schritt versetzt.
std::vector<Rect> Cascade(size_t count, const Rect& area, const Gaps& gaps) {
    std::vector<Rect> out;
    if (count == 0) return out;

    const Rect bounds = gaps.outer > 0 ? area.Inset(gaps.outer, gaps.outer) : area;
    const int stepX = std::max(24, bounds.Width() / 32);
    const int stepY = std::max(24, bounds.Height() / 24);

    // Fenstergröße so wählen, dass auch das hinterste noch vollständig passt.
    const int w = bounds.Width() - stepX * static_cast<int>(count - 1);
    const int h = bounds.Height() - stepY * static_cast<int>(count - 1);
    const int useW = std::max(w, bounds.Width() / 2);
    const int useH = std::max(h, bounds.Height() / 2);

    for (size_t i = 0; i < count; ++i) {
        const int dx = stepX * static_cast<int>(i);
        const int dy = stepY * static_cast<int>(i);
        out.push_back(Rect::FromXYWH(bounds.left + dx, bounds.top + dy, useW, useH)
                          .ClampedInto(bounds));
    }
    return out;
}

}  // namespace

std::vector<Rect> LayoutWindows(Action action, size_t count, const Rect& area, const Gaps& gaps) {
    if (count == 0 || area.IsEmpty()) return {};

    switch (action) {
        case Action::TileAll: {
            const auto cols = static_cast<size_t>(
                std::ceil(std::sqrt(static_cast<double>(count))));
            return Grid(count, cols, area, gaps);
        }
        case Action::RowsAll:
            return Grid(count, 1, area, gaps);
        case Action::ColumnsAll:
            return Grid(count, count, area, gaps);
        case Action::CascadeAll:
        case Action::CascadeActiveApp:
            return Cascade(count, area, gaps);
        default:
            return {};
    }
}

}  // namespace wintangle
