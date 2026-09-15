#include "Action.h"

#include <array>
#include <unordered_map>

namespace wintangle {
namespace {

struct Entry {
    Action action;
    std::string_view name;
    std::string_view labelEn;
    std::string_view labelDe;
};

// Single source of truth for name and labels. Order = catalogue order.
constexpr std::array<Entry, static_cast<size_t>(Action::Count_)> kEntries{{
    {Action::LeftHalf, "left-half", "Left Half", "Linke Hälfte"},
    {Action::RightHalf, "right-half", "Right Half", "Rechte Hälfte"},
    {Action::TopHalf, "top-half", "Top Half", "Obere Hälfte"},
    {Action::BottomHalf, "bottom-half", "Bottom Half", "Untere Hälfte"},
    {Action::CenterHalf, "center-half", "Center Half", "Mittlere Hälfte"},

    {Action::TopLeft, "top-left", "Top Left", "Oben links"},
    {Action::TopRight, "top-right", "Top Right", "Oben rechts"},
    {Action::BottomLeft, "bottom-left", "Bottom Left", "Unten links"},
    {Action::BottomRight, "bottom-right", "Bottom Right", "Unten rechts"},

    {Action::FirstThird, "first-third", "First Third", "Erstes Drittel"},
    {Action::CenterThird, "center-third", "Center Third", "Mittleres Drittel"},
    {Action::LastThird, "last-third", "Last Third", "Letztes Drittel"},
    {Action::FirstTwoThirds, "first-two-thirds", "First Two Thirds", "Erste zwei Drittel"},
    {Action::LastTwoThirds, "last-two-thirds", "Last Two Thirds", "Letzte zwei Drittel"},

    {Action::TopLeftSixth, "top-left-sixth", "Top Left Sixth", "Sechstel oben links"},
    {Action::TopCenterSixth, "top-center-sixth", "Top Center Sixth", "Sechstel oben Mitte"},
    {Action::TopRightSixth, "top-right-sixth", "Top Right Sixth", "Sechstel oben rechts"},
    {Action::BottomLeftSixth, "bottom-left-sixth", "Bottom Left Sixth", "Sechstel unten links"},
    {Action::BottomCenterSixth, "bottom-center-sixth", "Bottom Center Sixth", "Sechstel unten Mitte"},
    {Action::BottomRightSixth, "bottom-right-sixth", "Bottom Right Sixth", "Sechstel unten rechts"},

    {Action::TopLeftEighth, "top-left-eighth", "Top Eighth 1", "Achtel oben 1"},
    {Action::TopCenterLeftEighth, "top-center-left-eighth", "Top Eighth 2", "Achtel oben 2"},
    {Action::TopCenterRightEighth, "top-center-right-eighth", "Top Eighth 3", "Achtel oben 3"},
    {Action::TopRightEighth, "top-right-eighth", "Top Eighth 4", "Achtel oben 4"},
    {Action::BottomLeftEighth, "bottom-left-eighth", "Bottom Eighth 1", "Achtel unten 1"},
    {Action::BottomCenterLeftEighth, "bottom-center-left-eighth", "Bottom Eighth 2", "Achtel unten 2"},
    {Action::BottomCenterRightEighth, "bottom-center-right-eighth", "Bottom Eighth 3", "Achtel unten 3"},
    {Action::BottomRightEighth, "bottom-right-eighth", "Bottom Eighth 4", "Achtel unten 4"},

    {Action::TopLeftNinth, "top-left-ninth", "Top Left Ninth", "Neuntel oben links"},
    {Action::TopCenterNinth, "top-center-ninth", "Top Center Ninth", "Neuntel oben Mitte"},
    {Action::TopRightNinth, "top-right-ninth", "Top Right Ninth", "Neuntel oben rechts"},
    {Action::MiddleLeftNinth, "middle-left-ninth", "Middle Left Ninth", "Neuntel Mitte links"},
    {Action::MiddleCenterNinth, "middle-center-ninth", "Middle Center Ninth", "Neuntel Mitte"},
    {Action::MiddleRightNinth, "middle-right-ninth", "Middle Right Ninth", "Neuntel Mitte rechts"},
    {Action::BottomLeftNinth, "bottom-left-ninth", "Bottom Left Ninth", "Neuntel unten links"},
    {Action::BottomCenterNinth, "bottom-center-ninth", "Bottom Center Ninth", "Neuntel unten Mitte"},
    {Action::BottomRightNinth, "bottom-right-ninth", "Bottom Right Ninth", "Neuntel unten rechts"},

    {Action::Maximize, "maximize", "Maximize", "Maximieren"},
    {Action::AlmostMaximize, "almost-maximize", "Almost Maximize", "Fast maximieren"},
    {Action::MaximizeHeight, "maximize-height", "Maximize Height", "Höhe maximieren"},
    {Action::MaximizeWidth, "maximize-width", "Maximize Width", "Breite maximieren"},
    {Action::Larger, "larger", "Larger", "Größer"},
    {Action::Smaller, "smaller", "Smaller", "Kleiner"},

    {Action::Center, "center", "Center", "Zentrieren"},
    {Action::CenterProminently, "center-prominently", "Center Prominently", "Prominent zentrieren"},
    {Action::Restore, "restore", "Restore", "Wiederherstellen"},
    {Action::ReverseAll, "reverse-all", "Mirror", "Spiegeln"},
    {Action::MoveLeft, "move-left", "Move Left", "Nach links bewegen"},
    {Action::MoveRight, "move-right", "Move Right", "Nach rechts bewegen"},
    {Action::MoveUp, "move-up", "Move Up", "Nach oben bewegen"},
    {Action::MoveDown, "move-down", "Move Down", "Nach unten bewegen"},

    {Action::NextDisplay, "next-display", "Next Display", "Nächster Bildschirm"},
    {Action::PreviousDisplay, "previous-display", "Previous Display", "Vorheriger Bildschirm"},

    {Action::TileAll, "tile-all", "Tile All", "Alle kacheln"},
    {Action::RowsAll, "rows-all", "Arrange as Rows", "Alle als Zeilen"},
    {Action::ColumnsAll, "columns-all", "Arrange as Columns", "Alle als Spalten"},
    {Action::CascadeAll, "cascade-all", "Cascade All", "Alle staffeln"},
    {Action::CascadeActiveApp, "cascade-active-app", "Cascade Active App", "Aktive App staffeln"},
}};

const Entry& Lookup(Action a) { return kEntries[static_cast<size_t>(a)]; }

}  // namespace

std::string_view ActionName(Action a) { return Lookup(a).name; }

std::string_view ActionLabel(Action a, Language language) {
    const Entry& entry = Lookup(a);
    switch (language) {
        case Language::German: return entry.labelDe;
        case Language::English:
        default: return entry.labelEn;
    }
}

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
