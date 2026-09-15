#include <string>

#include "check.h"
#include "core/SnapZones.h"
#include "core/Uri.h"

using namespace wintangle;

TEST(Uri_yields_the_action_name) {
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?name=left-half"),
             std::string("left-half"));
    // Windows likes to append a slash and to upper-case the scheme.
    CHECK_EQ(ParseExecuteActionUri("WinTangle://Execute-Action?name=Left-Half"),
             std::string("left-half"));
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?foo=1&name=maximize"),
             std::string("maximize"));
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?name=top%2Dleft"),
             std::string("top-left"));
}

TEST(Uri_rejects_everything_else) {
    CHECK(ParseExecuteActionUri("").empty());
    CHECK(ParseExecuteActionUri("https://example.com").empty());
    CHECK(ParseExecuteActionUri("wintangle://etwas-anderes?name=maximize").empty());
    CHECK(ParseExecuteActionUri("wintangle://execute-action").empty());
    CHECK(ParseExecuteActionUri("wintangle://execute-action?andere=maximize").empty());
    // "execute-action-xyz" must not pass as a match.
    CHECK(ParseExecuteActionUri("wintangle://execute-actions?name=maximize").empty());
}

TEST(Url_decoding) {
    CHECK_EQ(UrlDecode("a%20b+c"), std::string("a b c"));
    CHECK_EQ(UrlDecode("100%"), std::string("100%"));   // incomplete, left as is
    CHECK_EQ(UrlDecode("%zz"), std::string("%zz"));     // not hex, left as is
}

namespace {
const Rect kScreen{0, 0, 1920, 1080};
Action Zone(int x, int y) {
    const auto a = SnapZoneAt(x, y, kScreen);
    return a ? *a : Action::Count_;
}
}  // namespace

TEST(Snap_zones_along_the_edges) {
    CHECK(Zone(2, 540) == Action::LeftHalf);
    CHECK(Zone(1918, 540) == Action::RightHalf);
    CHECK(Zone(960, 1) == Action::Maximize);
}

TEST(Snap_corners_yield_quarters) {
    CHECK(Zone(1, 1) == Action::TopLeft);
    CHECK(Zone(1918, 2) == Action::TopRight);
    CHECK(Zone(1, 1078) == Action::BottomLeft);
    CHECK(Zone(1918, 1078) == Action::BottomRight);
}

TEST(The_bottom_edge_is_split_into_thirds) {
    CHECK(Zone(300, 1079) == Action::FirstThird);
    CHECK(Zone(960, 1079) == Action::CenterThird);
    CHECK(Zone(1600, 1079) == Action::LastThird);
}

TEST(Nothing_happens_in_the_middle_of_the_screen) {
    CHECK(!SnapZoneAt(960, 540, kScreen).has_value());
    CHECK(!SnapZoneAt(100, 300, kScreen).has_value());
    // An empty display must not crash.
    CHECK(!SnapZoneAt(0, 0, Rect{}).has_value());
}

TEST(Zones_follow_the_display_offset) {
    const Rect second{1920, 0, 3840, 1080};
    const auto a = SnapZoneAt(1921, 540, second);
    CHECK(a.has_value() && *a == Action::LeftHalf);
    const auto b = SnapZoneAt(3838, 540, second);
    CHECK(b.has_value() && *b == Action::RightHalf);
    // The middle of the second display is not a zone -- the first display's
    // edge must not be counted here.
    CHECK(!SnapZoneAt(2880, 540, second).has_value());
}
