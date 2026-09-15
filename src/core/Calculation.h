#pragma once

#include <optional>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Everything an action needs in order to be calculated. Deliberately a plain
// value type: hotkey, tray menu, wintangle:// URI and drag-snap all fill in the
// same structure.
struct CalcInput {
    Action action = Action::LeftHalf;
    Rect window;    // current window frame (shadow corrected)
    Rect workArea;  // work area of the display the window sits on
    Gaps gaps;

    // Number of immediately preceding identical invocations on the same
    // window. 0 = first invocation.
    int repeat = 0;
    bool cycleSizes = true;

    // For Action::Restore.
    std::optional<Rect> restoreRect;

    // For Next/PreviousDisplay: work area of the target display.
    std::optional<Rect> targetArea;
};

// Calculates the target position. Returns nullopt when the action has nothing
// to do (restore without history, display switch with only one display) or when
// it runs through the multi-window path instead.
std::optional<Rect> Calculate(const CalcInput& in);

// Returns the fraction a grid action occupies, or nullopt for actions that are
// not grid based (center, larger, move, ...). `repeat` drives the size cycling
// of the half actions.
std::optional<Fraction> FractionFor(Action a, int repeat, bool cycleSizes);

// Maps a rect proportionally from one work area onto another. Needed when
// moving between displays, so a half-screen window stays a half on the target
// display instead of keeping its pixel size.
Rect MapRectToArea(const Rect& rect, const Rect& from, const Rect& to);

}  // namespace wintangle
