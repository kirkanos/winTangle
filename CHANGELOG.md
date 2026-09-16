# Changelog

Every notable change to WinTangle. Format based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versioning follows
[Semantic Versioning](https://semver.org/).

The section for a given version ends up on the GitHub release page
automatically — creating a tag reads it out of this file.

## [Unreleased]

### Changed

- Updates install without walking through the setup wizard again, and start the
  program afterwards. WinTangle now also quits on WinSparkle's request just
  before the installer takes over, instead of being closed by it.

## [0.1.0] – 2026-09-16

First cut: window arrangement by keyboard and by dragging to a screen edge,
modelled on Rectangle for macOS.

### Added

- 58 actions: halves, quarters, thirds, sixths, eighths and ninths, maximize
  (also height or width only), larger/smaller, center, mirror, move, switch
  display.
- Size cycling on repeated presses: 1/2 → 2/3 → 1/3.
- Multi-window actions: tile, rows, columns, cascade, cascade active app.
- Restoring the frame a window had before the first change.
- Snap areas when dragging to a screen edge, with a semi transparent preview.
- Settings window with key combination capture, inner and outer gaps, an
  ignore list and import/export.
- Notification area icon carrying the full action catalogue, each entry with a
  small picture of where the window will end up, in the manner of Rectangle.
- Actions by URL: `wintangle://execute-action?name=left-half`.
- Autostart through the registry Run key.
- Configuration as JSON at `%APPDATA%\WinTangle\config.json`.
- Setup executable (Inno Setup, per-user installation without administrator
  rights) alongside the portable ZIP.
- Automatic update checking through WinSparkle, once a day, switchable.
- English and German interface, following the Windows display language by
  default and pinnable in the settings or through `"language"` in the config.
  Picking a language in the settings window applies it immediately, and
  cancelling puts the previous one back.
- Trace-free removal: the setup asks during uninstall whether settings,
  autostart, URL protocol and update state should go as well, and switches
  Windows' own snapping back on. Portable users get the same through
  `wintangle.exe --cleanup`.

### Note for anyone who installed a release candidate

Versions 0.1.0-rc1 through rc5 could leave Windows' own window snapping
switched off, which also disables Win+arrow. 0.1.0 switches it back on once at
first start. If it does not come back, the switch is under Settings > System >
Multitasking > "Snap windows".

### Known limits

- Windows of processes with higher privileges can only be arranged when
  WinTangle itself runs elevated (UIPI).
- Virtual desktops are not switched.
- The executable is not signed; SmartScreen speaks up on first launch.
- The mingw cross build carries no update checking, because WinSparkle ships
  MSVC binaries only.

[Unreleased]: https://github.com/kirkanos/winTangle/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/kirkanos/winTangle/releases/tag/v0.1.0
