#pragma once

#include <string_view>

#include "Action.h"
#include "Language.h"

namespace wintangle {

// The interface text, one entry per string and language.
//
// Rectangle ships one .strings file per language. The Windows equivalent that
// keeps WinTangle a single dependency-free executable is a table compiled into
// the binary, which is what this is.

// Every piece of text the user can see. Keeping them as an enum rather than
// as raw literals is the point of the exercise: the compiler then knows about
// every string, and a test can prove that none of them is missing in any
// language.
enum class Str {
    // Tray menu groups
    GroupHalves, GroupQuarters, GroupThirds, GroupSixths, GroupEighths, GroupNinths,
    GroupSize, GroupPosition, GroupDisplays, GroupMultipleWindows,

    // Tray menu commands
    MenuCheckUpdates, MenuCheckUpdatesDaily, MenuSnapAreas, MenuCycleSizes,
    MenuLaunchAtLogin, MenuSettings, MenuAbout, MenuQuit,

    // Settings window
    SettingsTitle, SettingsActionsHeading, SettingsColumnAction, SettingsColumnShortcut,
    SettingsAssignHint, SettingsRemove, SettingsOuterGap,
    SettingsInnerGap, SettingsCycleSizes, SettingsSnapAreas, SettingsDisableWindowsSnap,
    SettingsLaunchAtLogin, SettingsMoveCursor, SettingsCheckUpdates, SettingsLanguage,
    SettingsLanguageAuto, SettingsImport, SettingsExport, SettingsRestoreDefaults,
    SettingsSave, SettingsCancel,
    SettingsUnbound,

    // Messages
    MsgImportFailed, MsgImportWarnings, MsgExportFailed, MsgSkipped,
    MsgRestoreDefaultsTitle, MsgRestoreDefaultsText,
    MsgConfigUnreadable, MsgDefaultsApply,
    MsgHotkeyConflictTitle, MsgHotkeyConflictIntro,
    MsgWindowNotMovableTitle, MsgWindowNotMovableText,
    MsgUnknownCallTitle, MsgAlreadyRunning, MsgStartFailed,
    MsgUpdatesDisabled, MsgSnapHookFailedTitle, MsgSnapHookFailedText,
    MsgAeroSnapBrokenTitle, MsgAeroSnapBrokenText,

    // About box
    AboutTitle, AboutDescription, AboutUrlHint, AboutVersion, AboutCopyright, AboutLicense,
    AboutSnapAreas, AboutStateActive, AboutStateInactive,

    // Cleanup
    CleanupTitle, CleanupStillRunning, CleanupConfirm, CleanupDoneTitle,
    CleanupRemoved, CleanupNotRemoved,
    TraceSettings, TraceAutostart, TraceUrlProtocol, TraceUpdateState, TraceWindowsSnap,

    Count_
};

// The language in use. Defaults to English until SetLanguage() says otherwise.
Language CurrentLanguage();
void SetLanguage(Language language);

// Localized text as UTF-8. Never empty: a missing translation falls back to
// English, and the test suite makes sure that never actually happens.
std::string_view Text(Str id);

// Localized label for an action, for the tray menu and the settings list.
std::string_view LocalizedActionLabel(Action action);

}  // namespace wintangle
