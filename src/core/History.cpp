#include "History.h"

#include <cstdlib>

namespace wintangle {

bool History::NearlyEqual(const Rect& a, const Rect& b) {
    return std::abs(a.left - b.left) <= kTolerance && std::abs(a.top - b.top) <= kTolerance &&
           std::abs(a.right - b.right) <= kTolerance &&
           std::abs(a.bottom - b.bottom) <= kTolerance;
}

int History::RepeatCountFor(WindowId id, Action action, const Rect& currentRect) const {
    const auto it = entries_.find(id);
    if (it == entries_.end()) return 0;
    const Entry& e = it->second;
    if (e.lastAction != action) return 0;
    // Von außen bewegt -> Kette gilt als unterbrochen.
    if (!NearlyEqual(e.appliedRect, currentRect)) return 0;
    return e.repeat;
}

std::optional<Rect> History::RestoreRectFor(WindowId id) const {
    const auto it = entries_.find(id);
    if (it == entries_.end() || !it->second.hasRestore) return std::nullopt;
    return it->second.restoreRect;
}

void History::Record(WindowId id, Action action, const Rect& before, const Rect& after) {
    Entry& e = entries_[id];

    const bool continues = e.lastAction == action && NearlyEqual(e.appliedRect, before);
    e.repeat = continues ? e.repeat + 1 : 1;

    // Der Restore-Rahmen bleibt der allererste; nach einem Restore ist er
    // verbraucht und wird beim nächsten Eingriff neu gesetzt.
    if (!e.hasRestore && action != Action::Restore) {
        e.restoreRect = before;
        e.hasRestore = true;
    }
    if (action == Action::Restore) {
        e.hasRestore = false;
    }

    e.lastAction = action;
    e.appliedRect = after;
}

void History::Forget(WindowId id) { entries_.erase(id); }

}  // namespace wintangle
