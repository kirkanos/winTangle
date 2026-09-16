#include "MenuIcons.h"

#include <vector>

#include "core/ActionGlyph.h"

namespace wintangle {
namespace {

// Logical size at 96 dpi. Matches what Windows reserves for a menu bitmap.
constexpr int kBaseSize = 16;

struct Pixel {
    BYTE b, g, r, a;
};

// Premultiplied, as menu bitmaps require.
Pixel Premultiply(BYTE r, BYTE g, BYTE b, BYTE a) {
    auto scale = [a](BYTE c) { return static_cast<BYTE>((c * a + 127) / 255); };
    return Pixel{scale(b), scale(g), scale(r), a};
}

class Canvas {
public:
    Canvas(Pixel* pixels, int size) : pixels_(pixels), size_(size) {}

    void FillRect(double x, double y, double w, double h, Pixel color) {
        const int left = Scale(x);
        const int top = Scale(y);
        const int right = std::max(left + 1, Scale(x + w));
        const int bottom = std::max(top + 1, Scale(y + h));
        for (int py = top; py < std::min(bottom, size_); ++py) {
            for (int px = left; px < std::min(right, size_); ++px) {
                pixels_[py * size_ + px] = color;
            }
        }
    }

    void StrokeRect(double x, double y, double w, double h, Pixel color, int thickness) {
        const double t = static_cast<double>(thickness) / size_;
        FillRect(x, y, w, t, color);                 // top
        FillRect(x, y + h - t, w, t, color);         // bottom
        FillRect(x, y, t, h, color);                 // left
        FillRect(x + w - t, y, t, h, color);         // right
    }

private:
    int Scale(double value) const {
        return std::clamp(static_cast<int>(std::lround(value * size_)), 0, size_);
    }

    Pixel* pixels_;
    int size_;
};

}  // namespace

MenuIcons::~MenuIcons() { Clear(); }

void MenuIcons::Clear() {
    for (auto& [action, bitmap] : bitmaps_) {
        if (bitmap) DeleteObject(bitmap);
    }
    bitmaps_.clear();
}

HBITMAP MenuIcons::For(Action action, UINT dpi) {
    if (dpi != dpi_) {
        Clear();
        dpi_ = dpi;
    }
    const auto cached = bitmaps_.find(action);
    if (cached != bitmaps_.end()) return cached->second;

    const int size = MulDiv(kBaseSize, static_cast<int>(dpi ? dpi : 96), 96);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = size;
    info.bmiHeader.biHeight = -size;  // top-down, so row 0 is the top
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) {
        if (bitmap) DeleteObject(bitmap);
        bitmaps_[action] = nullptr;
        return nullptr;
    }

    auto* pixels = static_cast<Pixel*>(bits);
    std::fill(pixels, pixels + static_cast<size_t>(size) * size, Pixel{0, 0, 0, 0});

    // The accent matches the drag preview, so the same colour means the same
    // thing throughout. The frame follows the menu text so it stays legible on
    // a dark menu.
    const Pixel accent = Premultiply(0x3A, 0x7B, 0xD5, 0xFF);
    const COLORREF menuText = GetSysColor(COLOR_MENUTEXT);
    const Pixel frame = Premultiply(GetRValue(menuText), GetGValue(menuText),
                                    GetBValue(menuText), 0x8C);
    const int thickness = std::max(1, size / 16);

    Canvas canvas(pixels, size);
    const Glyph glyph = GlyphFor(action);

    if (glyph.frame) canvas.StrokeRect(0.0, 0.0, 1.0, 1.0, frame, thickness);
    for (const auto& f : glyph.outlined) canvas.StrokeRect(f.x, f.y, f.w, f.h, frame, thickness);
    for (const auto& f : glyph.filled) canvas.FillRect(f.x, f.y, f.w, f.h, accent);

    bitmaps_[action] = bitmap;
    return bitmap;
}

}  // namespace wintangle
