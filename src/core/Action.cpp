#include "Action.h"

#include <array>
#include <unordered_map>

namespace wintangle {
namespace {

struct Entry {
    Action action;
    std::string_view name;
    std::string_view label;
};

// Single source of truth for name and label. Order = catalogue order.
constexpr std::array<Entry, static_cast<size_t>(Action::Count_)> kEntries{{
    {Action::LeftHalf, "left-half", "Left Half"},
    {Action::RightHalf, "right-half", "Right Half"},
    {Action::TopHalf, "top-half", "Top Half"},
    {Action::BottomHalf, "bottom-half", "Bottom Half"},
    {Action::CenterHalf, "center-half", "Center Half"},

    {Action::TopLeft, "top-left", "Top Left"},
    {Action::TopRight, "top-right", "Top Right"},
    {Action::BottomLeft, "bottom-left", "Bottom Left"},
    {Action::BottomRight, "bottom-right", "Bottom Right"},

    {Action::FirstThird, "first-third", "First Third"},
    {Action::CenterThird, "center-third", "Center Third"},
    {Action::LastThird, "last-third", "Last Third"},
    {Action::FirstTwoThirds, "first-two-thirds", "First Two Thirds"},
    {Action::LastTwoThirds, "last-two-thirds", "Last Two Thirds"},

    {Action::TopLeftSixth, "top-left-sixth", "Top Left Sixth"},
    {Action::TopCenterSixth, "top-center-sixth", "Top Center Sixth"},
    {Action::TopRightSixth, "top-right-sixth", "Top Right Sixth"},
    {Action::BottomLeftSixth, "bottom-left-sixth", "Bottom Left Sixth"},
    {Action::BottomCenterSixth, "bottom-center-sixth", "Bottom Center Sixth"},
    {Action::BottomRightSixth, "bottom-right-sixth", "Bottom Right Sixth"},

    {Action::TopLeftEighth, "top-left-eighth", "Top Eighth 1"},
    {Action::TopCenterLeftEighth, "top-center-left-eighth", "Top Eighth 2"},
    {Action::TopCenterRightEighth, "top-center-right-eighth", "Top Eighth 3"},
    {Action::TopRightEighth, "top-right-eighth", "Top Eighth 4"},
    {Action::BottomLeftEighth, "bottom-left-eighth", "Bottom Eighth 1"},
    {Action::BottomCenterLeftEighth, "bottom-center-left-eighth", "Bottom Eighth 2"},
    {Action::BottomCenterRightEighth, "bottom-center-right-eighth", "Bottom Eighth 3"},
    {Action::BottomRightEighth, "bottom-right-eighth", "Bottom Eighth 4"},

    {Action::TopLeftNinth, "top-left-ninth", "Top Left Ninth"},
    {Action::TopCenterNinth, "top-center-ninth", "Top Center Ninth"},
    {Action::TopRightNinth, "top-right-ninth", "Top Right Ninth"},
    {Action::MiddleLeftNinth, "middle-left-ninth", "Middle Left Ninth"},
    {Action::MiddleCenterNinth, "middle-center-ninth", "Middle Center Ninth"},
    {Action::MiddleRightNinth, "middle-right-ninth", "Middle Right Ninth"},
    {Action::BottomLeftNinth, "bottom-left-ninth", "Bottom Left Ninth"},
    {Action::BottomCenterNinth, "bottom-center-ninth", "Bottom Center Ninth"},
    {Action::BottomRightNinth, "bottom-right-ninth", "Bottom Right Ninth"},

    {Action::Maximize, "maximize", "Maximize"},
    {Action::AlmostMaximize, "almost-maximize", "Almost Maximize"},
    {Action::MaximizeHeight, "maximize-height", "Maximize Height"},
    {Action::MaximizeWidth, "maximize-width", "Maximize Width"},
    {Action::Larger, "larger", "Larger"},
    {Action::Smaller, "smaller", "Smaller"},

    {Action::Center, "center", "Center"},
    {Action::CenterProminently, "center-prominently", "Center Prominently"},
    {Action::Restore, "restore", "Restore"},
    {Action::ReverseAll, "reverse-all", "Mirror"},
    {Action::MoveLeft, "move-left", "Move Left"},
    {Action::MoveRight, "move-right", "Move Right"},
    {Action::MoveUp, "move-up", "Move Up"},
    {Action::MoveDown, "move-down", "Move Down"},

    {Action::NextDisplay, "next-display", "Next Display"},
    {Action::PreviousDisplay, "previous-display", "Previous Display"},

    {Action::TileAll, "tile-all", "Tile All"},
    {Action::RowsAll, "rows-all", "Arrange as Rows"},
    {Action::ColumnsAll, "columns-all", "Arrange as Columns"},
    {Action::CascadeAll, "cascade-all", "Cascade All"},
    {Action::CascadeActiveApp, "cascade-active-app", "Cascade Active App"},
}};

const Entry& Lookup(Action a) { return kEntries[static_cast<size_t>(a)]; }

}  // namespace

std::string_view ActionName(Action a) { return Lookup(a).name; }
std::string_view ActionLabel(Action a) { return Lookup(a).label; }

bool ActionFromName(std::string_view name, Action& out) {
    static const std::unordered_map<std::string_view, Action> kByName = [] {
        std::unordered_map<std::string_view, Action> m;
        for (const auto& e : kEntries) m.emplace(e.name, e.action);
        return m;
    }();
    const auto it = kByName.find(name);
    if (it == kByName.end()) return false;
    out = it->second;
    return true;
}

const std::vector<Action>& AllActions() {
    static const std::vector<Action> kAll = [] {
        std::vector<Action> v;
        v.reserve(kEntries.size());
        for (const auto& e : kEntries) v.push_back(e.action);
        return v;
    }();
    return kAll;
}

bool IsMultiWindowAction(Action a) {
    switch (a) {
        case Action::TileAll:
        case Action::RowsAll:
        case Action::ColumnsAll:
        case Action::CascadeAll:
        case Action::CascadeActiveApp:
            return true;
        default:
            return false;
    }
}

}  // namespace wintangle
