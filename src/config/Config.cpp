#include "Config.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "core/Json.h"

namespace wintangle {
namespace {

std::string Lower(std::string_view s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

void Bind(Config& c, Action a, const char* combo) {
    Shortcut s;
    if (ParseShortcut(combo, s)) c.shortcuts[a] = s;
}

}  // namespace

Config Config::Defaults() {
    Config c;
    c.gaps = Gaps{0, 0};

    // The core: halves on the arrow keys -- Ctrl+Alt rather than Win, because
    // Windows reserves Win+arrow for AeroSnap and RegisterHotKey never sees it.
    Bind(c, Action::LeftHalf, "Ctrl+Alt+Left");
    Bind(c, Action::RightHalf, "Ctrl+Alt+Right");
    Bind(c, Action::TopHalf, "Ctrl+Alt+Up");
    Bind(c, Action::BottomHalf, "Ctrl+Alt+Down");
    Bind(c, Action::CenterHalf, "Ctrl+Alt+Shift+C");

    Bind(c, Action::TopLeft, "Ctrl+Alt+U");
    Bind(c, Action::TopRight, "Ctrl+Alt+I");
    Bind(c, Action::BottomLeft, "Ctrl+Alt+J");
    Bind(c, Action::BottomRight, "Ctrl+Alt+K");

    Bind(c, Action::FirstThird, "Ctrl+Alt+D");
    Bind(c, Action::CenterThird, "Ctrl+Alt+F");
    Bind(c, Action::LastThird, "Ctrl+Alt+G");
    Bind(c, Action::FirstTwoThirds, "Ctrl+Alt+E");
    Bind(c, Action::LastTwoThirds, "Ctrl+Alt+T");

    Bind(c, Action::Maximize, "Ctrl+Alt+Enter");
    Bind(c, Action::AlmostMaximize, "Ctrl+Alt+Shift+Enter");
    Bind(c, Action::MaximizeHeight, "Ctrl+Alt+Shift+Up");
    Bind(c, Action::MaximizeWidth, "Ctrl+Alt+Shift+Down");

    Bind(c, Action::Larger, "Ctrl+Alt+Plus");
    Bind(c, Action::Smaller, "Ctrl+Alt+Minus");

    Bind(c, Action::Center, "Ctrl+Alt+C");
    Bind(c, Action::Restore, "Ctrl+Alt+Backspace");

    Bind(c, Action::MoveLeft, "Ctrl+Alt+Shift+Left");
    Bind(c, Action::MoveRight, "Ctrl+Alt+Shift+Right");

    Bind(c, Action::NextDisplay, "Ctrl+Alt+Win+Right");
    Bind(c, Action::PreviousDisplay, "Ctrl+Alt+Win+Left");

    Bind(c, Action::TileAll, "Ctrl+Alt+Shift+T");
    Bind(c, Action::CascadeAll, "Ctrl+Alt+Shift+S");

    // Sixths, eighths, ninths and rows/columns are deliberately left unbound:
    // they are reachable from the tray menu and the URI, and would otherwise
    // clutter the keyboard.
    return c;
}

std::optional<Shortcut> Config::ShortcutFor(Action a) const {
    const auto it = shortcuts.find(a);
    if (it == shortcuts.end()) return std::nullopt;
    return it->second;
}

bool Config::IsIgnored(std::string_view exeName) const {
    const std::string needle = Lower(exeName);
    return std::any_of(ignoredApps.begin(), ignoredApps.end(),
                       [&](const std::string& s) { return Lower(s) == needle; });
}

std::string Config::ToJson() const {
    using json::Value;

    Value gapsObj;
    gapsObj.Set("outer", gaps.outer);
    gapsObj.Set("inner", gaps.inner);

    Value shortcutsObj;
    // Write in catalogue order so the file stays stable.
    for (Action a : AllActions()) {
        const auto it = shortcuts.find(a);
        if (it == shortcuts.end()) continue;
        shortcutsObj.Set(std::string(ActionName(a)), Value(FormatShortcut(it->second)));
    }

    Value ignored;
    for (const auto& app : ignoredApps) ignored.Push(Value(app));
    if (ignoredApps.empty()) ignored = Value(json::Array{});

    Value root;
    root.Set("version", kVersion);
    root.Set("gaps", gapsObj);
    root.Set("cycleSizes", cycleSizes);
    root.Set("snapAreasEnabled", snapAreasEnabled);
    root.Set("disableWindowsSnap", disableWindowsSnap);
    root.Set("launchAtLogin", launchAtLogin);
    root.Set("moveCursorWithWindow", moveCursorWithWindow);
    root.Set("automaticUpdates", automaticUpdates);
    root.Set("language", language ? std::string(LanguageTag(*language)) : std::string("auto"));
    root.Set("ignoredApps", ignored);
    root.Set("shortcuts", shortcutsObj);
    return root.Dump(2) + "\n";
}

bool Config::FromJson(std::string_view text, Config& out, std::string& error,
                      std::vector<std::string>* warnings) {
    json::Value root;
    if (!json::Parse(text, root, error)) return false;
    if (!root.IsObject()) {
        error = "The configuration must be a JSON object";
        return false;
    }

    Config c = Defaults();

    if (root.Has("gaps")) {
        const auto& g = root["gaps"];
        c.gaps.outer = std::max(0, g["outer"].AsInt(c.gaps.outer));
        c.gaps.inner = std::max(0, g["inner"].AsInt(c.gaps.inner));
    }
    if (root.Has("cycleSizes")) c.cycleSizes = root["cycleSizes"].AsBool(c.cycleSizes);
    if (root.Has("snapAreasEnabled"))
        c.snapAreasEnabled = root["snapAreasEnabled"].AsBool(c.snapAreasEnabled);
    if (root.Has("disableWindowsSnap"))
        c.disableWindowsSnap = root["disableWindowsSnap"].AsBool(c.disableWindowsSnap);
    if (root.Has("launchAtLogin"))
        c.launchAtLogin = root["launchAtLogin"].AsBool(c.launchAtLogin);
    if (root.Has("moveCursorWithWindow"))
        c.moveCursorWithWindow = root["moveCursorWithWindow"].AsBool(c.moveCursorWithWindow);
    if (root.Has("automaticUpdates"))
        c.automaticUpdates = root["automaticUpdates"].AsBool(c.automaticUpdates);

    if (root.Has("language")) {
        const std::string& tag = root["language"].AsString();
        Language language{};
        if (tag == "auto" || tag.empty()) {
            c.language.reset();
        } else if (LanguageFromTag(tag, language)) {
            c.language = language;
        } else if (warnings) {
            warnings->push_back("Unknown language: " + tag);
        }
    }

    if (root["ignoredApps"].IsArray()) {
        c.ignoredApps.clear();
        for (const auto& v : root["ignoredApps"].AsArray()) {
            if (!v.AsString().empty()) c.ignoredApps.push_back(v.AsString());
        }
    }

    // A shortcuts block replaces the defaults completely; otherwise there
    // would be no way to get rid of a default binding.
    if (root["shortcuts"].IsObject()) {
        c.shortcuts.clear();
        for (const auto& [key, value] : root["shortcuts"].AsObject()) {
            Action a{};
            if (!ActionFromName(key, a)) {
                if (warnings) warnings->push_back("Unknown action: " + key);
                continue;
            }
            // An empty string explicitly means "not bound".
            if (value.AsString().empty()) continue;
            Shortcut s;
            if (!ParseShortcut(value.AsString(), s)) {
                if (warnings)
                    warnings->push_back("Unreadable key combination for " + key + ": " +
                                        value.AsString());
                continue;
            }
            c.shortcuts[a] = s;
        }
    }

    out = std::move(c);
    return true;
}

bool Config::SaveToFile(const std::string& path, std::string& error) const {
    // Write to a temporary file first, then rename: a crash in the middle of
    // saving must not destroy the configuration.
    const std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f) {
            error = "Could not write " + tmp;
            return false;
        }
        f << ToJson();
        if (!f) {
            error = "Error while writing " + tmp;
            return false;
        }
    }
    std::remove(path.c_str());
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        error = "Could not rename " + tmp + " to " + path;
        return false;
    }
    return true;
}

bool Config::LoadFromFile(const std::string& path, Config& out, std::string& error,
                          std::vector<std::string>* warnings) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        // A missing file is not an error: the defaults apply then.
        out = Defaults();
        return true;
    }
    std::ostringstream buf;
    buf << f.rdbuf();
    return FromJson(buf.str(), out, error, warnings);
}

}  // namespace wintangle
