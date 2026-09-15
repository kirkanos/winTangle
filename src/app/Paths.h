#pragma once

#include <string>

#include "platform/Win32.h"

namespace wintangle {

// %APPDATA%\WinTangle, created on first use.
std::wstring AppDataDir();

// %APPDATA%\WinTangle\config.json
std::wstring ConfigPath();

// Full path to our own executable (for autostart and the URL protocol).
std::wstring ExecutablePath();

}  // namespace wintangle
