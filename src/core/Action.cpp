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

// Einzige Quelle der Wahrheit für Name/Label. Reihenfolge = Katalogreihenfolge.
constexpr std::array<Entry, static_cast<size_t>(Action::Count_)> kEntries{{
    {Action::LeftHalf, "left-half", "Linke Hälfte"},
    {Action::RightHalf, "right-half", "Rechte Hälfte"},
    {Action::TopHalf, "top-half", "Obere Hälfte"},
    {Action::BottomHalf, "bottom-half", "Untere Hälfte"},
    {Action::CenterHalf, "center-half", "Mittlere Hälfte"},

    {Action::TopLeft, "top-left", "Oben links"},
    {Action::TopRight, "top-right", "Oben rechts"},
    {Action::BottomLeft, "bottom-left", "Unten links"},
    {Action::BottomRight, "bottom-right", "Unten rechts"},

    {Action::FirstThird, "first-third", "Erstes Drittel"},
    {Action::CenterThird, "center-third", "Mittleres Drittel"},
    {Action::LastThird, "last-third", "Letztes Drittel"},
    {Action::FirstTwoThirds, "first-two-thirds", "Erste zwei Drittel"},
    {Action::LastTwoThirds, "last-two-thirds", "Letzte zwei Drittel"},

    {Action::TopLeftSixth, "top-left-sixth", "Sechstel oben links"},
    {Action::TopCenterSixth, "top-center-sixth", "Sechstel oben Mitte"},
    {Action::TopRightSixth, "top-right-sixth", "Sechstel oben rechts"},
    {Action::BottomLeftSixth, "bottom-left-sixth", "Sechstel unten links"},
    {Action::BottomCenterSixth, "bottom-center-sixth", "Sechstel unten Mitte"},
    {Action::BottomRightSixth, "bottom-right-sixth", "Sechstel unten rechts"},

    {Action::TopLeftEighth, "top-left-eighth", "Achtel oben 1"},
    {Action::TopCenterLeftEighth, "top-center-left-eighth", "Achtel oben 2"},
    {Action::TopCenterRightEighth, "top-center-right-eighth", "Achtel oben 3"},
    {Action::TopRightEighth, "top-right-eighth", "Achtel oben 4"},
    {Action::BottomLeftEighth, "bottom-left-eighth", "Achtel unten 1"},
    {Action::BottomCenterLeftEighth, "bottom-center-left-eighth", "Achtel unten 2"},
    {Action::BottomCenterRightEighth, "bottom-center-right-eighth", "Achtel unten 3"},
    {Action::BottomRightEighth, "bottom-right-eighth", "Achtel unten 4"},

    {Action::TopLeftNinth, "top-left-ninth", "Neuntel oben links"},
    {Action::TopCenterNinth, "top-center-ninth", "Neuntel oben Mitte"},
    {Action::TopRightNinth, "top-right-ninth", "Neuntel oben rechts"},
    {Action::MiddleLeftNinth, "middle-left-ninth", "Neuntel Mitte links"},
    {Action::MiddleCenterNinth, "middle-center-ninth", "Neuntel Mitte"},
    {Action::MiddleRightNinth, "middle-right-ninth", "Neuntel Mitte rechts"},
    {Action::BottomLeftNinth, "bottom-left-ninth", "Neuntel unten links"},
    {Action::BottomCenterNinth, "bottom-center-ninth", "Neuntel unten Mitte"},
    {Action::BottomRightNinth, "bottom-right-ninth", "Neuntel unten rechts"},

    {Action::Maximize, "maximize", "Maximieren"},
    {Action::AlmostMaximize, "almost-maximize", "Fast maximieren"},
    {Action::MaximizeHeight, "maximize-height", "Höhe maximieren"},
    {Action::MaximizeWidth, "maximize-width", "Breite maximieren"},
    {Action::Larger, "larger", "Größer"},
    {Action::Smaller, "smaller", "Kleiner"},

    {Action::Center, "center", "Zentrieren"},
    {Action::CenterProminently, "center-prominently", "Prominent zentrieren"},
    {Action::Restore, "restore", "Wiederherstellen"},
    {Action::ReverseAll, "reverse-all", "Spiegeln"},
    {Action::MoveLeft, "move-left", "Nach links bewegen"},
    {Action::MoveRight, "move-right", "Nach rechts bewegen"},
    {Action::MoveUp, "move-up", "Nach oben bewegen"},
    {Action::MoveDown, "move-down", "Nach unten bewegen"},

    {Action::NextDisplay, "next-display", "Nächster Bildschirm"},
    {Action::PreviousDisplay, "previous-display", "Vorheriger Bildschirm"},

    {Action::TileAll, "tile-all", "Alle kacheln"},
    {Action::RowsAll, "rows-all", "Alle als Zeilen"},
    {Action::ColumnsAll, "columns-all", "Alle als Spalten"},
    {Action::CascadeAll, "cascade-all", "Alle staffeln"},
    {Action::CascadeActiveApp, "cascade-active-app", "Aktive App staffeln"},
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
