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

#include "core/Rect.h"

namespace wintangle {

inline Rect FromRECT(const RECT& r) { return Rect{r.left, r.top, r.right, r.bottom}; }
inline RECT ToRECT(const Rect& r) { return RECT{r.left, r.top, r.right, r.bottom}; }

// UTF-8 <-> UTF-16, because the platform free core works with std::string
// while Win32 is addressed with wide strings throughout.
std::wstring Widen(std::string_view utf8);
std::string Narrow(std::wstring_view utf16);

// The last Win32 error as readable text, for logs and messages.
std::wstring LastErrorMessage(DWORD code = GetLastError());

}  // namespace wintangle
