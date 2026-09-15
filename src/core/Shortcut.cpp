#include "Shortcut.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <vector>

namespace wintangle {
namespace {

struct KeyEntry {
    unsigned vk;
    std::string_view name;
};

// Virtual key codes as plain numbers, so the core needs no <windows.h>.
const std::vector<KeyEntry>& KeyTable() {
    static const std::vector<KeyEntry> kKeys = [] {
        std::vector<KeyEntry> keys{
            {0x08, "Backspace"}, {0x09, "Tab"},      {0x0D, "Enter"},   {0x1B, "Escape"},
            {0x20, "Space"},     {0x21, "PageUp"},   {0x22, "PageDown"},{0x23, "End"},
            {0x24, "Home"},      {0x25, "Left"},     {0x26, "Up"},      {0x27, "Right"},
            {0x28, "Down"},      {0x2D, "Insert"},   {0x2E, "Delete"},
            {0x6A, "NumMultiply"}, {0x6B, "NumPlus"}, {0x6D, "NumMinus"},
            {0x6E, "NumDecimal"},  {0x6F, "NumDivide"},
            {0xBA, "Semicolon"}, {0xBB, "Plus"},     {0xBC, "Comma"},   {0xBD, "Minus"},
            {0xBE, "Period"},    {0xBF, "Slash"},    {0xC0, "Backquote"},
            {0xDB, "BracketLeft"}, {0xDC, "Backslash"}, {0xDD, "BracketRight"},
            {0xDE, "Quote"},
        };
        // Letters and digits sit on their ASCII values.
        static std::array<char, 26> letters{};
        for (unsigned i = 0; i < 26; ++i) {
            letters[i] = static_cast<char>('A' + i);
            keys.push_back({0x41 + i, std::string_view(&letters[i], 1)});
        }
        static std::array<char, 10> digits{};
        for (unsigned i = 0; i < 10; ++i) {
            digits[i] = static_cast<char>('0' + i);
            keys.push_back({0x30 + i, std::string_view(&digits[i], 1)});
        }
        static std::array<std::string, 24> fkeys{};
        for (unsigned i = 0; i < 24; ++i) {
            fkeys[i] = "F" + std::to_string(i + 1);
            keys.push_back({0x70 + i, fkeys[i]});
        }
        static std::array<std::string, 10> numpad{};
        for (unsigned i = 0; i < 10; ++i) {
            numpad[i] = "Num" + std::to_string(i);
            keys.push_back({0x60 + i, numpad[i]});
        }
        return keys;
    }();
    return kKeys;
}

std::string Upper(std::string_view s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
}

std::string Trim(std::string_view s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return std::string(s.substr(b, e - b));
}

unsigned ModifierFromToken(const std::string& upper) {
    if (upper == "CTRL" || upper == "CONTROL" || upper == "STRG") return kModCtrl;
    if (upper == "ALT" || upper == "OPTION") return kModAlt;
    if (upper == "SHIFT" || upper == "UMSCHALT") return kModShift;
    if (upper == "WIN" || upper == "SUPER" || upper == "META" || upper == "CMD") return kModWin;
    return 0;
}

unsigned VkFromToken(const std::string& upper) {
    for (const auto& k : KeyTable()) {
        if (Upper(k.name) == upper) return k.vk;
    }
    // A few convenient synonyms.
    if (upper == "RETURN") return 0x0D;
    if (upper == "ESC") return 0x1B;
    if (upper == "PGUP") return 0x21;
    if (upper == "PGDN") return 0x22;
    if (upper == "DEL") return 0x2E;
    if (upper == "LEERTASTE") return 0x20;
    return 0;
}

}  // namespace

std::string_view KeyName(unsigned vk) {
    for (const auto& k : KeyTable()) {
        if (k.vk == vk) return k.name;
    }
    return {};
}

bool ParseShortcut(std::string_view text, Shortcut& out) {
    Shortcut result;
    size_t start = 0;
    bool haveKey = false;

    while (start <= text.size()) {
        const size_t plus = text.find('+', start);
        const std::string token = Trim(text.substr(
            start, plus == std::string_view::npos ? std::string_view::npos : plus - start));
        start = (plus == std::string_view::npos) ? text.size() + 1 : plus + 1;
        if (token.empty()) continue;

        const std::string upper = Upper(token);
        if (const unsigned mod = ModifierFromToken(upper)) {
            result.mods |= mod;
            continue;
        }
        const unsigned vk = VkFromToken(upper);
        if (vk == 0) return false;
        if (haveKey) return false;  // two non-modifier keys
        result.vk = vk;
        haveKey = true;
    }

    // Without a modifier the hotkey would swallow that key system wide.
    if (!haveKey || result.mods == 0) return false;
    out = result;
    return true;
}

std::string FormatShortcut(const Shortcut& s) {
    if (!s.IsValid()) return {};
    std::string out;
    if (s.mods & kModCtrl) out += "Ctrl+";
    if (s.mods & kModAlt) out += "Alt+";
    if (s.mods & kModShift) out += "Shift+";
    if (s.mods & kModWin) out += "Win+";
    out += KeyName(s.vk);
    return out;
}

}  // namespace wintangle
