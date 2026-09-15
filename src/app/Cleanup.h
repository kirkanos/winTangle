#pragma once

#include <string>
#include <vector>

#include "core/Strings.h"

namespace wintangle {

// Removes every trace WinTangle leaves on the system.
//
// There are exactly five, and all of them live under HKEY_CURRENT_USER or in
// the user profile -- WinTangle never writes machine wide:
//
//   1. %APPDATA%\WinTangle\            configuration
//   2. HKCU\...\CurrentVersion\Run     autostart entry
//   3. HKCU\Software\Classes\wintangle URL protocol
//   4. HKCU\Software\WinTangle         WinSparkle's update state
//   5. SPI_SETWINARRANGING             Windows' own snapping, if the program
//                                      turned it off
//
// Number five is the one that is easy to miss: that is a Windows setting the
// program may have changed, and it would otherwise stay switched off after
// uninstalling.
//
// This function is the only place that list is maintained -- the uninstaller
// calls it through "wintangle.exe --cleanup" rather than repeating the
// deletions in its own script.
struct CleanupReport {
    // The traces themselves, as string ids -- the report is only turned into
    // text when it is shown, so it follows the language in use.
    std::vector<Str> removed;
    std::vector<Str> failed;

    bool AnythingRemoved() const { return !removed.empty(); }
};

CleanupReport RemoveAllTraces();

// Formatted for a message to the user.
std::wstring FormatCleanupReport(const CleanupReport& report);

}  // namespace wintangle
