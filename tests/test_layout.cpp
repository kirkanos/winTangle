#include <string>

#include "check.h"
#include "ui/SettingsLayout.h"

using namespace wintangle;
using namespace wintangle::settings_layout;

namespace {

bool Overlaps(const Rect& a, const Rect& b) {
    return a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom;
}

}  // namespace

TEST(No_two_controls_overlap_in_the_settings_window) {
    for (size_t i = 0; i < std::size(kAll); ++i) {
        for (size_t j = i + 1; j < std::size(kAll); ++j) {
            if (Overlaps(kAll[i].rect, kAll[j].rect)) {
                ::check::Fail(__FILE__, __LINE__,
                              std::string(kAll[i].name) + " overlaps " + kAll[j].name);
            }
        }
    }
}

TEST(Every_control_fits_inside_the_client_area) {
    const Rect client{0, 0, kClientWidth, kClientHeight};
    for (const auto& slot : kAll) {
        const Rect& r = slot.rect;
        const bool inside = r.left >= client.left && r.top >= client.top &&
                            r.right <= client.right && r.bottom <= client.bottom;
        if (!inside) {
            ::check::Fail(__FILE__, __LINE__, std::string(slot.name) + " sticks out");
        }
    }
}

TEST(Every_control_has_a_usable_size) {
    for (const auto& slot : kAll) {
        if (slot.rect.Width() < 20 || slot.rect.Height() < 14) {
            ::check::Fail(__FILE__, __LINE__, std::string(slot.name) + " is too small");
        }
    }
}

TEST(Controls_keep_a_margin_to_the_window_edge) {
    for (const auto& slot : kAll) {
        if (slot.rect.left < kMargin || slot.rect.top < kMargin ||
            slot.rect.right > kClientWidth - kMargin ||
            slot.rect.bottom > kClientHeight - kMargin) {
            ::check::Fail(__FILE__, __LINE__, std::string(slot.name) + " has no margin");
        }
    }
}
