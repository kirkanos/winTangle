#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "Action.h"
#include "Rect.h"

namespace wintangle {

// Remembers per window what happened last. Two things depend on it:
//
//  1. Restore  the frame the window had before WinTangle first touched it.
//  2. Cycling  how often the same action ran back to back (left half ->
//              two thirds -> one third).
//
// Both become invalid as soon as the window is moved or resized from the
// outside. That is why the frame we set last is stored as well: if the current
// one differs, somebody else has been at it.
class History {
public:
    // Unique window id (the HWND as a number on Windows).
    using WindowId = std::uint64_t;

    struct Entry {
        Action lastAction = Action::Count_;
        int repeat = 0;        // number of identical invocations so far
        Rect appliedRect;      // what we set last
        Rect restoreRect;      // frame before the first action
        bool hasRestore = false;
    };

    // How often `action` has already been applied to this window back to
    // back. 0 when the chain is broken.
    int RepeatCountFor(WindowId id, Action action, const Rect& currentRect) const;

    // The frame before the first WinTangle action, if known.
    std::optional<Rect> RestoreRectFor(WindowId id) const;

    // Call after a successful invocation. `before` is the frame prior to the
    // action, `after` the one actually applied.
    void Record(WindowId id, Action action, const Rect& before, const Rect& after);

    // Window was closed, or the entry is no longer needed.
    void Forget(WindowId id);

    size_t Size() const { return entries_.size(); }

private:
    // Tolerance within which an applied frame still counts as "unchanged".
    // Some apps adjust their size by a few pixels (minimum size, grid
    // snapping); that must not tear the cycling chain apart.
    static constexpr int kTolerance = 8;

    static bool NearlyEqual(const Rect& a, const Rect& b);

    std::unordered_map<WindowId, Entry> entries_;
};

}  // namespace wintangle
