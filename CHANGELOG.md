# Changelog

Alle nennenswerten Änderungen an WinTangle. Format nach
[Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
Versionierung nach [Semantic Versioning](https://semver.org/lang/de/).

Der Abschnitt zur jeweiligen Version landet automatisch auf der
GitHub-Release-Seite — beim Anlegen eines Tags wird er aus dieser Datei
gelesen.

## [Unreleased]

## [0.1.0] – noch nicht veröffentlicht

Erste Fassung: Fensteranordnung per Tastenkombination und per Ziehen an den
Bildschirmrand, nach dem Vorbild von Rectangle für macOS.

### Hinzugefügt

- 58 Aktionen: Hälften, Viertel, Drittel, Sechstel, Achtel und Neuntel,
  maximieren (auch nur Höhe oder Breite), vergrößern/verkleinern, zentrieren,
  spiegeln, verschieben, Monitorwechsel.
- Zyklus bei wiederholtem Tastendruck: 1/2 → 2/3 → 1/3.
- Mehr-Fenster-Aktionen: kacheln, Zeilen, Spalten, staffeln, aktive App
  staffeln.
- Wiederherstellen des Rahmens vor dem ersten Eingriff.
- Snap-Bereiche beim Ziehen an den Bildschirmrand mit halbtransparenter
  Vorschau.
- Einstellungsfenster mit Aufnahme von Tastenkombinationen, Innen- und
  Außenabständen, Ignorierliste und Import/Export.
- Symbol im Infobereich mit dem vollständigen Aktionskatalog.
- Aktionen per URL: `wintangle://execute-action?name=left-half`.
- Autostart über den Run-Schlüssel der Registry.
- Konfiguration als JSON unter `%APPDATA%\WinTangle\config.json`.
- Setup-Exe (Inno Setup, Installation pro Benutzer ohne Administratorrechte)
  neben dem portablen ZIP.
- Automatische Update-Prüfung über WinSparkle, einmal täglich, abschaltbar.

### Bekannte Grenzen

- Fenster von Prozessen mit höheren Rechten lassen sich nur anordnen, wenn
  WinTangle selbst erhöht läuft (UIPI).
- Virtuelle Desktops werden nicht umgeschaltet.
- Die Exe ist nicht signiert; SmartScreen meldet sich beim ersten Start.
- Der mingw-Cross-Build enthält keine Update-Prüfung, weil WinSparkle nur
  MSVC-Binärdateien ausliefert.

[Unreleased]: https://github.com/kirkanos/winTangle/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/kirkanos/winTangle/releases/tag/v0.1.0
