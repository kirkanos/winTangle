// Prints the menu glyph data from the core, one action per line, for
// tools/preview_glyphs.py to rasterise. Exists so the pictures next to the
// menu entries can be looked at while developing on a machine that cannot open
// the menu.
//
//   cmake --build build --target dump_glyphs
//   ./build/dump_glyphs | python3 tools/preview_glyphs.py glyphs.png
#include <cstdio>
#include "core/ActionGlyph.h"
using namespace wintangle;
int main() {
    for (Action a : AllActions()) {
        std::printf("%s", std::string(ActionName(a)).c_str());
        const Glyph g = GlyphFor(a);
        std::printf("|%d", g.frame ? 1 : 0);
        for (const auto& f : g.filled) std::printf("|F %.4f %.4f %.4f %.4f", f.x, f.y, f.w, f.h);
        for (const auto& f : g.outlined) std::printf("|O %.4f %.4f %.4f %.4f", f.x, f.y, f.w, f.h);
        std::printf("\n");
    }
}
