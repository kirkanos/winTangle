#include <set>
#include <string>

#include "check.h"
#include "config/Config.h"
#include "core/Json.h"
#include "core/Shortcut.h"

using namespace wintangle;

TEST(Json_liest_und_schreibt_verschachtelte_Werte) {
    json::Value v;
    std::string err;
    const std::string text = R"({
        "a": 1, "b": "zwei", "c": true, "d": null,
        "e": [1, 2, {"f": -3.5}],
        "g": {"h": {"i": "tief"}}
    })";
    CHECK(json::Parse(text, v, err));
    CHECK_EQ(v["a"].AsInt(), 1);
    CHECK_EQ(v["b"].AsString(), std::string("zwei"));
    CHECK(v["c"].AsBool());
    CHECK(v["d"].IsNull());
    CHECK_EQ(v["e"].AsArray().size(), size_t{3});
    CHECK_EQ(v["e"].AsArray()[2]["f"].AsNumber(), -3.5);
    CHECK_EQ(v["g"]["h"]["i"].AsString(), std::string("tief"));
    // Fehlende Felder liefern Null statt zu werfen.
    CHECK(v["gibtsnicht"].IsNull());
    CHECK_EQ(v["gibtsnicht"]["auch-nicht"].AsInt(7), 7);
}

TEST(Json_meldet_Fehler_mit_Position) {
    json::Value v;
    std::string err;
    CHECK(!json::Parse("{\"a\": }", v, err));
    CHECK(!err.empty());
    CHECK(err.find("Zeile") != std::string::npos);
}

TEST(Json_ueberlebt_eine_Rundreise) {
    json::Value v;
    std::string err;
    CHECK(json::Parse(R"({"s":"a\"b\n","n":42,"arr":[],"obj":{}})", v, err));
    json::Value again;
    CHECK(json::Parse(v.Dump(), again, err));
    CHECK_EQ(again["s"].AsString(), std::string("a\"b\n"));
    CHECK_EQ(again["n"].AsInt(), 42);
    CHECK(again["arr"].IsArray());
    CHECK(again["obj"].IsObject());
}

TEST(Json_erlaubt_Zeilenkommentare_in_der_Konfig) {
    json::Value v;
    std::string err;
    CHECK(json::Parse("{\n // Kommentar\n \"a\": 1\n}", v, err));
    CHECK_EQ(v["a"].AsInt(), 1);
}

TEST(Shortcuts_werden_gelesen_und_kanonisch_zurueckgeschrieben) {
    Shortcut s;
    CHECK(ParseShortcut("Ctrl+Alt+Left", s));
    CHECK_EQ(s.mods, unsigned{kModCtrl | kModAlt});
    CHECK_EQ(s.vk, unsigned{0x25});
    CHECK_EQ(FormatShortcut(s), std::string("Ctrl+Alt+Left"));

    // Synonyme und Reihenfolge sind egal, die Ausgabe ist es nicht.
    Shortcut t;
    CHECK(ParseShortcut(" alt + strg + LEFT ", t));
    CHECK(t == s);
    CHECK(ParseShortcut("Win+Shift+F12", t));
    CHECK_EQ(FormatShortcut(t), std::string("Shift+Win+F12"));
    CHECK(ParseShortcut("Ctrl+Alt+Return", t));
    CHECK_EQ(FormatShortcut(t), std::string("Ctrl+Alt+Enter"));
}

TEST(Shortcuts_ohne_Modifier_oder_mit_Unsinn_werden_abgelehnt) {
    Shortcut s;
    CHECK(!ParseShortcut("Left", s));           // wuerde die Pfeiltaste global schlucken
    CHECK(!ParseShortcut("Ctrl+Alt", s));       // keine Taste
    CHECK(!ParseShortcut("Ctrl+Gibtsnicht", s));
    CHECK(!ParseShortcut("Ctrl+A+B", s));       // zwei echte Tasten
    CHECK(!ParseShortcut("", s));
}

TEST(Standardbelegung_ist_kollisionsfrei) {
    const Config c = Config::Defaults();
    std::set<std::string> seen;
    for (const auto& [action, sc] : c.shortcuts) {
        const std::string combo = FormatShortcut(sc);
        CHECK(!combo.empty());
        if (!seen.insert(combo).second)
            ::check::Fail(__FILE__, __LINE__, "Doppelte Belegung: " + combo);
    }
    // Win+Pfeil gehoert Windows selbst und darf nicht vorbelegt sein.
    for (const auto& [action, sc] : c.shortcuts) {
        const bool winArrow = (sc.mods == kModWin) && sc.vk >= 0x25 && sc.vk <= 0x28;
        CHECK(!winArrow);
    }
    CHECK(c.ShortcutFor(Action::LeftHalf).has_value());
    CHECK(!c.ShortcutFor(Action::TopLeftNinth).has_value());
}

TEST(Konfiguration_ueberlebt_eine_Rundreise) {
    Config c = Config::Defaults();
    c.gaps = Gaps{12, 8};
    c.cycleSizes = false;
    c.snapAreasEnabled = false;
    c.ignoredApps = {"vmware.exe", "Citrix.exe"};
    c.shortcuts[Action::TopLeftNinth] = Shortcut{kModCtrl | kModShift, 0x70};

    Config back;
    std::string err;
    CHECK(Config::FromJson(c.ToJson(), back, err));
    CHECK_EQ(err, std::string(""));
    CHECK_EQ(back.gaps.outer, 12);
    CHECK_EQ(back.gaps.inner, 8);
    CHECK(!back.cycleSizes);
    CHECK(!back.snapAreasEnabled);
    CHECK_EQ(back.ignoredApps.size(), size_t{2});
    CHECK(back.IsIgnored("VMWare.exe"));   // Gross-/Kleinschreibung egal
    CHECK(!back.IsIgnored("notepad.exe"));
    CHECK(back.shortcuts == c.shortcuts);
}

TEST(Teilweise_Konfiguration_ergaenzt_die_Vorgaben) {
    Config c;
    std::string err;
    CHECK(Config::FromJson(R"({"gaps": {"outer": 6}})", c, err));
    CHECK_EQ(c.gaps.outer, 6);
    CHECK_EQ(c.gaps.inner, 0);
    // Ohne shortcuts-Block bleiben die Standardbelegungen erhalten.
    CHECK(c.ShortcutFor(Action::LeftHalf).has_value());
}

TEST(Ein_shortcuts_Block_ersetzt_die_Vorgaben_vollstaendig) {
    Config c;
    std::string err;
    CHECK(Config::FromJson(R"({"shortcuts": {"maximize": "Ctrl+Alt+M"}})", c, err));
    CHECK_EQ(c.shortcuts.size(), size_t{1});
    CHECK(!c.ShortcutFor(Action::LeftHalf).has_value());
    CHECK_EQ(FormatShortcut(*c.ShortcutFor(Action::Maximize)), std::string("Ctrl+Alt+M"));
}

TEST(Kaputte_Eintraege_werden_gemeldet_aber_nicht_fatal) {
    Config c;
    std::string err;
    std::vector<std::string> warnings;
    const char* text = R"({"shortcuts": {
        "maximize": "Ctrl+Alt+M",
        "gibt-es-nicht": "Ctrl+Alt+X",
        "center": "Quatsch",
        "restore": ""
    }})";
    CHECK(Config::FromJson(text, c, err, &warnings));
    CHECK_EQ(warnings.size(), size_t{2});
    CHECK_EQ(c.shortcuts.size(), size_t{1});
    CHECK(c.ShortcutFor(Action::Maximize).has_value());
    CHECK(!c.ShortcutFor(Action::Restore).has_value());
}

TEST(Kaputtes_JSON_wird_abgelehnt) {
    Config c;
    std::string err;
    CHECK(!Config::FromJson("{ das ist kein json", c, err));
    CHECK(!err.empty());
    CHECK(!Config::FromJson("[1,2,3]", c, err));
}
