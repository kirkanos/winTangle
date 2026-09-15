// Shared Windows header. Included first by everything in the Win32 layer so
// the include order and the lean defines are the same everywhere.
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

#include "core/Language.h"
#include "core/Rect.h"
#include "core/Strings.h"

namespace wintangle {

inline Rect FromRECT(const RECT& r) { return Rect{r.left, r.top, r.right, r.bottom}; }
inline RECT ToRECT(const Rect& r) { return RECT{r.left, r.top, r.right, r.bottom}; }

// UTF-8 <-> UTF-16, because the platform free core works with std::string
// while Win32 is addressed with wide strings throughout.
std::wstring Widen(std::string_view utf8);
std::string Narrow(std::wstring_view utf16);

// The last Win32 error as readable text, for logs and messages.
std::wstring LastErrorMessage(DWORD code = GetLastError());

// Localized text as a wide string. Short name on purpose: it appears in every
// menu entry and every dialog.
inline std::wstring T(Str id) { return Widen(Text(id)); }

// The language Windows itself is set to, as far as WinTangle speaks it.
// Falls back to English for everything else.
Language DetectUiLanguage();

}  // namespace wintangle
