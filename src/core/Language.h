#pragma once

#include <string_view>
#include <vector>

namespace wintangle {

// Languages the interface is available in. Lives in its own header because
// both the action catalogue and the string catalogue need it, and they would
// otherwise include each other.
enum class Language {
    English,
    German,

    Count_
};

// "en" / "de" -- used in the config file.
std::string_view LanguageTag(Language language);

// Accepts "de" as well as "de-DE" and "de_AT": only the primary tag matters.
bool LanguageFromTag(std::string_view tag, Language& out);

// Name of the language in that language itself ("English", "Deutsch"), for the
// picker in the settings window.
std::string_view LanguageDisplayName(Language language);

const std::vector<Language>& AllLanguages();

}  // namespace wintangle
