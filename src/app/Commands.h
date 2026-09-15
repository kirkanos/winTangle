#pragma once

#include "platform/Win32.h"

namespace wintangle {

// Menu and message ids in one place, so the tray menu, the settings window and
// the window procedure do not step on each other.
enum : unsigned {
    // Actions in the tray menu: kCmdActionBase + index into AllActions().
    kCmdActionBase = 1000,

    kCmdSettings = 100,
    kCmdToggleSnapAreas = 101,
    kCmdToggleAutostart = 102,
    kCmdToggleCycleSizes = 103,
    kCmdAbout = 104,
    kCmdQuit = 105,
    kCmdCheckUpdates = 106,
    kCmdToggleAutoUpdates = 107,
};

// Custom window messages.
enum : unsigned {
    kMsgTrayCallback = WM_APP + 1,   // click on the tray icon
    kMsgUriCommand = WM_APP + 2,     // wintangle:// call from a second instance
    kMsgTaskbarCreated = WM_APP + 3, // Explorer restarted (remapped)
    kMsgConfigChanged = WM_APP + 4,  // the settings were saved
};

}  // namespace wintangle
