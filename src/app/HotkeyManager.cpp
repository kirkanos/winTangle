#include "HotkeyManager.h"

#include "core/Shortcut.h"

namespace wintangle {

std::vector<HotkeyManager::Conflict> HotkeyManager::Apply(const Config& config) {
    UnregisterAll();

    std::vector<Conflict> conflicts;
    int id = 1;  // 0 is valid, but too easy to confuse with "unset"

    for (Action action : AllActions()) {
        const auto shortcut = config.ShortcutFor(action);
        if (!shortcut || !shortcut->IsValid()) continue;

        // MOD_NOREPEAT: holding the key must not fire the action over and
        // over -- otherwise a held Ctrl+Alt+Left cycles wildly.
        const UINT mods = shortcut->mods | MOD_NOREPEAT;
        if (RegisterHotKey(window_, id, mods, shortcut->vk)) {
            byId_[id] = action;
            ++id;
        } else {
            conflicts.push_back({action, Widen(FormatShortcut(*shortcut))});
        }
    }
    return conflicts;
}

void HotkeyManager::UnregisterAll() {
    for (const auto& [id, action] : byId_) {
        UnregisterHotKey(window_, id);
    }
    byId_.clear();
}

bool HotkeyManager::ActionForId(int id, Action& out) const {
    const auto it = byId_.find(id);
    if (it == byId_.end()) return false;
    out = it->second;
    return true;
}

}  // namespace wintangle
