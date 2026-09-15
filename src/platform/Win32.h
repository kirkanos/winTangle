// Gemeinsamer Windows-Header. Wird von allem im Win32-Teil zuerst eingebunden,
// damit die Reihenfolge und die Lean-Defines ueberall gleich sind.
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <dwmapi.h>
#include <shellapi.h>
#include <shellscalingapi.h>

#include <string>

#include "core/Rect.h"

namespace wintangle {

inline Rect FromRECT(const RECT& r) { return Rect{r.left, r.top, r.right, r.bottom}; }
inline RECT ToRECT(const Rect& r) { return RECT{r.left, r.top, r.right, r.bottom}; }

// UTF-8 <-> UTF-16, weil der plattformfreie Kern mit std::string arbeitet und
// Win32 durchgehend mit Wide Strings angesprochen wird.
std::wstring Widen(std::string_view utf8);
std::string Narrow(std::wstring_view utf16);

// Letzter Win32-Fehler als lesbarer Text, fuer Log und Meldungen.
std::wstring LastErrorMessage(DWORD code = GetLastError());

}  // namespace wintangle
