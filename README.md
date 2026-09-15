# WinTangle

Window arrangement for Windows, modelled on
[Rectangle](https://github.com/rxhanson/Rectangle) for macOS: snap windows to
halves, thirds, quarters, sixths, eighths and ninths with a global hotkey or by
dragging them to a screen edge.

Native C++/Win32, no runtime dependency, a single executable.

## Status

| Area | State |
|---|---|
| Calculation core (all 58 actions, cycling, gaps, layouts, history) | done, 55 unit tests green |
| Configuration (JSON, shortcut parser, import/export) | done, tested |
| URL scheme `wintangle://` including its parser | done, parser tested |
| Win32 layer (windows, displays, hotkeys, tray, drag-snap, settings) | compiles warning free (mingw-w64 cross build), **never yet run on real Windows** |

Development happens on macOS. The core runs and is tested there, the Win32
layer is cross-built locally with mingw-w64 — that catches compile and link
errors but says nothing about runtime behaviour. The MSVC build in CI is what
counts. Exactly one step is therefore still open: start the executable on a
Windows machine and work through the test matrix below.

## Installation

Two routes, both on the
[releases page](https://github.com/kirkanos/winTangle/releases):

- **Setup executable** — installs per user (no administrator rights, no UAC
  prompt), sets up autostart, uninstall and the `wintangle://` protocol, and
  tells you when a new version is out.
- **ZIP** — unzip and run. No entry in the app list, autostart from the tray
  menu.

Neither is signed, so SmartScreen will speak up on first launch ("More info" →
"Run anyway"). Every release page carries the checksums.

## Building

Windows (Visual Studio 2022, C++ desktop workload):

```
cmake -S . -B build -A x64
cmake --build build --config Release
build\src\platform\Release\wintangle.exe
```

macOS/Linux (core and tests only):

```
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

Cross-building the whole app on macOS (`brew install mingw-w64`), to check the
Win32 layer without a Windows machine:

```
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake
cmake --build build-win -j
```

Update checking is off automatically in that configuration
(`WINTANGLE_ENABLE_UPDATES=OFF`): WinSparkle ships MSVC binaries only. The
program still works completely, the update controls are simply disabled. The
resulting executable depends on `libstdc++`/`libgcc` and is meant purely for
verification; what ships is the statically linked MSVC build.

## Default bindings

`Win+arrow` belongs to Windows' own snapping and cannot be overridden through
`RegisterHotKey` — which is why everything sits on `Ctrl+Alt`, just as
Rectangle uses `⌃⌥`.

| Action | Combination |
|---|---|
| Left / right / top / bottom half | `Ctrl+Alt+←/→/↑/↓` |
| Quarters | `Ctrl+Alt+U/I/J/K` |
| Thirds | `Ctrl+Alt+D/F/G`, two thirds `Ctrl+Alt+E/T` |
| Maximize | `Ctrl+Alt+Enter` |
| Maximize height / width | `Ctrl+Alt+Shift+↑/↓` |
| Larger / smaller | `Ctrl+Alt++` / `Ctrl+Alt+-` |
| Center / restore | `Ctrl+Alt+C` / `Ctrl+Alt+Backspace` |
| Next / previous display | `Ctrl+Alt+Win+→/←` |
| Tile all / cascade all | `Ctrl+Alt+Shift+T` / `Ctrl+Alt+Shift+S` |

Sixths, eighths and ninths are deliberately unbound and reachable from the tray
menu, the settings window or by URL.

Pressing the same half binding again cycles the size: 1/2 → 2/3 → 1/3.

## Configuration

`%APPDATA%\WinTangle\config.json`, editable in the settings window and
importable/exportable from there. Line comments (`//`) are allowed.

```json
{
  "gaps": { "outer": 8, "inner": 8 },
  "cycleSizes": true,
  "snapAreasEnabled": true,
  "ignoredApps": ["vmware.exe"],
  "shortcuts": { "left-half": "Ctrl+Alt+Left" }
}
```

A `shortcuts` block replaces the default bindings entirely; an empty value
(`""`) explicitly unbinds an action.

## Language

The interface speaks English and German. By default it follows the language
Windows itself is displayed in (`GetUserDefaultUILanguage`); the settings
window can pin it, as can the config file:

```json
{ "language": "auto" }
```

`"auto"`, `"en"` or `"de"`; regional tags such as `"de-AT"` resolve to their
base language.

Picking a language in the settings window relabels the window on the spot, so
the choice is visible before saving. Cancelling puts the previous language
back — otherwise the tray menu would end up speaking a language the user just
backed out of.

Adding a language means adding one column to the table in
`src/core/Strings.cpp` and one German-style label column in
`src/core/Action.cpp`. The test suite then insists that every string and every
action label is filled in for the new language, so nothing can be forgotten
quietly. That is the same idea as Rectangle's per-language `.strings` files,
compiled into the binary so WinTangle stays a single file with no resources to
deploy alongside it.

## Actions by URL

```
wintangle://execute-action?name=left-half
```

The counterpart to Rectangle's `rectangle://`. Useful for Stream Deck,
AutoHotkey or scripts. The protocol is registered under `HKCU` at startup.

## Removing every trace

When uninstalling, the setup asks whether the personal data should go too. The
default is **No** — somebody merely moving to a new version should keep their
key bindings. With **Yes**, nothing is left behind.

The portable build has no uninstaller, so it offers the same by hand:

```
wintangle.exe --cleanup
```

Either way this is what gets removed:

| Trace | Location |
|---|---|
| Settings | `%APPDATA%\WinTangle\` |
| Autostart | `HKCU\…\CurrentVersion\Run` |
| URL protocol | `HKCU\Software\Classes\wintangle` |
| Update state | `HKCU\Software\WinTangle` |
| Windows snapping | `SPI_SETWINARRANGING` is switched back on |

That last one is the easy one to miss: it is a Windows setting WinTangle may
have changed, and it would otherwise stay switched off after uninstalling.

WinTangle writes exclusively under `HKEY_CURRENT_USER` and into the user
profile — nothing machine wide, nothing requiring administrator rights.

## Updates

An installed copy checks once a day whether a new version exists and offers to
install it — the same approach Rectangle takes through Sparkle, here through
[WinSparkle](https://winsparkle.org) (MIT licence). Switch it off in the tray
menu or the settings; check manually with "Check for Updates…".

Technically WinSparkle reads an appcast XML from
`releases/latest/download/appcast.xml`. GitHub always redirects that address to
the newest release, so it never needs maintaining. The file is produced during
the release workflow by `packaging/make_appcast.py`, and the changes it shows
come from CHANGELOG.md.

The ZIP download updates the same way — though doing so turns it into an
installation, because the update runs the installer.

## CI and releases

`ci.yml` runs on every push to `main` and on every pull request: core and tests
on Linux, the complete app with MSVC on Windows. The executable built there is
attached to the run as an artifact for 14 days.

A release is created by a tag:

```
# bump the version in CMakeLists.txt, add the CHANGELOG.md section, then
git tag v0.1.0 && git push origin v0.1.0
```

`release.yml` then builds, tests and creates the release page with the setup
executable (Inno Setup), the portable ZIP, `SHA256SUMS.txt` and `appcast.xml`.
The notes consist of the CHANGELOG section for that version, the commit list
since the previous tag and the checksums. Tags with a suffix (`v0.2.0-rc1`) are
marked as pre-releases.

If the tag disagrees with the version in `CMakeLists.txt`, the run aborts —
otherwise the executable would carry a different number than the release page.

## Layout

```
src/core/      geometry, action catalogue, layouts, history, JSON, URI,
               snap zones, string catalogue -- platform free and fully unit
               tested
src/config/    configuration model, loading, saving, migrating
src/platform/  Win32: window access, displays, execution, WinMain
src/app/       hotkeys, tray icon, autostart, URL scheme, paths, cleanup
src/snap/      drag detection and the preview overlay
src/ui/        settings window
packaging/     Inno Setup script and appcast generation
```

The core works purely with `Rect` and knows nothing about `HWND`. Every action
is a pure function `(window, work area, repetition) → Rect`; hotkey, tray menu,
URL call and drag-snap all run through the same path.

## What still needs checking on Windows

1. Single display at 100% — every key combination.
2. Two displays at 100% and 150% with different resolutions — positions exact,
   no drift when moving between them.
3. Taskbar on the left, on top, auto-hiding — `rcWork` respected.
4. Maximized window → half → restore.
5. A window with a minimum size (Windows Terminal settings) — no overlap, the
   cycling chain stays intact.
6. An elevated window (Task Manager) — one clear notice instead of silence.
7. Chrome, VS Code (Electron), a WinUI app, an old Win32 app — Electron and
   WinUI are the usual outliers for shadows and DPI.
8. Install, update and uninstall including the cleanup prompt.

## Known limits

* **Windows of elevated processes** (Task Manager, programs started as
  administrator) can only be arranged when WinTangle itself runs elevated —
  that is a Windows security boundary (UIPI), not a gap in the program.
  WinTangle says so once instead of failing silently.
* **Virtual desktops** are not switched. `IVirtualDesktopManager` is officially
  read-only; everything beyond that hangs off undocumented COM interfaces that
  Microsoft breaks between Windows builds. Rectangle Pro sits at the same
  boundary with macOS Spaces.
* **Drag-snap and Windows AeroSnap** both react to the same drag towards an
  edge. Windows' own snapping can be switched off in the settings.

## Prior art

Rectangle by Ryan Hanson, MIT licence. WinTangle is an independent
reimplementation for Win32 — what was taken over is the feature set, the
default bindings and the behaviour, not any code.

## Licence

Copyright (C) 2026 Andreas Hacker

WinTangle is free software under the
[GNU General Public License v3](LICENSE) or (at your option) any later version.
Redistribution and modification are permitted as long as derived works carry
the same terms and publish their source. The program comes with absolutely no
warranty.
