#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Language.h"

namespace wintangle {

// The full catalogue of actions, matching Rectangle one to one. This order is
// also the order used in the settings window and the tray menu.
enum class Action {
    // Halves
    LeftHalf, RightHalf, TopHalf, BottomHalf, CenterHalf,
    // Quarters
    TopLeft, TopRight, BottomLeft, BottomRight,
    // Thirds
    FirstThird, CenterThird, LastThird, FirstTwoThirds, LastTwoThirds,
    // Sixths
    TopLeftSixth, TopCenterSixth, TopRightSixth,
    BottomLeftSixth, BottomCenterSixth, BottomRightSixth,
    // Eighths
    TopLeftEighth, TopCenterLeftEighth, TopCenterRightEighth, TopRightEighth,
    BottomLeftEighth, BottomCenterLeftEighth, BottomCenterRightEighth, BottomRightEighth,
    // Ninths
    TopLeftNinth, TopCenterNinth, TopRightNinth,
    MiddleLeftNinth, MiddleCenterNinth, MiddleRightNinth,
    BottomLeftNinth, BottomCenterNinth, BottomRightNinth,
    // Size
    Maximize, AlmostMaximize, MaximizeHeight, MaximizeWidth,
    Larger, Smaller,
    // Position
    Center, CenterProminently, Restore, ReverseAll,
    MoveLeft, MoveRight, MoveUp, MoveDown,
    // Displays
    NextDisplay, PreviousDisplay,
    // Multiple windows
    TileAll, RowsAll, ColumnsAll, CascadeAll, CascadeActiveApp,

    Count_
};

// Stable identifier used in the config file, the wintangle:// URI and logs.
std::string_view ActionName(Action a);

// Human readable label for the tray menu and the settings window, in the
// requested language. Prefer LocalizedActionLabel() from Strings.h, which fills
// in the language currently in use.
std::string_view ActionLabel(Action a, Language language);

// Inverse of ActionName. Returns false for an unknown name.
bool ActionFromName(std::string_view name, Action& out);

// Every action in catalogue order.
const std::vector<Action>& AllActions();

// Actions that touch several windows at once and therefore do not go through
// Calculate() but through the multi-window path.
bool IsMultiWindowAction(Action a);

}  // namespace wintangle
