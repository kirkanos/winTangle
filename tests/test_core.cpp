#include <string>

#include "check.h"
#include "core/Calculation.h"
#include "core/History.h"
#include "core/Layout.h"

using namespace wintangle;

namespace {

std::string Str(const Rect& r) {
    return "{" + std::to_string(r.left) + "," + std::to_string(r.top) + "," +
           std::to_string(r.right) + "," + std::to_string(r.bottom) + "}";
}

// Typischer 1080p-Monitor mit 40px Taskbar unten.
const Rect kWork{0, 0, 1920, 1040};
// Zweiter Monitor rechts daneben, andere Auflösung.
const Rect kWork2{1920, 0, 4480, 1400};

Rect Calc(Action a, const Rect& window = Rect{100, 100, 900, 700}, int repeat = 0,
          Gaps gaps = Gaps{}, const Rect& area = kWork) {
    CalcInput in;
    in.action = a;
    in.window = window;
    in.workArea = area;
    in.gaps = gaps;
    in.repeat = repeat;
    const auto r = Calculate(in);
    return r ? *r : Rect{};
}

}  // namespace

#define CHECK_RECT(actual, expected)                                            \
    do {                                                                        \
        const Rect _a = (actual);                                               \
        const Rect _e = (expected);                                             \
        if (_a != _e)                                                           \
            ::check::Fail(__FILE__, __LINE__, Str(_a) + " erwartet " + Str(_e)); \
    } while (0)

TEST(Haelften_teilen_die_Arbeitsflaeche_exakt) {
    CHECK_RECT(Calc(Action::LeftHalf), (Rect{0, 0, 960, 1040}));
    CHECK_RECT(Calc(Action::RightHalf), (Rect{960, 0, 1920, 1040}));
    CHECK_RECT(Calc(Action::TopHalf), (Rect{0, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomHalf), (Rect{0, 520, 1920, 1040}));
    CHECK_RECT(Calc(Action::CenterHalf), (Rect{480, 0, 1440, 1040}));
}

TEST(Viertel_stossen_lueckenlos_aneinander) {
    CHECK_RECT(Calc(Action::TopLeft), (Rect{0, 0, 960, 520}));
    CHECK_RECT(Calc(Action::TopRight), (Rect{960, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomLeft), (Rect{0, 520, 960, 1040}));
    CHECK_RECT(Calc(Action::BottomRight), (Rect{960, 520, 1920, 1040}));
}

TEST(Drittel_lassen_keine_Pixelluecke) {
    const Rect a = Calc(Action::FirstThird);
    const Rect b = Calc(Action::CenterThird);
    const Rect c = Calc(Action::LastThird);
    CHECK_EQ(a.right, b.left);
    CHECK_EQ(b.right, c.left);
    CHECK_EQ(c.right, kWork.right);
    CHECK_RECT(Calc(Action::FirstTwoThirds), (Rect{0, 0, 1280, 1040}));
    CHECK_RECT(Calc(Action::LastTwoThirds), (Rect{640, 0, 1920, 1040}));
}

TEST(Neuntel_decken_die_Flaeche_vollstaendig_ab) {
    const Action rows[3][3] = {
        {Action::TopLeftNinth, Action::TopCenterNinth, Action::TopRightNinth},
        {Action::MiddleLeftNinth, Action::MiddleCenterNinth, Action::MiddleRightNinth},
        {Action::BottomLeftNinth, Action::BottomCenterNinth, Action::BottomRightNinth}};

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            const Rect cell = Calc(rows[r][c]);
            if (c > 0) CHECK_EQ(Calc(rows[r][c - 1]).right, cell.left);
            if (r > 0) CHECK_EQ(Calc(rows[r - 1][c]).bottom, cell.top);
        }
    }
    CHECK_EQ(Calc(rows[2][2]).right, kWork.right);
    CHECK_EQ(Calc(rows[2][2]).bottom, kWork.bottom);
}

TEST(Achtel_ergeben_vier_Spalten_mal_zwei_Zeilen) {
    CHECK_RECT(Calc(Action::TopLeftEighth), (Rect{0, 0, 480, 520}));
    CHECK_RECT(Calc(Action::TopRightEighth), (Rect{1440, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomCenterLeftEighth), (Rect{480, 520, 960, 1040}));
}

TEST(Wiederholter_Druck_zykliert_die_Groesse) {
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 0), (Rect{0, 0, 960, 1040}));
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 1), (Rect{0, 0, 1280, 1040}));
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 2), (Rect{0, 0, 640, 1040}));
    // Nach drei Schritten beginnt der Zyklus von vorn.
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 3), (Rect{0, 0, 960, 1040}));
    // Rechte Hälfte zykliert spiegelbildlich, bleibt also rechtsbündig.
    CHECK_RECT(Calc(Action::RightHalf, Rect{}, 1), (Rect{640, 0, 1920, 1040}));
}

TEST(Zyklus_laesst_sich_abschalten) {
    CalcInput in;
    in.action = Action::LeftHalf;
    in.workArea = kWork;
    in.repeat = 1;
    in.cycleSizes = false;
    CHECK_RECT(*Calculate(in), (Rect{0, 0, 960, 1040}));
}

TEST(Abstaende_gelten_aussen_ganz_und_innen_halb) {
    const Gaps gaps{10, 10};
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 0, gaps), (Rect{10, 10, 955, 1030}));
    CHECK_RECT(Calc(Action::RightHalf, Rect{}, 0, gaps), (Rect{965, 10, 1910, 1030}));
    CHECK_RECT(Calc(Action::Maximize, Rect{}, 0, gaps), (Rect{10, 10, 1910, 1030}));
    // Zwei benachbarte Hälften haben zusammen genau den Innenabstand.
    CHECK_EQ(Calc(Action::RightHalf, Rect{}, 0, gaps).left -
                 Calc(Action::LeftHalf, Rect{}, 0, gaps).right,
             10);
}

TEST(Maximieren_und_fast_maximieren) {
    CHECK_RECT(Calc(Action::Maximize), kWork);
    CHECK_RECT(Calc(Action::AlmostMaximize), (Rect{96, 52, 1824, 988}));
}

TEST(Hoehe_und_Breite_maximieren_lassen_die_andere_Achse_in_Ruhe) {
    const Rect w{300, 200, 800, 600};
    CHECK_RECT(Calc(Action::MaximizeHeight, w), (Rect{300, 0, 800, 1040}));
    CHECK_RECT(Calc(Action::MaximizeWidth, w), (Rect{0, 200, 1920, 600}));
}

TEST(Zentrieren_behaelt_die_Groesse) {
    const Rect w{0, 0, 800, 600};
    const Rect centered = Calc(Action::Center, w);
    CHECK_EQ(centered.Width(), 800);
    CHECK_EQ(centered.Height(), 600);
    CHECK_EQ(centered.CenterX(), kWork.CenterX());
    CHECK_EQ(centered.CenterY(), kWork.CenterY());
}

TEST(Prominent_zentriert_sitzt_hoeher_als_mittig) {
    const Rect w{0, 0, 800, 600};
    const Rect prominent = Calc(Action::CenterProminently, w);
    CHECK(prominent.CenterY() < kWork.CenterY());
    CHECK(prominent.Width() > w.Width());
    CHECK(prominent.left >= kWork.left && prominent.right <= kWork.right);
}

TEST(Groesser_und_kleiner_behalten_den_Mittelpunkt) {
    const Rect w{560, 320, 1360, 720};
    const Rect larger = Calc(Action::Larger, w);
    CHECK_EQ(larger.CenterX(), w.CenterX());
    CHECK_EQ(larger.CenterY(), w.CenterY());
    CHECK(larger.Width() > w.Width());
    CHECK(Calc(Action::Smaller, w).Width() < w.Width());
}

TEST(Groesser_sprengt_die_Arbeitsflaeche_nicht) {
    Rect w = kWork;
    for (int i = 0; i < 5; ++i) w = Calc(Action::Larger, w);
    CHECK(w.left >= kWork.left && w.right <= kWork.right);
    CHECK(w.top >= kWork.top && w.bottom <= kWork.bottom);
}

TEST(Kleiner_hat_eine_Untergrenze) {
    Rect w{0, 0, 800, 600};
    for (int i = 0; i < 20; ++i) w = Calc(Action::Smaller, w);
    CHECK(w.Width() >= 240);
    CHECK(w.Height() >= 160);
}

TEST(Spiegeln_vertauscht_links_und_rechts) {
    CHECK_RECT(Calc(Action::ReverseAll, Rect{0, 0, 640, 1040}), (Rect{1280, 0, 1920, 1040}));
    // Zweimal spiegeln ergibt wieder das Original.
    const Rect w{100, 100, 900, 700};
    CHECK_RECT(Calc(Action::ReverseAll, Calc(Action::ReverseAll, w)), w);
}

TEST(Bewegen_bleibt_in_der_Arbeitsflaeche) {
    Rect w{0, 0, 400, 300};
    for (int i = 0; i < 40; ++i) w = Calc(Action::MoveLeft, w);
    CHECK_EQ(w.left, kWork.left);
    for (int i = 0; i < 60; ++i) w = Calc(Action::MoveRight, w);
    CHECK_EQ(w.right, kWork.right);
}

TEST(Monitorwechsel_skaliert_proportional) {
    CalcInput in;
    in.action = Action::NextDisplay;
    in.window = Rect{0, 0, 960, 1040};  // linke Hälfte auf Monitor 1
    in.workArea = kWork;
    in.targetArea = kWork2;
    const Rect moved = *Calculate(in);
    // Auf dem Zielmonitor wieder die linke Hälfte, nicht dieselbe Pixelgröße.
    CHECK_RECT(moved, (Rect{1920, 0, 3200, 1400}));
}

TEST(Monitorwechsel_ohne_Ziel_tut_nichts) {
    CalcInput in;
    in.action = Action::NextDisplay;
    in.window = Rect{0, 0, 960, 1040};
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
}

TEST(Restore_liefert_den_gemerkten_Rahmen) {
    CalcInput in;
    in.action = Action::Restore;
    in.window = Rect{0, 0, 960, 1040};
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
    in.restoreRect = Rect{200, 150, 1000, 750};
    CHECK_RECT(*Calculate(in), (Rect{200, 150, 1000, 750}));
}

TEST(Mehrfenster_Aktionen_laufen_nicht_ueber_Calculate) {
    CalcInput in;
    in.action = Action::TileAll;
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
}

TEST(Alle_Aktionen_haben_eindeutige_Namen) {
    for (Action a : AllActions()) {
        Action back{};
        CHECK(ActionFromName(ActionName(a), back));
        CHECK(back == a);
        CHECK(!ActionLabel(a).empty());
    }
    Action unused{};
    CHECK(!ActionFromName("gibt-es-nicht", unused));
}

TEST(Kacheln_fuellt_die_Flaeche_restlos) {
    const auto three = LayoutWindows(Action::TileAll, 3, kWork, Gaps{});
    CHECK_EQ(three.size(), size_t{3});
    CHECK_RECT(three[0], (Rect{0, 0, 960, 520}));
    CHECK_RECT(three[1], (Rect{960, 0, 1920, 520}));
    // Die letzte Zeile ist nur halb besetzt und wird deshalb auf volle Breite verteilt.
    CHECK_RECT(three[2], (Rect{0, 520, 1920, 1040}));

    const auto four = LayoutWindows(Action::TileAll, 4, kWork, Gaps{});
    CHECK_RECT(four[3], (Rect{960, 520, 1920, 1040}));
}

TEST(Zeilen_und_Spalten) {
    const auto rows = LayoutWindows(Action::RowsAll, 3, kWork, Gaps{});
    CHECK_EQ(rows[0].Width(), kWork.Width());
    CHECK_EQ(rows[0].bottom, rows[1].top);
    CHECK_EQ(rows[2].bottom, kWork.bottom);

    const auto cols = LayoutWindows(Action::ColumnsAll, 3, kWork, Gaps{});
    CHECK_EQ(cols[0].Height(), kWork.Height());
    CHECK_EQ(cols[0].right, cols[1].left);
    CHECK_EQ(cols[2].right, kWork.right);
}

TEST(Staffeln_versetzt_jedes_Fenster_und_bleibt_im_Bild) {
    const auto c = LayoutWindows(Action::CascadeAll, 4, kWork, Gaps{});
    CHECK_EQ(c.size(), size_t{4});
    for (size_t i = 1; i < c.size(); ++i) {
        CHECK(c[i].left > c[i - 1].left);
        CHECK(c[i].top > c[i - 1].top);
    }
    for (const auto& r : c) {
        CHECK(r.right <= kWork.right && r.bottom <= kWork.bottom);
    }
}

TEST(Layout_ohne_Fenster_liefert_nichts) {
    CHECK(LayoutWindows(Action::TileAll, 0, kWork, Gaps{}).empty());
    CHECK(LayoutWindows(Action::LeftHalf, 3, kWork, Gaps{}).empty());
}

TEST(Historie_zaehlt_nur_ununterbrochene_Wiederholungen) {
    History h;
    const History::WindowId id = 1;
    const Rect before{100, 100, 900, 700};
    const Rect half{0, 0, 960, 1040};

    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, before), 0);
    h.Record(id, Action::LeftHalf, before, half);
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, half), 1);

    // Andere Aktion unterbricht die Kette.
    CHECK_EQ(h.RepeatCountFor(id, Action::RightHalf, half), 0);

    // Vom Benutzer verschoben -> Kette unterbrochen.
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, Rect{300, 300, 1260, 1340}), 0);

    // Kleine Korrektur durch die App selbst zählt noch als dieselbe Position.
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, Rect{0, 0, 956, 1040}), 1);
}

TEST(Historie_merkt_den_Rahmen_vor_dem_ersten_Eingriff) {
    History h;
    const History::WindowId id = 42;
    const Rect original{100, 100, 900, 700};

    CHECK(!h.RestoreRectFor(id).has_value());
    h.Record(id, Action::LeftHalf, original, Rect{0, 0, 960, 1040});
    h.Record(id, Action::Maximize, Rect{0, 0, 960, 1040}, kWork);
    // Auch nach zwei Aktionen zeigt Restore auf den Ursprungsrahmen.
    CHECK_RECT(*h.RestoreRectFor(id), original);

    h.Record(id, Action::Restore, kWork, original);
    CHECK(!h.RestoreRectFor(id).has_value());

    h.Forget(id);
    CHECK_EQ(h.Size(), size_t{0});
}
