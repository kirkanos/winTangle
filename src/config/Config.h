#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "core/Action.h"
#include "core/Rect.h"
#include "core/Shortcut.h"

namespace wintangle {

// The complete configuration. Platform free, so loading, saving and migrating
// stay testable without Windows; only the file path comes from the Win32 layer.
struct Config {
    // File format version, so later migrations remain possible.
    static constexpr int kVersion = 1;

    Gaps gaps;
    bool cycleSizes = true;       // repeated presses cycle 1/2 -> 2/3 -> 1/3
    bool snapAreasEnabled = true; // dragging to the screen edge
    bool disableWindowsSnap = false;  // turn off Windows' own AeroSnap
    bool launchAtLogin = false;
    bool moveCursorWithWindow = false;
    bool automaticUpdates = true;   // daily check via WinSparkle

    // Executable names (lower case, without path) WinTangle keeps its hands off.
    std::vector<std::string> ignoredApps;

    // Only bound actions appear here. An action without an entry deliberately
    // has no hotkey.
    std::map<Action, Shortcut> shortcuts;

    // Rectangle's defaults translated to Windows: ⌃⌥ becomes Ctrl+Alt, ⌘
    // becomes Win. Win+arrow stays free because Windows claims it itself.
    static Config Defaults();

    std::optional<Shortcut> ShortcutFor(Action a) const;
    bool IsIgnored(std::string_view exeName) const;

    std::string ToJson() const;

    // Reads the configuration. Unknown fields are ignored, missing ones are
    // filled in from the defaults. `warnings` collects everything that was
    // skipped (unknown action, unreadable shortcut) -- the file is never
    // discarded because of it.
    static bool FromJson(std::string_view text, Config& out, std::string& error,
                         std::vector<std::string>* warnings = nullptr);

    bool SaveToFile(const std::string& path, std::string& error) const;
    static bool LoadFromFile(const std::string& path, Config& out, std::string& error,
                             std::vector<std::string>* warnings = nullptr);
};

}  // namespace wintangle
