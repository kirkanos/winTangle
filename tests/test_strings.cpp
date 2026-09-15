#include <set>
#include <string>

#include "check.h"
#include "core/Strings.h"

using namespace wintangle;

namespace {

// Restores the language after a test that changes it, so the order in which
// tests run cannot matter.
struct LanguageGuard {
    Language previous = CurrentLanguage();
    ~LanguageGuard() { SetLanguage(previous); }
};

}  // namespace

TEST(Every_string_exists_in_every_language) {
    LanguageGuard guard;
    for (Language language : AllLanguages()) {
        SetLanguage(language);
        for (size_t i = 0; i < static_cast<size_t>(Str::Count_); ++i) {
            const auto id = static_cast<Str>(i);
            if (Text(id).empty()) {
                ::check::Fail(__FILE__, __LINE__,
                              "Empty string #" + std::to_string(i) + " in " +
                                  std::string(LanguageDisplayName(language)));
            }
        }
    }
}

TEST(Every_action_has_a_label_in_every_language) {
    for (Language language : AllLanguages()) {
        for (Action a : AllActions()) {
            if (ActionLabel(a, language).empty()) {
                ::check::Fail(__FILE__, __LINE__,
                              "No label for " + std::string(ActionName(a)) + " in " +
                                  std::string(LanguageDisplayName(language)));
            }
        }
    }
}

TEST(Translations_actually_differ_from_English) {
    // A German column that merely repeats the English text is almost always a
    // forgotten translation. A handful of words really are identical in both
    // languages, so only a gross mismatch is treated as a failure.
    size_t identical = 0;
    for (size_t i = 0; i < static_cast<size_t>(Str::Count_); ++i) {
        const auto id = static_cast<Str>(i);
        SetLanguage(Language::English);
        const std::string en(Text(id));
        SetLanguage(Language::German);
        if (en == Text(id)) ++identical;
    }
    SetLanguage(Language::English);
    // "Position" and the copyright line are legitimately the same.
    CHECK(identical <= 4);
}

TEST(Action_labels_are_translated) {
    CHECK_EQ(std::string(ActionLabel(Action::LeftHalf, Language::English)),
             std::string("Left Half"));
    CHECK_EQ(std::string(ActionLabel(Action::LeftHalf, Language::German)),
             std::string("Linke Hälfte"));
}

TEST(Language_tags_survive_a_round_trip) {
    for (Language language : AllLanguages()) {
        Language back{};
        CHECK(LanguageFromTag(LanguageTag(language), back));
        CHECK(back == language);
        CHECK(!LanguageDisplayName(language).empty());
    }
}

TEST(Regional_tags_resolve_to_their_base_language) {
    Language language{};
    CHECK(LanguageFromTag("de-DE", language));
    CHECK(language == Language::German);
    CHECK(LanguageFromTag("de_AT", language));
    CHECK(language == Language::German);
    CHECK(LanguageFromTag("en-GB", language));
    CHECK(language == Language::English);
    CHECK(!LanguageFromTag("fr", language));
    CHECK(!LanguageFromTag("", language));
}

TEST(Switching_language_changes_what_Text_returns) {
    LanguageGuard guard;
    SetLanguage(Language::English);
    const std::string english(Text(Str::MenuQuit));
    SetLanguage(Language::German);
    const std::string german(Text(Str::MenuQuit));
    CHECK_EQ(english, std::string("Quit"));
    CHECK_EQ(german, std::string("Beenden"));
    CHECK(LocalizedActionLabel(Action::Maximize) == std::string("Maximieren"));
}
