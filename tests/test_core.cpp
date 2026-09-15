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

// A typical 1080p display with a 40px taskbar at the bottom.
const Rect kWork{0, 0, 1920, 1040};
// A second display to its right, different resolution.
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

TEST(Halves_split_the_work_area_exactly) {
    CHECK_RECT(Calc(Action::LeftHalf), (Rect{0, 0, 960, 1040}));
    CHECK_RECT(Calc(Action::RightHalf), (Rect{960, 0, 1920, 1040}));
    CHECK_RECT(Calc(Action::TopHalf), (Rect{0, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomHalf), (Rect{0, 520, 1920, 1040}));
    CHECK_RECT(Calc(Action::CenterHalf), (Rect{480, 0, 1440, 1040}));
}

TEST(Quarters_meet_without_a_seam) {
    CHECK_RECT(Calc(Action::TopLeft), (Rect{0, 0, 960, 520}));
    CHECK_RECT(Calc(Action::TopRight), (Rect{960, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomLeft), (Rect{0, 520, 960, 1040}));
    CHECK_RECT(Calc(Action::BottomRight), (Rect{960, 520, 1920, 1040}));
}

TEST(Thirds_leave_no_pixel_gap) {
    const Rect a = Calc(Action::FirstThird);
    const Rect b = Calc(Action::CenterThird);
    const Rect c = Calc(Action::LastThird);
    CHECK_EQ(a.right, b.left);
    CHECK_EQ(b.right, c.left);
    CHECK_EQ(c.right, kWork.right);
    CHECK_RECT(Calc(Action::FirstTwoThirds), (Rect{0, 0, 1280, 1040}));
    CHECK_RECT(Calc(Action::LastTwoThirds), (Rect{640, 0, 1920, 1040}));
}

TEST(Ninths_cover_the_area_completely) {
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

TEST(Eighths_form_four_columns_by_two_rows) {
    CHECK_RECT(Calc(Action::TopLeftEighth), (Rect{0, 0, 480, 520}));
    CHECK_RECT(Calc(Action::TopRightEighth), (Rect{1440, 0, 1920, 520}));
    CHECK_RECT(Calc(Action::BottomCenterLeftEighth), (Rect{480, 520, 960, 1040}));
}

TEST(Repeated_presses_cycle_the_size) {
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 0), (Rect{0, 0, 960, 1040}));
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 1), (Rect{0, 0, 1280, 1040}));
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 2), (Rect{0, 0, 640, 1040}));
    // After three steps the cycle starts over.
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 3), (Rect{0, 0, 960, 1040}));
    // The right half cycles mirrored, so it stays flush right.
    CHECK_RECT(Calc(Action::RightHalf, Rect{}, 1), (Rect{640, 0, 1920, 1040}));
}

TEST(Cycling_can_be_turned_off) {
    CalcInput in;
    in.action = Action::LeftHalf;
    in.workArea = kWork;
    in.repeat = 1;
    in.cycleSizes = false;
    CHECK_RECT(*Calculate(in), (Rect{0, 0, 960, 1040}));
}

TEST(Gaps_apply_fully_outside_and_halved_inside) {
    const Gaps gaps{10, 10};
    CHECK_RECT(Calc(Action::LeftHalf, Rect{}, 0, gaps), (Rect{10, 10, 955, 1030}));
    CHECK_RECT(Calc(Action::RightHalf, Rect{}, 0, gaps), (Rect{965, 10, 1910, 1030}));
    CHECK_RECT(Calc(Action::Maximize, Rect{}, 0, gaps), (Rect{10, 10, 1910, 1030}));
    // Two adjacent halves are exactly the inner gap apart.
    CHECK_EQ(Calc(Action::RightHalf, Rect{}, 0, gaps).left -
                 Calc(Action::LeftHalf, Rect{}, 0, gaps).right,
             10);
}

TEST(Maximize_and_almost_maximize) {
    CHECK_RECT(Calc(Action::Maximize), kWork);
    CHECK_RECT(Calc(Action::AlmostMaximize), (Rect{96, 52, 1824, 988}));
}

TEST(Maximizing_height_or_width_leaves_the_other_axis_alone) {
    const Rect w{300, 200, 800, 600};
    CHECK_RECT(Calc(Action::MaximizeHeight, w), (Rect{300, 0, 800, 1040}));
    CHECK_RECT(Calc(Action::MaximizeWidth, w), (Rect{0, 200, 1920, 600}));
}

TEST(Centering_keeps_the_size) {
    const Rect w{0, 0, 800, 600};
    const Rect centered = Calc(Action::Center, w);
    CHECK_EQ(centered.Width(), 800);
    CHECK_EQ(centered.Height(), 600);
    CHECK_EQ(centered.CenterX(), kWork.CenterX());
    CHECK_EQ(centered.CenterY(), kWork.CenterY());
}

TEST(Center_prominently_sits_higher_than_centre) {
    const Rect w{0, 0, 800, 600};
    const Rect prominent = Calc(Action::CenterProminently, w);
    CHECK(prominent.CenterY() < kWork.CenterY());
    CHECK(prominent.Width() > w.Width());
    CHECK(prominent.left >= kWork.left && prominent.right <= kWork.right);
}

TEST(Larger_and_smaller_keep_the_centre) {
    const Rect w{560, 320, 1360, 720};
    const Rect larger = Calc(Action::Larger, w);
    CHECK_EQ(larger.CenterX(), w.CenterX());
    CHECK_EQ(larger.CenterY(), w.CenterY());
    CHECK(larger.Width() > w.Width());
    CHECK(Calc(Action::Smaller, w).Width() < w.Width());
}

TEST(Larger_never_bursts_the_work_area) {
    Rect w = kWork;
    for (int i = 0; i < 5; ++i) w = Calc(Action::Larger, w);
    CHECK(w.left >= kWork.left && w.right <= kWork.right);
    CHECK(w.top >= kWork.top && w.bottom <= kWork.bottom);
}

TEST(Smaller_has_a_lower_bound) {
    Rect w{0, 0, 800, 600};
    for (int i = 0; i < 20; ++i) w = Calc(Action::Smaller, w);
    CHECK(w.Width() >= 240);
    CHECK(w.Height() >= 160);
}

TEST(Mirroring_swaps_left_and_right) {
    CHECK_RECT(Calc(Action::ReverseAll, Rect{0, 0, 640, 1040}), (Rect{1280, 0, 1920, 1040}));
    // Mirroring twice yields the original again.
    const Rect w{100, 100, 900, 700};
    CHECK_RECT(Calc(Action::ReverseAll, Calc(Action::ReverseAll, w)), w);
}

TEST(Moving_stays_inside_the_work_area) {
    Rect w{0, 0, 400, 300};
    for (int i = 0; i < 40; ++i) w = Calc(Action::MoveLeft, w);
    CHECK_EQ(w.left, kWork.left);
    for (int i = 0; i < 60; ++i) w = Calc(Action::MoveRight, w);
    CHECK_EQ(w.right, kWork.right);
}

TEST(Switching_display_scales_proportionally) {
    CalcInput in;
    in.action = Action::NextDisplay;
    in.window = Rect{0, 0, 960, 1040};  // left half on display 1
    in.workArea = kWork;
    in.targetArea = kWork2;
    const Rect moved = *Calculate(in);
    // The left half again on the target display, not the same pixel size.
    CHECK_RECT(moved, (Rect{1920, 0, 3200, 1400}));
}

TEST(Switching_display_without_a_target_does_nothing) {
    CalcInput in;
    in.action = Action::NextDisplay;
    in.window = Rect{0, 0, 960, 1040};
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
}

TEST(Restore_yields_the_remembered_frame) {
    CalcInput in;
    in.action = Action::Restore;
    in.window = Rect{0, 0, 960, 1040};
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
    in.restoreRect = Rect{200, 150, 1000, 750};
    CHECK_RECT(*Calculate(in), (Rect{200, 150, 1000, 750}));
}

TEST(Multi_window_actions_do_not_go_through_Calculate) {
    CalcInput in;
    in.action = Action::TileAll;
    in.workArea = kWork;
    CHECK(!Calculate(in).has_value());
}

TEST(Every_action_has_a_unique_name) {
    for (Action a : AllActions()) {
        Action back{};
        CHECK(ActionFromName(ActionName(a), back));
        CHECK(back == a);
        CHECK(!ActionLabel(a, Language::English).empty());
    }
    Action unused{};
    CHECK(!ActionFromName("no-such-action", unused));
}

TEST(Tiling_fills_the_area_completely) {
    const auto three = LayoutWindows(Action::TileAll, 3, kWork, Gaps{});
    CHECK_EQ(three.size(), size_t{3});
    CHECK_RECT(three[0], (Rect{0, 0, 960, 520}));
    CHECK_RECT(three[1], (Rect{960, 0, 1920, 520}));
    // The last row is only half filled and therefore spread to full width.
    CHECK_RECT(three[2], (Rect{0, 520, 1920, 1040}));

    const auto four = LayoutWindows(Action::TileAll, 4, kWork, Gaps{});
    CHECK_RECT(four[3], (Rect{960, 520, 1920, 1040}));
}

TEST(Rows_and_columns) {
    const auto rows = LayoutWindows(Action::RowsAll, 3, kWork, Gaps{});
    CHECK_EQ(rows[0].Width(), kWork.Width());
    CHECK_EQ(rows[0].bottom, rows[1].top);
    CHECK_EQ(rows[2].bottom, kWork.bottom);

    const auto cols = LayoutWindows(Action::ColumnsAll, 3, kWork, Gaps{});
    CHECK_EQ(cols[0].Height(), kWork.Height());
    CHECK_EQ(cols[0].right, cols[1].left);
    CHECK_EQ(cols[2].right, kWork.right);
}

TEST(Cascading_offsets_every_window_and_stays_on_screen) {
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

TEST(Layout_without_windows_yields_nothing) {
    CHECK(LayoutWindows(Action::TileAll, 0, kWork, Gaps{}).empty());
    CHECK(LayoutWindows(Action::LeftHalf, 3, kWork, Gaps{}).empty());
}

TEST(History_counts_only_unbroken_repetitions) {
    History h;
    const History::WindowId id = 1;
    const Rect before{100, 100, 900, 700};
    const Rect half{0, 0, 960, 1040};

    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, before), 0);
    h.Record(id, Action::LeftHalf, before, half);
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, half), 1);

    // A different action breaks the chain.
    CHECK_EQ(h.RepeatCountFor(id, Action::RightHalf, half), 0);

    // Moved by the user -> chain broken.
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, Rect{300, 300, 1260, 1340}), 0);

    // A small correction by the app itself still counts as the same position.
    CHECK_EQ(h.RepeatCountFor(id, Action::LeftHalf, Rect{0, 0, 956, 1040}), 1);
}

TEST(History_remembers_the_frame_before_the_first_change) {
    History h;
    const History::WindowId id = 42;
    const Rect original{100, 100, 900, 700};

    CHECK(!h.RestoreRectFor(id).has_value());
    h.Record(id, Action::LeftHalf, original, Rect{0, 0, 960, 1040});
    h.Record(id, Action::Maximize, Rect{0, 0, 960, 1040}, kWork);
    // Even after two actions, restore still points at the original frame.
    CHECK_RECT(*h.RestoreRectFor(id), original);

    h.Record(id, Action::Restore, kWork, original);
    CHECK(!h.RestoreRectFor(id).has_value());

    h.Forget(id);
    CHECK_EQ(h.Size(), size_t{0});
}
