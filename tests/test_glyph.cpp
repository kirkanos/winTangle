#include <string>

#include "check.h"
#include "core/ActionGlyph.h"
#include "core/Calculation.h"

using namespace wintangle;

namespace {

bool Inside(const Fraction& f) {
    return f.x >= -0.001 && f.y >= -0.001 && f.w > 0.0 && f.h > 0.0 &&
           f.x + f.w <= 1.001 && f.y + f.h <= 1.001;
}

}  // namespace

TEST(Every_action_has_a_glyph) {
    for (Action a : AllActions()) {
        const Glyph glyph = GlyphFor(a);
        if (glyph.filled.empty() && glyph.outlined.empty()) {
            ::check::Fail(__FILE__, __LINE__, "No glyph for " + std::string(ActionName(a)));
        }
    }
}

TEST(No_glyph_draws_outside_its_icon) {
    for (Action a : AllActions()) {
        const Glyph glyph = GlyphFor(a);
        for (const auto& f : glyph.filled) {
            if (!Inside(f))
                ::check::Fail(__FILE__, __LINE__,
                              "Filled shape outside the icon: " + std::string(ActionName(a)));
        }
        for (const auto& f : glyph.outlined) {
            if (!Inside(f))
                ::check::Fail(__FILE__, __LINE__,
                              "Outline outside the icon: " + std::string(ActionName(a)));
        }
    }
}

TEST(Grid_actions_draw_exactly_what_they_do) {
    // The icon must come from the same source as the behaviour, otherwise the
    // two drift apart the first time a fraction is adjusted.
    for (Action a : AllActions()) {
        const auto fraction = FractionFor(a, 0, false);
        if (!fraction) continue;
        const Glyph glyph = GlyphFor(a);
        CHECK_EQ(glyph.filled.size(), size_t{1});
        CHECK(glyph.filled[0] == *fraction);
    }
}

TEST(Halves_and_quarters_look_different) {
    CHECK(!(GlyphFor(Action::LeftHalf).filled[0] == GlyphFor(Action::RightHalf).filled[0]));
    CHECK(!(GlyphFor(Action::TopLeft).filled[0] == GlyphFor(Action::BottomRight).filled[0]));
    CHECK(!(GlyphFor(Action::Larger).filled[0] == GlyphFor(Action::Smaller).filled[0]));
    CHECK(!GlyphFor(Action::NextDisplay).frame);
    CHECK_EQ(GlyphFor(Action::CascadeAll).outlined.size(), size_t{2});
}
