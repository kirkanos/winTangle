#pragma once

#include <string>
#include <string_view>

namespace wintangle {

// The modifier bits are deliberately identical to RegisterHotKey's MOD_*
// values, so nothing has to be translated between config and Win32.
enum Modifier : unsigned {
    kModAlt = 0x0001,
    kModCtrl = 0x0002,
    kModShift = 0x0004,
    kModWin = 0x0008,
};

// A key combination: modifiers plus one virtual key code.
struct Shortcut {
    unsigned mods = 0;
    unsigned vk = 0;

    bool IsValid() const { return vk != 0; }
    bool operator==(const Shortcut& o) const { return mods == o.mods && vk == o.vk; }
    bool operator!=(const Shortcut& o) const { return !(*this == o); }
};

// "Ctrl+Alt+Left" -> Shortcut. Case insensitive; "Control", "Strg",
// "Win"/"Super"/"Meta" and "Option" are accepted as synonyms. Returns false
// when the key is unknown or no modifier is present -- a global hotkey without
// a modifier would swallow that key system wide.
bool ParseShortcut(std::string_view text, Shortcut& out);

// Canonical spelling, as written to the config file and shown in the settings
// window: "Ctrl+Alt+Shift+Win+Key".
std::string FormatShortcut(const Shortcut& s);

// Display name of a virtual key code ("Left", "F3", "A"), empty if unknown.
std::string_view KeyName(unsigned vk);

}  // namespace wintangle
