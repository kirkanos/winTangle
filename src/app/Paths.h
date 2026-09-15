#pragma once

#include <string>

#include "platform/Win32.h"

namespace wintangle {

// %APPDATA%\WinTangle, wird beim ersten Aufruf angelegt.
std::wstring AppDataDir();

// %APPDATA%\WinTangle\config.json
std::wstring ConfigPath();

// Vollstaendiger Pfad zur eigenen Exe (fuer Autostart und URL-Protokoll).
std::wstring ExecutablePath();

}  // namespace wintangle
