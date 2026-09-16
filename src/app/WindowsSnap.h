#pragma once

namespace wintangle {

// Windows' own window arranging ("Aero Snap") -- the feature that snaps a
// window when it is dragged to a screen edge.
//
// This is a Windows setting, not ours. WinTangle only touches it when the user
// explicitly asks for it, because its own snap areas react to the same drag and
// the two would otherwise fight. Everything here follows one rule: never leave
// the setting in a state the user did not ask for.
//
// To make that possible across restarts and crashes, the value found before a
// change is written to HKCU\Software\WinTangle. Without that bookkeeping there
// is no way to tell "the user switched it off themselves" from "we switched it
// off", and restoring it would overrule the user either way.

// Current state of the Windows setting.
bool IsWindowArrangingEnabled();

// Brings the Windows setting in line with the preference, and nothing more:
//
//   disableRequested == true   remember the current value once, then switch off
//   disableRequested == false  restore the remembered value, if we changed it
//
// When the preference is off and we never changed anything, the setting is left
// completely alone.
void ApplyWindowArrangingPreference(bool disableRequested);

// One-time repair for machines that ran 0.1.0-rc1 or rc2. Those forced the
// setting on every start instead of leaving it alone, so they could switch off
// Aero Snap for users who never asked. Turns it back on exactly once, recorded
// so it never overrules a later decision by the user.
void RepairWindowArrangingIfDamagedByOldVersion(bool disableRequested);

}  // namespace wintangle
