#include <set>
#include <string>

#include "check.h"
#include "config/Config.h"
#include "core/Json.h"
#include "core/Shortcut.h"

using namespace wintangle;

TEST(Json_reads_and_writes_nested_values) {
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
    // Missing fields yield null instead of throwing.
    CHECK(v["gibtsnicht"].IsNull());
    CHECK_EQ(v["missing"]["also-missing"].AsInt(7), 7);
}

TEST(Json_reports_errors_with_a_position) {
    json::Value v;
    std::string err;
    CHECK(!json::Parse("{\"a\": }", v, err));
    CHECK(!err.empty());
    CHECK(err.find("line") != std::string::npos);
}

TEST(Json_survives_a_round_trip) {
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

TEST(Json_allows_line_comments_in_the_config) {
    json::Value v;
    std::string err;
    CHECK(json::Parse("{\n // Kommentar\n \"a\": 1\n}", v, err));
    CHECK_EQ(v["a"].AsInt(), 1);
}

TEST(Shortcuts_parse_and_format_canonically) {
    Shortcut s;
    CHECK(ParseShortcut("Ctrl+Alt+Left", s));
    CHECK_EQ(s.mods, unsigned{kModCtrl | kModAlt});
    CHECK_EQ(s.vk, unsigned{0x25});
    CHECK_EQ(FormatShortcut(s), std::string("Ctrl+Alt+Left"));

    // Synonyms and order do not matter, the output does.
    Shortcut t;
    CHECK(ParseShortcut(" alt + strg + LEFT ", t));
    CHECK(t == s);
    CHECK(ParseShortcut("Win+Shift+F12", t));
    CHECK_EQ(FormatShortcut(t), std::string("Shift+Win+F12"));
    CHECK(ParseShortcut("Ctrl+Alt+Return", t));
    CHECK_EQ(FormatShortcut(t), std::string("Ctrl+Alt+Enter"));
}

TEST(Shortcuts_without_a_modifier_or_with_nonsense_are_rejected) {
    Shortcut s;
    CHECK(!ParseShortcut("Left", s));           // would swallow the arrow key globally
    CHECK(!ParseShortcut("Ctrl+Alt", s));       // no key at all
    CHECK(!ParseShortcut("Ctrl+Gibtsnicht", s));
    CHECK(!ParseShortcut("Ctrl+A+B", s));       // two real keys
    CHECK(!ParseShortcut("", s));
}

TEST(Default_bindings_are_collision_free) {
    const Config c = Config::Defaults();
    std::set<std::string> seen;
    for (const auto& [action, sc] : c.shortcuts) {
        const std::string combo = FormatShortcut(sc);
        CHECK(!combo.empty());
        if (!seen.insert(combo).second)
            ::check::Fail(__FILE__, __LINE__, "Duplicate binding: " + combo);
    }
    // Win+arrow belongs to Windows itself and must not be bound by default.
    for (const auto& [action, sc] : c.shortcuts) {
        const bool winArrow = (sc.mods == kModWin) && sc.vk >= 0x25 && sc.vk <= 0x28;
        CHECK(!winArrow);
    }
    CHECK(c.ShortcutFor(Action::LeftHalf).has_value());
    CHECK(!c.ShortcutFor(Action::TopLeftNinth).has_value());
}

TEST(Configuration_survives_a_round_trip) {
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
    CHECK(back.IsIgnored("VMWare.exe"));   // case insensitive
    CHECK(!back.IsIgnored("notepad.exe"));
    CHECK(back.shortcuts == c.shortcuts);
}

TEST(A_partial_config_falls_back_to_the_defaults) {
    Config c;
    std::string err;
    CHECK(Config::FromJson(R"({"gaps": {"outer": 6}})", c, err));
    CHECK_EQ(c.gaps.outer, 6);
    CHECK_EQ(c.gaps.inner, 0);
    // Without a shortcuts block the default bindings survive.
    CHECK(c.ShortcutFor(Action::LeftHalf).has_value());
}

TEST(A_shortcuts_block_replaces_the_defaults_entirely) {
    Config c;
    std::string err;
    CHECK(Config::FromJson(R"({"shortcuts": {"maximize": "Ctrl+Alt+M"}})", c, err));
    CHECK_EQ(c.shortcuts.size(), size_t{1});
    CHECK(!c.ShortcutFor(Action::LeftHalf).has_value());
    CHECK_EQ(FormatShortcut(*c.ShortcutFor(Action::Maximize)), std::string("Ctrl+Alt+M"));
}

TEST(Broken_entries_are_reported_but_not_fatal) {
    Config c;
    std::string err;
    std::vector<std::string> warnings;
    const char* text = R"({"shortcuts": {
        "maximize": "Ctrl+Alt+M",
        "no-such-action": "Ctrl+Alt+X",
        "center": "Quatsch",
        "restore": ""
    }})";
    CHECK(Config::FromJson(text, c, err, &warnings));
    CHECK_EQ(warnings.size(), size_t{2});
    CHECK_EQ(c.shortcuts.size(), size_t{1});
    CHECK(c.ShortcutFor(Action::Maximize).has_value());
    CHECK(!c.ShortcutFor(Action::Restore).has_value());
}

TEST(Broken_JSON_is_rejected) {
    Config c;
    std::string err;
    CHECK(!Config::FromJson("{ this is not json", c, err));
    CHECK(!err.empty());
    CHECK(!Config::FromJson("[1,2,3]", c, err));
}
