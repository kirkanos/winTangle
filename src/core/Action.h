#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace wintangle {

// Vollständiger Aktionskatalog, 1:1 zu Rectangle. Die Reihenfolge ist auch die
// Reihenfolge im Einstellungsdialog und im Tray-Menü.
enum class Action {
    // Hälften
    LeftHalf, RightHalf, TopHalf, BottomHalf, CenterHalf,
    // Viertel
    TopLeft, TopRight, BottomLeft, BottomRight,
    // Drittel
    FirstThird, CenterThird, LastThird, FirstTwoThirds, LastTwoThirds,
    // Sechstel
    TopLeftSixth, TopCenterSixth, TopRightSixth,
    BottomLeftSixth, BottomCenterSixth, BottomRightSixth,
    // Achtel
    TopLeftEighth, TopCenterLeftEighth, TopCenterRightEighth, TopRightEighth,
    BottomLeftEighth, BottomCenterLeftEighth, BottomCenterRightEighth, BottomRightEighth,
    // Neuntel
    TopLeftNinth, TopCenterNinth, TopRightNinth,
    MiddleLeftNinth, MiddleCenterNinth, MiddleRightNinth,
    BottomLeftNinth, BottomCenterNinth, BottomRightNinth,
    // Größe
    Maximize, AlmostMaximize, MaximizeHeight, MaximizeWidth,
    Larger, Smaller,
    // Position
    Center, CenterProminently, Restore, ReverseAll,
    MoveLeft, MoveRight, MoveUp, MoveDown,
    // Monitore
    NextDisplay, PreviousDisplay,
    // Mehrere Fenster
    TileAll, RowsAll, ColumnsAll, CascadeAll, CascadeActiveApp,

    Count_
};

// Stabiler Bezeichner für Konfigdatei, wintangle://-URI und Logs.
std::string_view ActionName(Action a);

// Menschenlesbare Beschriftung für Tray-Menü und Einstellungen.
std::string_view ActionLabel(Action a);

// Kehrt ActionName um. Gibt false zurück, wenn der Name unbekannt ist.
bool ActionFromName(std::string_view name, Action& out);

// Alle Aktionen in Katalogreihenfolge.
const std::vector<Action>& AllActions();

// Aktionen, die mehrere Fenster gleichzeitig anfassen und deshalb nicht über
// Calculate(), sondern über den Multi-Window-Pfad laufen.
bool IsMultiWindowAction(Action a);

}  // namespace wintangle
