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

    // Kern: Haelften auf den Pfeiltasten -- Ctrl+Alt statt Win, weil Windows
    // Win+Pfeil fuer AeroSnap reserviert und RegisterHotKey das nicht bekommt.
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

    // Sechstel, Achtel, Neuntel, Zeilen/Spalten bleiben bewusst unbelegt --
    // sie sind ueber Tray-Menue und URI erreichbar und wuerden sonst die
    // Tastatur ueberfrachten.
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
    // In Katalogreihenfolge schreiben, damit die Datei stabil bleibt.
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
    root.Set("ignoredApps", ignored);
    root.Set("shortcuts", shortcutsObj);
    return root.Dump(2) + "\n";
}

bool Config::FromJson(std::string_view text, Config& out, std::string& error,
                      std::vector<std::string>* warnings) {
    json::Value root;
    if (!json::Parse(text, root, error)) return false;
    if (!root.IsObject()) {
        error = "Die Konfiguration muss ein JSON-Objekt sein";
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

    if (root["ignoredApps"].IsArray()) {
        c.ignoredApps.clear();
        for (const auto& v : root["ignoredApps"].AsArray()) {
            if (!v.AsString().empty()) c.ignoredApps.push_back(v.AsString());
        }
    }

    // Ein vorhandener shortcuts-Block ersetzt die Vorgaben vollstaendig,
    // sonst liessen sich Standardbelegungen nie loswerden.
    if (root["shortcuts"].IsObject()) {
        c.shortcuts.clear();
        for (const auto& [key, value] : root["shortcuts"].AsObject()) {
            Action a{};
            if (!ActionFromName(key, a)) {
                if (warnings) warnings->push_back("Unbekannte Aktion: " + key);
                continue;
            }
            // Leerer String heisst ausdruecklich "nicht belegt".
            if (value.AsString().empty()) continue;
            Shortcut s;
            if (!ParseShortcut(value.AsString(), s)) {
                if (warnings)
                    warnings->push_back("Unlesbare Tastenkombination fuer " + key + ": " +
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
    // Erst in eine temporaere Datei schreiben, dann umbenennen: ein Absturz
    // mitten im Speichern darf die Konfiguration nicht zerstoeren.
    const std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f) {
            error = "Konnte " + tmp + " nicht schreiben";
            return false;
        }
        f << ToJson();
        if (!f) {
            error = "Fehler beim Schreiben von " + tmp;
            return false;
        }
    }
    std::remove(path.c_str());
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        error = "Konnte " + tmp + " nicht nach " + path + " umbenennen";
        return false;
    }
    return true;
}

bool Config::LoadFromFile(const std::string& path, Config& out, std::string& error,
                          std::vector<std::string>* warnings) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        // Keine Datei ist kein Fehler: dann gelten die Vorgaben.
        out = Defaults();
        return true;
    }
    std::ostringstream buf;
    buf << f.rdbuf();
    return FromJson(buf.str(), out, error, warnings);
}

}  // namespace wintangle
