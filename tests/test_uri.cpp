#include <string>

#include "check.h"
#include "core/SnapZones.h"
#include "core/Uri.h"

using namespace wintangle;

TEST(Uri_liefert_den_Aktionsnamen) {
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?name=left-half"),
             std::string("left-half"));
    // Windows haengt gern einen Schraegstrich an und schreibt das Schema gross.
    CHECK_EQ(ParseExecuteActionUri("WinTangle://Execute-Action?name=Left-Half"),
             std::string("left-half"));
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?foo=1&name=maximize"),
             std::string("maximize"));
    CHECK_EQ(ParseExecuteActionUri("wintangle://execute-action?name=top%2Dleft"),
             std::string("top-left"));
}

TEST(Uri_weist_alles_andere_ab) {
    CHECK(ParseExecuteActionUri("").empty());
    CHECK(ParseExecuteActionUri("https://example.com").empty());
    CHECK(ParseExecuteActionUri("wintangle://etwas-anderes?name=maximize").empty());
    CHECK(ParseExecuteActionUri("wintangle://execute-action").empty());
    CHECK(ParseExecuteActionUri("wintangle://execute-action?andere=maximize").empty());
    // "execute-action-xyz" darf nicht als Treffer durchgehen.
    CHECK(ParseExecuteActionUri("wintangle://execute-actions?name=maximize").empty());
}

TEST(Url_Dekodierung) {
    CHECK_EQ(UrlDecode("a%20b+c"), std::string("a b c"));
    CHECK_EQ(UrlDecode("100%"), std::string("100%"));   // unvollstaendig, bleibt stehen
    CHECK_EQ(UrlDecode("%zz"), std::string("%zz"));     // kein Hex, bleibt stehen
}

namespace {
const Rect kScreen{0, 0, 1920, 1080};
Action Zone(int x, int y) {
    const auto a = SnapZoneAt(x, y, kScreen);
    return a ? *a : Action::Count_;
}
}  // namespace

TEST(Snap_Zonen_an_den_Raendern) {
    CHECK(Zone(2, 540) == Action::LeftHalf);
    CHECK(Zone(1918, 540) == Action::RightHalf);
    CHECK(Zone(960, 1) == Action::Maximize);
}

TEST(Snap_Ecken_ergeben_Viertel) {
    CHECK(Zone(1, 1) == Action::TopLeft);
    CHECK(Zone(1918, 2) == Action::TopRight);
    CHECK(Zone(1, 1078) == Action::BottomLeft);
    CHECK(Zone(1918, 1078) == Action::BottomRight);
}

TEST(Unterer_Rand_ist_in_Drittel_geteilt) {
    CHECK(Zone(300, 1079) == Action::FirstThird);
    CHECK(Zone(960, 1079) == Action::CenterThird);
    CHECK(Zone(1600, 1079) == Action::LastThird);
}

TEST(Mitten_im_Bild_passiert_nichts) {
    CHECK(!SnapZoneAt(960, 540, kScreen).has_value());
    CHECK(!SnapZoneAt(100, 300, kScreen).has_value());
    // Leerer Monitor darf nicht abstuerzen.
    CHECK(!SnapZoneAt(0, 0, Rect{}).has_value());
}

TEST(Zonen_folgen_dem_Monitor_Offset) {
    const Rect second{1920, 0, 3840, 1080};
    const auto a = SnapZoneAt(1921, 540, second);
    CHECK(a.has_value() && *a == Action::LeftHalf);
    const auto b = SnapZoneAt(3838, 540, second);
    CHECK(b.has_value() && *b == Action::RightHalf);
    // Die Mitte des zweiten Monitors ist keine Zone -- der Rand des ersten
    // Monitors darf nicht mitgerechnet werden.
    CHECK(!SnapZoneAt(2880, 540, second).has_value());
}
