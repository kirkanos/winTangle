#pragma once

namespace wintangle {

// Autostart via HKCU\Software\Microsoft\Windows\CurrentVersion\Run.
// Deliberately the Run key rather than a scheduled task: the key needs no
// administrator rights. If WinTangle runs elevated (to be able to touch
// windows of elevated processes), autostart has to be set up as a scheduled
// task instead.
bool IsAutostartEnabled();
bool SetAutostartEnabled(bool enabled);

}  // namespace wintangle
