#pragma once

#include <map>
#include <string>
#include <vector>

#include "config/Config.h"
#include "core/Action.h"
#include "platform/Win32.h"

namespace wintangle {

// Registers the global hotkeys. Deliberately RegisterHotKey and not a
// WH_KEYBOARD_LL hook: the hook would funnel every keystroke on the system
// through our process (latency, and it looks like a keylogger to antivirus)
// and needs the same privileges for elevated windows anyway. The price: the
// combinations Windows claims for itself (Win+arrow) never reach us.
class HotkeyManager {
public:
    struct Conflict {
        Action action;
        std::wstring combo;
    };

    explicit HotkeyManager(HWND messageWindow) : window_(messageWindow) {}
    ~HotkeyManager() { UnregisterAll(); }

    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;

    // Registers everything afresh. Returns the combinations Windows refused
    // (usually already taken by another program) -- the app shows them to the
    // user instead of swallowing them silently.
    std::vector<Conflict> Apply(const Config& config);

    void UnregisterAll();

    // From WM_HOTKEY: yields the action for the reported id.
    bool ActionForId(int id, Action& out) const;

private:
    HWND window_;
    std::map<int, Action> byId_;
};

}  // namespace wintangle
