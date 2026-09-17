#include "Strings.h"

#include <array>

namespace wintangle {
namespace {

struct Entry {
    Str id;
    std::string_view en;
    std::string_view de;
};

// One row per string, one column per language. Adding a language means adding
// a column here and a value to the Language enum -- the test suite then
// insists that every row is filled.
constexpr std::array<Entry, static_cast<size_t>(Str::Count_)> kStrings{{
    {Str::GroupHalves, "Halves", "Hälften"},
    {Str::GroupQuarters, "Quarters", "Viertel"},
    {Str::GroupThirds, "Thirds", "Drittel"},
    {Str::GroupSixths, "Sixths", "Sechstel"},
    {Str::GroupEighths, "Eighths", "Achtel"},
    {Str::GroupNinths, "Ninths", "Neuntel"},
    {Str::GroupSize, "Size", "Größe"},
    {Str::GroupPosition, "Position", "Position"},
    {Str::GroupDisplays, "Displays", "Bildschirme"},
    {Str::GroupMultipleWindows, "Multiple windows", "Mehrere Fenster"},

    {Str::MenuCheckUpdates, "Check for Updates…", "Nach Updates suchen…"},
    {Str::MenuCheckUpdatesDaily, "Check for Updates Daily", "Täglich nach Updates suchen"},
    {Str::MenuSnapAreas, "Snap Areas While Dragging", "Snap-Bereiche beim Ziehen"},
    {Str::MenuCycleSizes, "Cycle Sizes", "Größen zyklieren"},
    {Str::MenuLaunchAtLogin, "Launch at Login", "Mit Windows starten"},
    {Str::MenuSettings, "Settings…", "Einstellungen…"},
    {Str::MenuAbout, "About WinTangle", "Über WinTangle"},
    {Str::MenuQuit, "Quit", "Beenden"},

    {Str::SettingsTitle, "WinTangle Settings", "WinTangle – Einstellungen"},
    {Str::SettingsActionsHeading, "Actions and key combinations",
     "Aktionen und Tastenkombinationen"},
    {Str::SettingsColumnAction, "Action", "Aktion"},
    {Str::SettingsColumnShortcut, "Key combination", "Tastenkombination"},
    {Str::SettingsAssignHint,
     "Select an action and press the key combination. Delete removes it.",
     "Aktion auswählen und Tastenkombination drücken. Entf entfernt sie."},
    {Str::SettingsRemove, "Remove", "Entfernen"},
    {Str::SettingsOuterGap, "Outer gap (px):", "Außenabstand (px):"},
    {Str::SettingsInnerGap, "Inner gap (px):", "Innenabstand (px):"},
    {Str::SettingsCycleSizes, "Cycle sizes (1/2 → 2/3 → 1/3)",
     "Größen zyklieren (1/2 → 2/3 → 1/3)"},
    {Str::SettingsSnapAreas, "Snap areas while dragging", "Snap-Bereiche beim Ziehen"},
    {Str::SettingsDisableWindowsSnap, "Turn off Windows' own snapping",
     "Windows-eigenes Andocken abschalten"},
    {Str::SettingsLaunchAtLogin, "Launch at login", "Mit Windows starten"},
    {Str::SettingsMoveCursor, "Move cursor with window", "Mauszeiger mitbewegen"},
    {Str::SettingsCheckUpdates, "Check for updates daily", "Täglich nach Updates suchen"},
    {Str::SettingsLanguage, "Language:", "Sprache:"},
    {Str::SettingsLanguageAuto, "Same as Windows", "Wie Windows"},
    {Str::SettingsImport, "Import…", "Importieren…"},
    {Str::SettingsExport, "Export…", "Exportieren…"},
    {Str::SettingsSave, "Save", "Speichern"},
    {Str::SettingsCancel, "Cancel", "Abbrechen"},
    {Str::SettingsUnbound, "—", "—"},

    {Str::MsgImportFailed, "Import failed", "Import fehlgeschlagen"},
    {Str::MsgImportWarnings, "Import with warnings", "Import mit Hinweisen"},
    {Str::MsgExportFailed, "Export failed", "Export fehlgeschlagen"},
    {Str::MsgSkipped, "Skipped:", "Übersprungen:"},
    {Str::MsgConfigUnreadable, "The configuration could not be read:",
     "Die Konfiguration konnte nicht gelesen werden:"},
    {Str::MsgDefaultsApply, "The default settings apply.",
     "Es gelten die Standardeinstellungen."},
    {Str::MsgHotkeyConflictTitle, "Key combinations unavailable",
     "Tastenkombinationen nicht verfügbar"},
    {Str::MsgHotkeyConflictIntro, "Already taken by another program:",
     "Bereits von einem anderen Programm belegt:"},
    {Str::MsgWindowNotMovableTitle, "Window cannot be moved", "Fenster nicht verschiebbar"},
    {Str::MsgWindowNotMovableText,
     "This window belongs to a process with higher privileges. For WinTangle to "
     "arrange it, WinTangle itself has to run as administrator.",
     "Dieses Fenster gehört einem Prozess mit höheren Rechten. Damit WinTangle es "
     "anordnen kann, muss WinTangle selbst als Administrator laufen."},
    {Str::MsgUnknownCallTitle, "Unknown call", "Unbekannter Aufruf"},
    {Str::MsgAlreadyRunning, "WinTangle is already running (notification area).",
     "WinTangle läuft bereits (Symbol im Infobereich)."},
    {Str::MsgStartFailed, "WinTangle could not be started.",
     "WinTangle konnte nicht gestartet werden."},
    {Str::MsgUpdatesDisabled,
     "This build was made without update checking.\nNew versions are available at:",
     "Diese Fassung wurde ohne Update-Prüfung gebaut.\nNeue Versionen gibt es unter:"},

    {Str::MsgSnapHookFailedTitle, "Snap areas unavailable",
     "Snap-Bereiche nicht verfügbar"},
    {Str::MsgSnapHookFailedText,
     "Windows refused the hook that detects a window being dragged. Security "
     "software usually causes this. Keyboard shortcuts are unaffected.",
     "Windows hat den Hook abgelehnt, der das Ziehen eines Fensters erkennt. "
     "Meist liegt das an Sicherheitssoftware. Die Tastenkürzel sind davon nicht "
     "betroffen."},

    {Str::MsgAeroSnapBrokenTitle, "Windows snapping is switched off",
     "Windows-Andocken ist abgeschaltet"},
    {Str::MsgAeroSnapBrokenText,
     "An earlier version of WinTangle switched off Windows' own window snapping, "
     "which also disables Win+arrow. It could not be switched back on "
     "automatically. Turn it on under Settings > System > Multitasking > "
     "\"Snap windows\".",
     "Eine frühere Fassung von WinTangle hat das Windows-eigene Andocken "
     "abgeschaltet, was auch Win+Pfeil deaktiviert. Es ließ sich nicht "
     "automatisch wieder einschalten. Bitte unter Einstellungen > System > "
     "Multitasking > \"Fenster andocken\" einschalten."},

    {Str::AboutTitle, "About WinTangle", "Über WinTangle"},
    {Str::AboutDescription,
     "Window arrangement by keyboard and by dragging,\nmodelled on Rectangle for macOS.",
     "Fensteranordnung per Tastenkombination und Ziehen,\nnach dem Vorbild von Rectangle "
     "für macOS."},
    {Str::AboutUrlHint, "Actions can also be triggered by URL:",
     "Aktionen lassen sich auch per URL auslösen:"},
    {Str::AboutVersion, "Version", "Version"},
    {Str::AboutCopyright, "Copyright (C) 2026 Andreas Hacker",
     "Copyright (C) 2026 Andreas Hacker"},
    {Str::AboutLicense,
     "Free software under the GNU General Public License v3 or later.\n"
     "Comes with absolutely no warranty. See the LICENSE file.",
     "Freie Software unter der GNU General Public License v3 oder später.\n"
     "Ohne jede Gewährleistung. Einzelheiten in der Datei LICENSE."},

    {Str::AboutSnapAreas, "Snap areas:", "Snap-Bereiche:"},
    {Str::AboutStateActive, "active", "aktiv"},
    {Str::AboutStateInactive, "not active", "nicht aktiv"},

    {Str::CleanupTitle, "Clean up WinTangle", "WinTangle aufräumen"},
    {Str::CleanupStillRunning,
     "WinTangle is still running. Quit it from the notification area icon first, "
     "then run this again.",
     "WinTangle läuft noch. Bitte erst über das Symbol im Infobereich beenden und "
     "dann erneut aufrufen."},
    {Str::CleanupConfirm,
     "Remove every trace of WinTangle?\n\n"
     "• settings and key bindings\n"
     "• autostart entry\n"
     "• URL protocol wintangle://\n"
     "• stored update state\n\n"
     "Windows' own snapping is switched back on if WinTangle turned it off. "
     "The program itself is not deleted.",
     "Alle Spuren von WinTangle entfernen?\n\n"
     "• Einstellungen und Tastenbelegung\n"
     "• Autostart-Eintrag\n"
     "• URL-Protokoll wintangle://\n"
     "• gespeicherter Update-Zustand\n\n"
     "Das Windows-eigene Andocken wird wieder eingeschaltet, falls WinTangle es "
     "abgeschaltet hat. Das Programm selbst wird dabei nicht gelöscht."},
    {Str::CleanupDoneTitle, "WinTangle cleaned up", "WinTangle aufgeräumt"},
    {Str::CleanupRemoved, "Removed:", "Entfernt:"},
    {Str::CleanupNotRemoved, "Not removed (is WinTangle still running?):",
     "Nicht entfernt (läuft WinTangle noch?):"},
    {Str::TraceSettings, "Settings (%APPDATA%\\WinTangle)",
     "Einstellungen (%APPDATA%\\WinTangle)"},
    {Str::TraceAutostart, "Autostart entry", "Autostart-Eintrag"},
    {Str::TraceUrlProtocol, "URL protocol wintangle://", "URL-Protokoll wintangle://"},
    {Str::TraceUpdateState, "Update state", "Update-Zustand"},
    {Str::TraceWindowsSnap, "Windows' own snapping switched back on",
     "Windows-eigenes Andocken wieder eingeschaltet"},
}};

// Catches a row added to the enum but forgotten in the table at compile time.
static_assert(kStrings.size() == static_cast<size_t>(Str::Count_),
              "Every Str needs a row in kStrings");

Language g_language = Language::English;

std::string_view Pick(const Entry& entry, Language language) {
    switch (language) {
        case Language::German: return entry.de;
        case Language::English:
        default: return entry.en;
    }
}

}  // namespace

Language CurrentLanguage() { return g_language; }

void SetLanguage(Language language) {
    if (language < Language::Count_) g_language = language;
}

std::string_view Text(Str id) {
    const Entry& entry = kStrings[static_cast<size_t>(id)];
    const std::string_view text = Pick(entry, g_language);
    // A missing translation falls back to English rather than showing nothing.
    return text.empty() ? entry.en : text;
}

std::string_view LocalizedActionLabel(Action action) {
    return ActionLabel(action, g_language);
}

std::string_view LanguageTag(Language language) {
    switch (language) {
        case Language::German: return "de";
        case Language::English:
        default: return "en";
    }
}

bool LanguageFromTag(std::string_view tag, Language& out) {
    // Accepts "de" as well as "de-DE" / "de_AT": only the primary tag matters.
    const std::string_view primary = tag.substr(0, tag.find_first_of("-_"));
    for (Language language : AllLanguages()) {
        if (primary == LanguageTag(language)) {
            out = language;
            return true;
        }
    }
    return false;
}

std::string_view LanguageDisplayName(Language language) {
    switch (language) {
        case Language::German: return "Deutsch";
        case Language::English:
        default: return "English";
    }
}

const std::vector<Language>& AllLanguages() {
    static const std::vector<Language> kAll = [] {
        std::vector<Language> v;
        for (size_t i = 0; i < static_cast<size_t>(Language::Count_); ++i) {
            v.push_back(static_cast<Language>(i));
        }
        return v;
    }();
    return kAll;
}

}  // namespace wintangle
